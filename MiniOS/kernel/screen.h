#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "main.h"

#define MAX_LINES       25
#define MAX_COLUMNS     80
#define MAX_OFFSET      2000 //25 lines * 80 chars


#pragma pack(push)
#pragma pack(1)
typedef enum _COLOR
{
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_PURPLE = 5,
	COLOR_BROWN = 6,
	COLOR_GRAY = 7,
	COLOR_DARK_GRAY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_PURPLE = 13,
	COLOR_YELLOW = 14,
	COLOR_WHITE = 15
}COLOR;

typedef struct _SCREEN
{
    char c;
    BYTE color;
}SCREEN, *PSCREEN;
#pragma pack(pop)

void HelloBoot();

void SetColor(BYTE Color);

void SetColor(BYTE Color);
void CursorMove(int row, int col);
void CursorPosition(int pos);
void ClearScreen();
void ClearPos(int Pos);
void ClearLine(int Pos);
void ScreenShiftLeft(int Pos);
void ScreenShiftRight(int Pos);
void CopyFromScreen(char* dst, int Pos);

WORD GetCursorPosition(void);
WORD GetLinePosition(void);
void NewLine();
void debugPrint(char* buf, int len);
void PutCharAtPos(char C, int Pos);
void PutStringAtPos(char* String, int Pos);
void PutStringLine(char* String, int Line);
void PutStringPreservePos(char* String);

void PutChar(char c);
void PutBuf(char* buf, int len);
void PutString(char* buf);


#endif // _SCREEN_H_