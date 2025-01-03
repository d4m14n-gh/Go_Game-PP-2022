/*
 * template for the course Podstawy Programowania
 * only to be used for the project in this course
 * author: K.M. Ocetkiewicz
 */
#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include<windows.h>
#include "conio2.h"
#ifdef __GNUC__
#include<stdlib.h>
#endif

#ifdef __cplusplus
#define EXTERNC	extern "C"
#else
#define EXTERNC
#endif

int _wscroll;


typedef struct _Conio2ThreadData {
	int attrib;
	int charCount;
	int charValue;
	int charFlag;
	int ungetCount;
	int ungetBuf[16];
	int _wscroll;
	int width;
	int height;
	int origwidth;
	int origheight;
	int origdepth;
	int lastmode;
	HANDLE output;
	HANDLE input;
	DWORD prevOutputMode;
	DWORD prevInputMode;
} Conio2ThreadData;


static Conio2ThreadData thData;


static void UpdateWScroll() {
	if (_wscroll != thData._wscroll) {
		if (_wscroll) SetConsoleMode(thData.output, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
		else SetConsoleMode(thData.output, ENABLE_PROCESSED_OUTPUT);
		thData._wscroll = _wscroll;
	};
};


static WORD ToWinAttribs(int attrib) {
	WORD rv = 0;
	if (attrib & 1) rv |= FOREGROUND_BLUE;
	if (attrib & 2) rv |= FOREGROUND_GREEN;
	if (attrib & 4) rv |= FOREGROUND_RED;
	if (attrib & 8) rv |= FOREGROUND_INTENSITY;
	if (attrib & 16) rv |= BACKGROUND_BLUE;
	if (attrib & 32) rv |= BACKGROUND_GREEN;
	if (attrib & 64) rv |= BACKGROUND_RED;
	if (attrib & 128) rv |= BACKGROUND_INTENSITY;
	return rv;
};


static int FromWinAttribs(WORD attrib) {
	int rv = 0;
	if (attrib & FOREGROUND_BLUE) rv |= 1;
	if (attrib & FOREGROUND_GREEN) rv |= 2;
	if (attrib & FOREGROUND_RED) rv |= 4;
	if (attrib & FOREGROUND_INTENSITY) rv |= 8;
	if (attrib & BACKGROUND_BLUE) rv |= 16;
	if (attrib & BACKGROUND_GREEN) rv |= 32;
	if (attrib & BACKGROUND_RED) rv |= 64;
	if (attrib & BACKGROUND_INTENSITY) rv |= 128;
	return rv;
};


static void GetCP(int* x, int* y) {
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(thData.output, &info);
	if (x) *x = info.dwCursorPosition.X + 1;
	if (y) *y = info.dwCursorPosition.Y + 1;
};


static int HandleKeyEvent(INPUT_RECORD* buf) {
	int ch;
	ch = (int)(buf->Event.KeyEvent.uChar.AsciiChar) & 255;
	if (ch == 0) ch = 0x8000 + buf->Event.KeyEvent.wVirtualKeyCode;
	if (ch == 0x8010 || ch == 0x8011 || ch == 0x8012 || ch == 0x8014
		|| ch == 0x8090 || ch == 0x8091) return 0;
	thData.charCount = buf->Event.KeyEvent.wRepeatCount;
	thData.charFlag = ch & 0x8000 ? 1 : 0;
	if (thData.charFlag) thData.charCount *= 2;
	int alt = (buf->Event.KeyEvent.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
	int ctrl = (buf->Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
	switch (ch) {
	case 0x8000 + 33:	ch = 0x8000 + 73; break;
	case 0x8000 + 34:	ch = 0x8000 + 81; break;
	case 0x8000 + 35:	ch = 0x8000 + 79; break;
	case 0x8000 + 36:	ch = 0x8000 + 71; break;
	case 0x8000 + 37:	if (ctrl) ch = 0x8073;
					else if (alt) ch = 0x809b;
					else ch = 0x8000 + 75; break;
	case 0x8000 + 38:	if (ctrl) ch = 0x808d;
					else if (alt) ch = 0x8098;
					else ch = 0x8000 + 72; break;
	case 0x8000 + 39:	if (ctrl) ch = 0x8074;
					else if (alt) ch = 0x809d;
					else ch = 0x8000 + 77; break;
	case 0x8000 + 40:	if (ctrl) ch = 0x8091;
					else if (alt) ch = 0x80a0;
					else ch = 0x8000 + 80; break;
	case 0x8000 + 46:	ch = 0x8000 + 83; break;
	case 0x8000 + 112:	ch = 0x8000 + 59; break;
	case 0x8000 + 113:	ch = 0x8000 + 60; break;
	case 0x8000 + 114:	ch = 0x8000 + 61; break;
	case 0x8000 + 115:	ch = 0x8000 + 62; break;
	case 0x8000 + 116:	ch = 0x8000 + 63; break;
	case 0x8000 + 117:	ch = 0x8000 + 64; break;
	case 0x8000 + 118:	ch = 0x8000 + 65; break;
	case 0x8000 + 119:	ch = 0x8000 + 66; break;
	case 0x8000 + 120:	ch = 0x8000 + 67; break;
	case 0x8000 + 121:	ch = 0x8000 + 68; break;
	case 0x8000 + 122:	ch = 0x8000 + 133; break;
	case 0x8000 + 123:	ch = 0x8000 + 134; break;
	};
	thData.charValue = ch & 0x7fff;
	return 1;
};


static void ResizeConsole(HANDLE con, int w, int h, int d) {
	int cw, ch;
	COORD s;
	SMALL_RECT r;
	CONSOLE_SCREEN_BUFFER_INFO info;

	BOOL res = GetConsoleScreenBufferInfo(con, &info);
	cw = info.srWindow.Right - info.srWindow.Left + 1;
	ch = info.srWindow.Bottom - info.srWindow.Top + 1;

	if (w < cw || h < ch) {
		r.Top = 0;
		r.Left = 0;
		r.Right = (SHORT)(min(w, cw) - 1);
		r.Bottom = (SHORT)(min(h, ch) - 1);
		SetConsoleWindowInfo(con, TRUE, &r);
	};

	if (d < h) d = h;
	s.X = (SHORT)w;
	s.Y = (SHORT)d;
	SetConsoleScreenBufferSize(con, s);
	r.Top = 0;
	r.Left = 0;
	r.Right = (SHORT)(w - 1);
	r.Bottom = (SHORT)(h - 1);
	SetConsoleWindowInfo(con, TRUE, &r);
};


static void ReloadConsoleSize() {
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (GetConsoleScreenBufferInfo(thData.output, &info)) {
		thData.width = info.srWindow.Right - info.srWindow.Left + 1;
		thData.height = info.srWindow.Bottom - info.srWindow.Top + 1;
	};
};


static void InitConio2(Conio2ThreadData* data) {
	data->output = GetStdHandle(STD_OUTPUT_HANDLE);
	data->input = GetStdHandle(STD_INPUT_HANDLE);
	data->ungetCount = 0;
	data->charCount = 0;

	CONSOLE_SCREEN_BUFFER_INFO info;
	BOOL rc = GetConsoleScreenBufferInfo(data->output, &info);
	if (rc) {
		data->origwidth = info.srWindow.Right - info.srWindow.Left + 1;
		data->origheight = info.srWindow.Bottom - info.srWindow.Top + 1;
		data->origdepth = info.dwSize.Y;
	}
	else {
		data->origwidth = 80;
		data->origheight = 25;
		data->origdepth = 25;
	}

	data->width = data->origwidth;
	data->height = data->origheight;

	data->attrib = 0x07;
	data->lastmode = C80;
	data->_wscroll = -1;
	SetConsoleTextAttribute(thData.output, ToWinAttribs(thData.attrib));
	GetConsoleMode(data->input, &(data->prevInputMode));
	GetConsoleMode(data->output, &(data->prevOutputMode));
	SetConsoleMode(data->input, ENABLE_PROCESSED_INPUT);
	UpdateWScroll();
};


static void ExitConio2(Conio2ThreadData* data) {
	ResizeConsole(data->output, data->origwidth, data->origheight, data->origdepth);
	SetConsoleMode(data->input, data->prevInputMode);
	SetConsoleMode(data->output, data->prevOutputMode);
};


EXTERNC
void gotoxy(int x, int y) {
	COORD pos;
	pos.X = x - 1;
	pos.Y = y - 1;
	SetConsoleCursorPosition(thData.output, pos);
};


EXTERNC
int wherex() {
	int x;
	GetCP(&x, NULL);
	return x;
};


EXTERNC
int wherey() {
	int y;
	GetCP(NULL, &y);
	return y;
};


EXTERNC
void _setcursortype(int cur_t) {
	CONSOLE_CURSOR_INFO inf;
	GetConsoleCursorInfo(thData.output, &inf);
	if (cur_t == _NOCURSOR) {
		inf.bVisible = FALSE;
	}
	else if (cur_t == _NORMALCURSOR) {
		inf.bVisible = TRUE;
		inf.dwSize = 13;
	}
	else if (cur_t == _SOLIDCURSOR) {
		inf.bVisible = TRUE;
		inf.dwSize = 100;
	}
	else return;
	SetConsoleCursorInfo(thData.output, &inf);
};


EXTERNC
void textattr(int newattr) {
	thData.attrib = newattr;
	SetConsoleTextAttribute(thData.output, ToWinAttribs(thData.attrib));
};


EXTERNC
void textbackground(int newcolor) {
	thData.attrib = (thData.attrib & 0x0f) | ((newcolor & 15) << 4);
	SetConsoleTextAttribute(thData.output, ToWinAttribs(thData.attrib));
};


EXTERNC
void textcolor(int newcolor) {
	thData.attrib = (thData.attrib & 0xf0) | (newcolor & 15);
	SetConsoleTextAttribute(thData.output, ToWinAttribs(thData.attrib));
};


EXTERNC
void clreol() {
	int cx, cy;
	COORD pos;
	DWORD nwr, len;
	GetCP(&cx, &cy);
	len = thData.width - cx + 1;
	if (len > 0) {
		pos.X = cx - 1;
		pos.Y = cy - 1;
		FillConsoleOutputCharacter(thData.output, ' ', len, pos, &nwr);
		FillConsoleOutputAttribute(thData.output, thData.attrib, len, pos, &nwr);
		gotoxy(cx, cy);
	};
};


EXTERNC
void clrscr() {
	ReloadConsoleSize();
	DWORD nwr, len = thData.width * thData.height;
	COORD pos;
	pos.X = pos.Y = 0;
	FillConsoleOutputCharacter(thData.output, ' ', len, pos, &nwr);
	FillConsoleOutputAttribute(thData.output, thData.attrib, len, pos, &nwr);
};


EXTERNC
void delline() {
	int y;
	y = wherey();
	movetext(1, y + 1, thData.width, thData.height, 1, y);
};


EXTERNC
void insline() {
	int y;
	y = wherey();
	movetext(1, y, thData.width, thData.height - 1, 1, y + 1);
};


EXTERNC
int getch() {
	BOOL rv;
	DWORD n;
	INPUT_RECORD buf;

	if (thData.ungetCount > 0) {
		thData.ungetCount--;
		return thData.ungetBuf[thData.ungetCount];
	};

	if (thData.charCount > 0) {
		thData.charCount--;
		if ((thData.charCount & 1) && thData.charFlag) return 0;
		else return thData.charValue;
	};

	while (1) {
		rv = ReadConsoleInput(thData.input, &buf, 1, &n);
		if (!rv) continue;
		if (buf.EventType != KEY_EVENT) continue;
		if (!buf.Event.KeyEvent.bKeyDown) continue;
		if (HandleKeyEvent(&buf)) break;
	};

	thData.charCount--;
	if ((thData.charCount & 1) && thData.charFlag) return 0;
	else return thData.charValue;
};


EXTERNC
int getche() {
	int ch;
	ch = getch();
	putch(ch);
	return ch;
};


EXTERNC
int kbhit() {
	BOOL rv;
	DWORD n;
	INPUT_RECORD buf;

	if (thData.ungetCount > 0) return 1;
	if (thData.charCount > 0) return 1;

	rv = PeekConsoleInput(thData.input, &buf, 1, &n);
	if (!rv) return 0;
	if (n == 0) return 0;
	rv = ReadConsoleInput(thData.input, &buf, 1, &n);
	if (!rv) return 0;
	if (buf.EventType != KEY_EVENT) return 0;
	if (!buf.Event.KeyEvent.bKeyDown) return 0;
	return HandleKeyEvent(&buf) ? 1 : 0;
};


EXTERNC
int ungetch(int ch) {
	if (thData.ungetCount >= 15) return EOF;
	thData.ungetBuf[thData.ungetCount++] = ch;
	return ch;
};


EXTERNC
int cputs(const char* str) {
	DWORD count;
	if (str == NULL) return EOF;
	UpdateWScroll();
	if (WriteConsoleA(thData.output, str, strlen(str), &count, NULL)) return count;
	else return EOF;
};


EXTERNC
int putch(int c) {
	DWORD count;
	UpdateWScroll();
	if (WriteConsoleA(thData.output, &c, 1, &count, NULL)) return c;
	else return EOF;
};


EXTERNC
int gettext(int left, int top, int right, int bottom, void* dest) {
	int i;
	SMALL_RECT trg;
	COORD pos, size;
	CHAR_INFO* buf;
	buf = (CHAR_INFO*)alloca((right - left + 1) * (bottom - top + 1) * sizeof(CHAR_INFO));

	pos.X = 0; pos.Y = 0;
	size.X = right - left + 1; size.Y = bottom - top + 1;
	trg.Left = left - 1; trg.Top = top - 1;
	trg.Right = right - 1; trg.Bottom = bottom - 1;
	ReadConsoleOutputA(thData.output, buf, size, pos, &trg);
	for (i = 0; i < (right - left + 1) * (bottom - top + 1); i++) {
		((char*)dest)[i * 2] = buf[i].Char.AsciiChar;
		((char*)dest)[i * 2 + 1] = FromWinAttribs(buf[i].Attributes);
	};
	return 1;
};


EXTERNC
int movetext(int left, int top, int right, int bottom, int destleft, int desttop) {
	char* buf;
	buf = (char*)alloca((right - left + 1) * (bottom - top + 1) * 2);
	gettext(left, top, right, bottom, buf);
	puttext(destleft, desttop, destleft + right - left, desttop + bottom - top, buf);
	return 1;
};


EXTERNC
int puttext(int left, int top, int right, int bottom, void* source) {
	int i;
	SMALL_RECT trg;
	COORD pos, size;
	CHAR_INFO* buf;
	buf = (CHAR_INFO*)alloca((right - left + 1) * (bottom - top + 1) * sizeof(CHAR_INFO));
	for (i = 0; i < (right - left + 1) * (bottom - top + 1); i++) {
		buf[i].Char.AsciiChar = ((char*)source)[i * 2];
		buf[i].Attributes = ToWinAttribs(((char*)source)[i * 2 + 1]);
	};
	pos.X = 0; pos.Y = 0;
	size.X = right - left + 1; size.Y = bottom - top + 1;
	trg.Left = left - 1; trg.Top = top - 1;
	trg.Right = right - 1; trg.Bottom = bottom - 1;
	WriteConsoleOutputA(thData.output, buf, size, pos, &trg);
	return 1;
};


EXTERNC
void settitle(const char* title) {
	SetConsoleTitleA(title);
};


void gettextinfo(struct text_info* info) {
	info->curx = wherex();
	info->cury = wherey();
	info->attribute = thData.attrib;
	info->normattr = 0x07;
	info->screenwidth = thData.width;
	info->screenheight = thData.height;
};


void textmode(int mode) {
	if (mode == C80) {
		thData.width = 80;
		thData.height = 25;
		ResizeConsole(thData.output, 80, 25, 25);
	}
	else if (mode == C4350) {
		thData.width = 80;
		thData.height = 50;
		ResizeConsole(thData.output, 80, 50, 50);
	}
	else if (mode == LASTMODE) {
		textmode(thData.lastmode);
	};
};


static void Conio2_Exit(void) {
	ExitConio2(&thData);
};

int Conio2_Init(void) {
	InitConio2(&thData);
	atexit(Conio2_Exit);
	return 0;
};

#ifdef __cplusplus
static int conio2_init = Conio2_Init();
#endif
