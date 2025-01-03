#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include "conio2.h"

#define RANDOMCOLOR ((colors)(rand()%16))
#define R4C ((colors)(rand()%5))
#define RBWC ((colors)(rand()%2?black:white))
///
#define AUTOEND false           ///automatyczne zakonczenie
#define SUBMIT false            ///potwierdzenie tury
#define KOMI 0.0//6.5                ///komi
#define HANDICAP false           ///handicap
#define HANDICAPKOMI 0.5        ///handicap komi
#define LINECOLOR gray          ///kolor lini
#define BOARDCOLOR golden       ///kolor planszy
#define CURSORCOLOR red         ///kolor kursora
#define BLACKSTONECOLOR black   ///kolor czarnego
#define WHITESTONECOLOR lGray   ///kolor bialego   ///v///ksztalt kursora
#define OFFSETX 45              ///przesuniecie planszy
#define OFFSETY 4
#define MOFFSETX 2              ///przesuniecie menu
#define MOFFSETY 5
#define OFFSETXCENTER true      ///wysrodkowanie planszy
#define INNEROFFSET 3           ///szerokosc ramki
#define FRAMECOLOR gray         ///kolor ramki
#define FRAMECOLOR2 black       ///2kolor ramki
#define EXPXY (x+y)%9           ///wzor ramki
///BOARSIZE
#define MAXBOARDSIZE 50         ///rozmiar planszy ///9x9 13x13 19x19

struct point {
    int x;
    int y;
};

enum stoneTypes {
    whiteStone = 0,
    blackStone = 1
};

enum colors {
    black = 0,
    blue = 1,
    green = 2,
    cyan = 3,
    red = 4,
    purple = 5,
    golden = 6,
    lGray = 7,
    gray = 8,
    lBlue = 9,
    lGreen = 10,
    lCyan = 11,
    lRed = 12,
    magenta = 13,
    yellow = 14,
    white = 15
};

struct stone {
    bool placed;
    stoneTypes type;
    int breaths;
    point position;
    stone* rootStone;
    stone* child;
};

struct stoneS {
    bool placed;
    stoneTypes type;
    int breaths;
    point position;
    point rootStone = { -1, -1 };
    point child = { -1, -1 };
};

struct gameState {
    int boardSize;
    stone stones[MAXBOARDSIZE][MAXBOARDSIZE];
    point koPos, points;
    int koRound;
    int round = 1;
    double komi;
    bool ended;
};

struct gameStateS {
    int boardSize;
    stoneS stones[MAXBOARDSIZE][MAXBOARDSIZE];
    point koPos, points;
    int koRound;
    int round = 1;
    double komi;
    bool saved;
};

gameState G;



void quitGame() {
    system("cls");
    text_info ti;
    gettextinfo(&ti);
    gotoxy(1, ti.screenheight + 1);
    _setcursortype(2);
}

void handleSignnal(int sig) {
    quitGame();
    exit(0);
}

void save(gameStateS* GSS, gameState GS) {
    stone cStn;
    point root, child;
    for (int i = 0; i < G.boardSize; i++)
        for (int j = 0; j < G.boardSize; j++) {
            root = { -1, -1 }, child = { -1, -1 };
            cStn = GS.stones[i][j];
            if (cStn.child)
                child = { cStn.child->position.x, cStn.child->position.y };
            if (cStn.rootStone)
                root = { cStn.rootStone->position.x, cStn.rootStone->position.y };
            GSS->stones[i][j] = { cStn.placed, cStn.type, cStn.breaths, cStn.position, root, child };
        }
    GSS->koPos = GS.koPos;
    GSS->koRound = GS.koRound;
    GSS->round = GS.round;
    GSS->points = GS.points;
    GSS->komi = GS.komi;
    GSS->boardSize = GS.boardSize;
    GSS->saved = true;
}

void load(gameState* GS, gameStateS GSS) {
    stoneS cStn;
    stone* root, * child;
    for (int i = 0; i < GSS.boardSize; i++)
        for (int j = 0; j < GSS.boardSize; j++) {
            root = NULL, child = NULL;
            cStn = GSS.stones[i][j];
            if (cStn.child.x != -1 && cStn.child.y != -1)
                child = &(GS->stones[cStn.child.x][cStn.child.y]);
            if (cStn.rootStone.x != -1 && cStn.rootStone.y != -1)
                root = &(GS->stones[cStn.rootStone.x][cStn.rootStone.y]);
            GS->stones[i][j] = { cStn.placed, cStn.type, cStn.breaths, cStn.position, root, child };
        }
    GS->koPos = GSS.koPos;
    GS->koRound = GSS.koRound;
    GS->round = GSS.round;
    GS->points = GSS.points;
    GS->komi = GSS.komi;
    GS->boardSize = GSS.boardSize;
}

void log(const char* str, int d) {
    static int rf = 0, dr = 5;
    if (d != -1) {
        dr = d;
        rf = 0;
    }
    else if ((++rf) % dr) return;
    gotoxy(1, 1);
    for (int i = 0; i < 20; i++)
        putch(' ');
    gotoxy(1, 1);
    cputs(str);
}

void scan(char* text, point pos, int length, const char* label, bool sufix) {
    _setcursortype(2);
    char inCh;
    gotoxy(pos.x, pos.y);
    cputs(label);
    for (int i = 0; true; i++) {
        gotoxy(pos.x + i + 1, pos.y + 1);
        inCh = getch();
        if (inCh == 13) {
            if (sufix) {
                const char sufix[4] = ".go";
                for (int j = 0; j < 4; j++)
                    text[i + j] = sufix[j];
            }
            else
                text[i] = 0;
            break;
        }
        else if (inCh == 8) {
            i--;
            if (i >= 0) {
                gotoxy(pos.x + i + 1, pos.y + 1);
                putch(' ');
                text[i] = 0;
                i--;
            }
        }
        else if (i == length || inCh < 32) i--;
        else {
            putch(inCh);
            text[i] = inCh;
        }
    }
    gotoxy(pos.x, pos.y);
    for (int i = 0; i < 25; i++)
        putch(' ');
    gotoxy(pos.x + 1, pos.y + 1);
    for (int i = 0; i < 25; i++)
        putch(' ');
    gotoxy(1, 1);
    _setcursortype(0);
}

int getSize() {
    int newSize = 9;
    gotoxy(MOFFSETX + 1, MOFFSETY + 20);
    cputs("select board size([tab]&[enter]):");
    int currentOption = 0;
    const char sizes[4][20] = { "9x9", "13x13", "19x19", "other(5-50)" };
    while (true) {
        for (int i = 0; i < 4; i++) {
            gotoxy(MOFFSETX + 2, MOFFSETY + 21 + i);
            if (i == currentOption) {
                textcolor(lGreen);
                putch('>');
            }
            else putch(' ');
            cputs(sizes[i]);
            textcolor(white);
        }
        char getc = getch();
        if (getc == 9) {
            currentOption++;
            currentOption %= 4;
        }
        else if (getc == 13) {
            char otherStr[3];
            if (currentOption == 0) newSize = 9;
            else if (currentOption == 1) newSize = 13;
            else if (currentOption == 2) newSize = 19;
            else {
                scan(otherStr, { MOFFSETX + 13, MOFFSETY + 24 }, 2, ":", false);
                if (otherStr[1] != 0) newSize = (otherStr[0] - '0') * 10 + (otherStr[1] - '0');
                else newSize = otherStr[0] - '0';
                if (newSize < 5 || newSize>81) newSize = 9;
            }
            break;
        }
    }
    for (int h = 0; h < 5; h++) {
        gotoxy(MOFFSETX + 1, MOFFSETY + 20 + h);
        for (int i = 0; i < 35; i++)
            putch(' ');
    }
    return newSize;
}

void newGame(bool useGetSize=true) {
    static gameStateS newGSS;
    if(useGetSize)
        newGSS.boardSize = getSize();
    else
        newGSS.boardSize = 9;
    load(&G, newGSS);
    G.ended = false;
    for (int i = 0; i < MAXBOARDSIZE; i++)
        for (int j = 0; j < MAXBOARDSIZE; j++)
            G.stones[i][j].position = { i, j };
    if (HANDICAP) G.round = -1;
    G.komi = KOMI;
    log("new game", 5);
}

bool onBoard(point pos) {
    return !(pos.x < 0 || pos.x >= G.boardSize || pos.y < 0 || pos.y >= G.boardSize);
}

stone* getUrdlStones(int i, point pos) {
    point stonePos;
    if (i % 4 == 0)
        stonePos = { pos.x, pos.y - 1 };
    else if (i % 4 == 1)
        stonePos = { pos.x + 1, pos.y };
    else if (i % 4 == 2)
        stonePos = { pos.x, pos.y + 1 };
    else
        stonePos = { pos.x - 1, pos.y };
    if (onBoard(stonePos))
        return &G.stones[stonePos.x][stonePos.y];
    else
        return NULL;
}

bool ko(point pos, bool check)
{
    int atari = 0;
    stone* atariStone = NULL;
    if (check) {
        for (int i = 0; i < 4; i++)
            if (getUrdlStones(i, pos) != NULL && getUrdlStones(i, pos)->placed && getUrdlStones(i, pos)->breaths == 1)
                atariStone = getUrdlStones(i, pos);
        if (atariStone != NULL && pos.x == G.koPos.x && pos.y == G.koPos.y && G.koRound + 1 == G.round && !atariStone->rootStone->child)
            return true;
    }
    else {
        G.koPos = pos;
        G.koRound = G.round;
    }
    return false;
}

void connectStones(point connector) {
    stone* rootStone = &G.stones[connector.x][connector.y];
    stone* currentStone, * lastStone = rootStone;
    for (int i = 0; i < 4; i++) {
        currentStone = getUrdlStones(i, connector);
        if (getUrdlStones(i, connector) == NULL || !currentStone->placed || currentStone->type != rootStone->type || currentStone->rootStone == rootStone)
            continue;
        currentStone = currentStone->rootStone;
        lastStone->child = currentStone;
        while (true) {
            currentStone->rootStone = rootStone;
            if (currentStone->child)
                currentStone = currentStone->child;
            else {
                lastStone = currentStone;
                break;
            }
        }
    }

}

int calcBreaths(stone* brStone) {
    brStone = brStone->rootStone;
    stone* closeStone;
    int boardCheck[MAXBOARDSIZE][MAXBOARDSIZE], brCount = 0;
    for (int i = 0; i < G.boardSize; i++)
        for (int j = 0; j < G.boardSize; j++)
            boardCheck[i][j] = false;
    while (true) {
        for (int i = 0; i < 4; i++) {
            closeStone = getUrdlStones(i, brStone->position);
            if (closeStone && !closeStone->placed)
                boardCheck[closeStone->position.x][closeStone->position.y] = true;
        }
        if (brStone->child)
            brStone = brStone->child;
        else
            break;
    }
    for (int i = 0; i < G.boardSize; i++)
        for (int j = 0; j < G.boardSize; j++)
            if (boardCheck[i][j])
                brCount++;
    brStone = brStone->rootStone;
    while (true) {
        brStone->breaths = brCount;
        if (brStone->child)
            brStone = brStone->child;
        else
            break;
    }
    return brCount;
}

void removeStone(stone* rmStone);
void updateStones(point pos, stoneTypes stoneType)
{
    for (int i = 0; i < 4; i++)
        if (getUrdlStones(i, pos) && getUrdlStones(i, pos)->placed && getUrdlStones(i, pos)->type == stoneType)
            if (!calcBreaths(getUrdlStones(i, pos)))
                removeStone(getUrdlStones(i, pos));
}

void removeStone(stone* rmStone) {
    rmStone = rmStone->rootStone;
    rmStone->placed = false;
    rmStone->type ? G.points.x++ : G.points.y++;
    if (!rmStone->rootStone->child)
        ko(rmStone->position, false);
    updateStones(rmStone->position, (stoneTypes)!rmStone->type);
    while (rmStone->child) {
        rmStone = rmStone->child;
        rmStone->placed = false;
        rmStone->type ? G.points.x++ : G.points.y++;
        updateStones(rmStone->position, (stoneTypes)!rmStone->type);
    }
}

bool canPlace(point pos) {
    stoneTypes stoneType = (stoneTypes)(G.round % 2);
    if (G.round == -1) stoneType = blackStone;
    if (G.stones[pos.x][pos.y].placed)
        return false;
    bool canSurvive = false;
    int empties = 0, sameClr = 0, atari = 0;
    for (int i = 0; i < 4; i++)
        if (getUrdlStones(i, pos) != NULL) {
            if (!getUrdlStones(i, pos)->placed)
                empties++;
            else if (getUrdlStones(i, pos)->type == stoneType) {
                sameClr++;
                if (getUrdlStones(i, pos)->breaths > 1)
                    canSurvive = true;
            }
            else if (getUrdlStones(i, pos)->breaths == 1)
                atari++;
        }
    if (!atari && !canSurvive && !empties || (ko(pos, true) && atari == 1))
        return false;
    return true;
}

bool placeStone(point pos) {
    stoneTypes stoneType = (stoneTypes)(G.round % 2);
    if (G.round == -1) stoneType = blackStone;
    if (!canPlace(pos))
        return false;
    G.stones[pos.x][pos.y] = { true, stoneType, 2, pos, &G.stones[pos.x][pos.y], NULL };
    connectStones(pos);
    calcBreaths(&G.stones[pos.x][pos.y]);
    updateStones(pos, (stoneTypes)!stoneType);
    return true;
}

int endCheck(point pos, bool checked[MAXBOARDSIZE][MAXBOARDSIZE], stoneTypes stoneType) {
    int ret = 0, retf;
    stone* urdl;
    bool isOther = false;
    if (G.stones[pos.x][pos.y].placed) {
        if (G.stones[pos.x][pos.y].type == stoneType)
            return 0;
        else
            return -1;
    }
    if (checked[pos.x][pos.y])
        return 0;
    checked[pos.x][pos.y] = true;
    for (int i = 0; i < 4; i++) {
        urdl = getUrdlStones(i, pos);
        if (urdl) {
            retf = endCheck(urdl->position, checked, stoneType);
            if (retf == -1)
                isOther = true;
            ret += retf;
        }
    }
    if (isOther) return -1;
    else return ret + 1;
}

void endGame() {
    int whC = 0, blC = 0, p;
    bool whiteA[MAXBOARDSIZE][MAXBOARDSIZE], blackA[MAXBOARDSIZE][MAXBOARDSIZE];
    for (int i = 0; i < G.boardSize; i++)
        for (int j = 0; j < G.boardSize; j++) {
            whiteA[i][j] = false;
            blackA[i][j] = false;
        }
    for (int i = 0; i < G.boardSize; i++)
        for (int j = 0; j < G.boardSize; j++) {
            p = endCheck({ i, j }, whiteA, whiteStone);
            if (p != -1) whC += p;
            p = endCheck({ i, j }, blackA, blackStone);
            if (p != -1) blC += p;
        }
    G.points.x += whC;
    G.points.y += blC;
    G.ended = true;
    log("game over", 100);
}

bool canEnd() {
    bool whiteA[MAXBOARDSIZE][MAXBOARDSIZE], blackA[MAXBOARDSIZE][MAXBOARDSIZE];
    for (int i = 0; i < G.boardSize; i++)
        for (int j = 0; j < G.boardSize; j++) {
            whiteA[i][j] = false;
            blackA[i][j] = false;
        }
    for (int i = 0; i < G.boardSize; i++)
        for (int j = 0; j < G.boardSize; j++) {
            if ((-1 == endCheck({ i, j }, whiteA, whiteStone)) && (-1 == endCheck({ i, j }, blackA, blackStone)) && (!G.stones[i][j].placed))
                return false;
        }
    return true;
}

void generateAndRender(point cursorPos, bool reRender);
int main() {
    static gameStateS lastG;
    char fName[24];
    gameStateS state;
    settitle("GO");
    srand(time(NULL));
    G.boardSize = 1;
    point cursorPos = { G.boardSize / 2, G.boardSize / 2 };
    generateAndRender(cursorPos, true);
    newGame(false);
    cursorPos = { G.boardSize / 2, G.boardSize / 2 };
    generateAndRender(cursorPos, true);
    char prch;

    signal(SIGINT, handleSignnal);
    signal(SIGINT, handleSignnal);
    signal(SIGTERM, handleSignnal);
    while (true) {
        prch = getch();
        if (prch >= 'A' && prch <= 'Z') prch += 'a' - 'A';
        if (prch == 'q') {
            quitGame();
            return 0;
        }
        else if (prch == 'n') {
            newGame();
            cursorPos = { G.boardSize / 2, G.boardSize / 2 };
        }
        else if (prch == 'r') generateAndRender(cursorPos, true);
        else if (prch == 0) {
            prch = getch();
            if (prch == 0x48 && onBoard({ cursorPos.x, cursorPos.y - 1 })) cursorPos.y--;
            else if (prch == 0x50 && onBoard({ cursorPos.x, cursorPos.y + 1 })) cursorPos.y++;
            else if (prch == 0x4b && onBoard({ cursorPos.x - 1, cursorPos.y })) cursorPos.x--;
            else if (prch == 0x4d && onBoard({ cursorPos.x + 1, cursorPos.y })) cursorPos.x++;
        }
        else if (G.ended) continue;
        else if (prch == 's') {
            save(&state, G);
            scan(fName, { MOFFSETX + 1, MOFFSETY + 20 }, 20, "enter name(max 20):", true);
            FILE* savef = fopen(fName, "wb+");
            if (fwrite(&state, sizeof(gameStateS), 1, savef)) log("successfuly saved", 5);
            else log("error occured", 5);
            fclose(savef);
        }
        else if (prch == 'l') {
            scan(fName, { MOFFSETX + 1, MOFFSETY + 20 }, 20, "enter name:", true);
            FILE* savef = fopen(fName, "rb");
            if (fread(&state, sizeof(gameStateS), 1, savef)) {
                load(&G, state);
                log("successful loading", 5);
                cursorPos = { G.boardSize / 2, G.boardSize / 2 };
            }
            else log("error occured", 5);
            fclose(savef);
        }
        else if (prch == 'e' && G.round == -1) {
            G.komi = HANDICAPKOMI;
            if (!placeStone(cursorPos)) continue;
        }
        else if (prch == 13 && G.round == -1) G.round = 1;
        else if (prch == 'e') {
            save(&lastG, G);
            if (!placeStone(cursorPos)) continue;
            if (SUBMIT) {
                generateAndRender(cursorPos, false);
                while (true) {
                    char sbt = getch();
                    if (sbt == 13) G.round++;
                    else if (sbt == 27) load(&G, lastG);
                    else continue;
                    break;
                }
            }
            else G.round++;
            if (AUTOEND && canEnd() && G.round > 2) endGame();
        }
        else if (prch == 'f' && G.round > 2) endGame();
        else if (prch == 'g') G.round++;
        generateAndRender(cursorPos, false);
    }
    return 0;
}

void print(colors up, colors down, char c, point xy, bool reRender = false) {
    struct letter {
        char c;
        colors up;
        colors down;
    };
    static letter buffer[320][90];
    if (buffer[xy.x][xy.y].up != up || buffer[xy.x][xy.y].down != down || buffer[xy.x][xy.y].c != c || reRender) {
        textattr(up + 16 * down);
        gotoxy(xy.x, xy.y);
        putch(c);
        textattr(white);
        buffer[xy.x][xy.y] = { c, up, down };
    }
}

void generateAndRender(point cursorPos, bool reRender) {
    const int boardBitSize = (G.boardSize * 6 - 1);
    const point bitmapSize = { boardBitSize + INNEROFFSET * 2 + 2, boardBitSize + 1 + INNEROFFSET * 2 + 2 };
    static text_info ti;
    static point pr = { 0, 0 };
    if (reRender) {
        Conio2_Init();
        _setcursortype(_NOCURSOR);
        gettextinfo(&ti);
        pr = { 0, 0 };
        system("CLS");
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    int offX;
    if (OFFSETX > (ti.screenwidth - bitmapSize.x) / 2 - 1)
        offX = OFFSETX;
    else
        offX = (ti.screenwidth - bitmapSize.x) / 2 - 1;
    const point offset = { offX, OFFSETY };
    const char stoneMap[6][6] = { "01110", "11111", "11111", "11111", "01110" };
    const char atariMap[6][6] = { "01110", "11111", "11011", "11111", "01110" };
    colors bitmap[MAXBOARDSIZE * 6 + INNEROFFSET * 2 + 2][MAXBOARDSIZE * 6 + INNEROFFSET * 2 + 3];
    for (int y = 0; y < bitmapSize.y; y++)
        for (int x = 0; x < bitmapSize.x; x++) {
            if (y < INNEROFFSET * 2 + boardBitSize + 2)
                bitmap[y][x] = (EXPXY) ? FRAMECOLOR : FRAMECOLOR2;
            else
                bitmap[y][x] = black;
            if (((y - INNEROFFSET - 1) % 6 == 2 || (x - INNEROFFSET - 1) % 6 == 2) && !(x<3 + INNEROFFSET || y<3 + INNEROFFSET || x>boardBitSize - 2 + INNEROFFSET || y>boardBitSize - 2 + INNEROFFSET))
                bitmap[y][x] = LINECOLOR;
            else if (x >= INNEROFFSET && x < bitmapSize.x - INNEROFFSET && y >= INNEROFFSET && y < bitmapSize.y - INNEROFFSET - 1)
                bitmap[y][x] = BOARDCOLOR;
        }
    for (int i = 0; i < G.boardSize; i++)
        for (int j = 0; j < G.boardSize; j++)
            for (int y = 0; y < 5; y++)
                for (int x = 0; x < 5; x++)
                    if (G.stones[j][i].placed && (G.stones[j][i].breaths > 1 ? stoneMap[y][x] - '0' : atariMap[y][x] - '0'))
                        bitmap[i * 6 + y + INNEROFFSET + 1][j * 6 + x + INNEROFFSET + 1] = (G.stones[j][i].type ? BLACKSTONECOLOR : WHITESTONECOLOR);
    const char CShape[6][6] = { "01110", "10001", "10101", "10001", "01110" };
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            if (CShape[i][j] - '0')
                bitmap[cursorPos.y * 6 + i + INNEROFFSET + 1][cursorPos.x * 6 + j + INNEROFFSET + 1] = CURSORCOLOR;
    ///MENU
    char bl[30], cz[30];
    sprintf(bl, "%9s %-6.1lf", "White:", (double)G.points.x + G.komi);
    sprintf(cz, "%9s %-6.1lf", "Black:", (double)G.points.y);
    for (int x = ti.screenwidth / 2 - 35; x <= ti.screenwidth / 2 + 35; x++)
        for (int y = 1; y <= 2; y++) {
            if (y == 1 && x > ti.screenwidth / 2 - 10 && x < ti.screenwidth / 2 + 10)
                print((G.round % 2 ? BLACKSTONECOLOR : WHITESTONECOLOR), gray, 254, { x, y }, reRender);
            else if (y == 1 && x > ti.screenwidth / 2 - 17 && x < ti.screenwidth / 2 + 17)
                print(gray, gray, 223, { x, y }, reRender);
            else if (y == 1 && x > ti.screenwidth / 2 - 33 && x < ti.screenwidth / 2)
                print(black, lGray, cz[x - ti.screenwidth / 2 + 32], { x, y }, reRender);
            else if (y == 1 && x<ti.screenwidth / 2 + 33 && x>ti.screenwidth / 2)
                print(black, lGray, bl[x - ti.screenwidth / 2 - 17], { x, y }, reRender);
            else if (y == 2)
                print(gray, black, 223, { x, y }, reRender);
            else
                print(gray, red, 223, { x, y }, reRender);
        }
    while (cursorPos.x - pr.x <= 0 && pr.x > 0) pr.x--;
    while (cursorPos.x - pr.x > (ti.screenwidth - OFFSETX) / 6 - 3 && bitmapSize.x + OFFSETX - pr.x * 6 > ti.screenwidth) pr.x++;
    while (cursorPos.y - pr.y <= 0 && pr.y > 0) pr.y--;
    while (cursorPos.y - pr.y > ((ti.screenheight - OFFSETY) / 3 - 3) && bitmapSize.y / 2 + OFFSETY - pr.y * 3 > ti.screenheight) pr.y++;
    for (int y = pr.y * 6; OFFSETY + 1 + y / 2 - pr.y * 3 <= ti.screenheight; y += 2)
        for (int x = pr.x * 6; OFFSETX + 1 + x - pr.x * 6 <= ti.screenwidth; x++) {
            if (x < bitmapSize.x && y < bitmapSize.y)
                print(bitmap[y][x], bitmap[y + 1][x], 223, { OFFSETX + 1 + x - pr.x * 6, OFFSETY + 1 + y / 2 - pr.y * 3 }, reRender);
            else //if((pr.x>0&&y<bitmapSize.y)||(pr.y>0&&x<bitmapSize.x))
                print(black, black, 223, { OFFSETX + 1 + x - pr.x * 6, OFFSETY + 1 + y / 2 - pr.y * 3 }, false);
        }
    char str[40];
    if (G.round != -1) sprintf(str, "Round: %-3d                          ", G.round);
    else sprintf(str, "Game state editor ([Enter]-confirm)");
    gotoxy(MOFFSETX + 1, MOFFSETY + 7);
    cputs(str);
    sprintf(str, "Cursor position: %c%d (x:%d, y:%d)    ", cursorPos.x + 'A', G.boardSize - (cursorPos.y), cursorPos.x + 1, G.boardSize - (cursorPos.y));
    gotoxy(MOFFSETX + 1, MOFFSETY + 5);
    cputs(str);

    if (reRender) {
        gotoxy(MOFFSETX + 1, MOFFSETY + 1);
        cputs("Damian (d4m14n) Trowski, 193443");
        gotoxy(MOFFSETX + 1, MOFFSETY + 3);
        cputs("Implementation: a-n");
        gotoxy(MOFFSETX + 1, MOFFSETY + 4);
        for (int i = 0; i < 30; i++)
            putch(196);
        gotoxy(MOFFSETX + 1, MOFFSETY + 9);
        cputs("Controls: ");
        gotoxy(MOFFSETX + 3, MOFFSETY + 11);
        cputs("Arrows: moving the cursor");
        gotoxy(MOFFSETX + 3, MOFFSETY + 12);
        cputs("[Q]: Quit");
        gotoxy(MOFFSETX + 3, MOFFSETY + 13);
        cputs("[N]: New game");
        gotoxy(MOFFSETX + 3, MOFFSETY + 14);
        cputs("[E]: Placing a stone");
        gotoxy(MOFFSETX + 3, MOFFSETY + 15);
        cputs("[S]: Saving game state");
        gotoxy(MOFFSETX + 3, MOFFSETY + 16);
        cputs("[L]: Loading game state");
        gotoxy(MOFFSETX + 3, MOFFSETY + 17);
        cputs("[R]: Refresh");
        if (SUBMIT) {
            gotoxy(MOFFSETX + 3, MOFFSETY + 18);
            cputs("[Esc]: cancel");
            gotoxy(MOFFSETX + 3, MOFFSETY + 19);
            cputs("[Enter]: confirm");
        }
    }
    log("", -1);
}
