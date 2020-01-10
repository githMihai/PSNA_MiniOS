#include "screen.h"

static PSCREEN gVideo = (PSCREEN)(0x000B8000);
static COLOR color = COLOR_LIGHT_GREEN;

void SetColor(BYTE Color)
{
	color = Color;
}

void CursorMove(int row, int col)
{
    unsigned short location = (row * MAX_COLUMNS) + col;       /* Short is a 16bit type , the formula is used here*/

    //Cursor Low port
    __outbyte(0x3D4, 0x0F);                                    //Sending the cursor low byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)(location & 0xFF));

    //Cursor High port
    __outbyte(0x3D4, 0x0E);                                    //Sending the cursor high byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)((location >> 8) & 0xFF)); //Char is a 8bit type
}

void CursorPosition(int pos)
{
    int row, col;

    if (pos > MAX_OFFSET)
    {
        pos = pos % MAX_OFFSET;
    }

    row = pos / MAX_COLUMNS;
    col = pos % MAX_COLUMNS;

    CursorMove(row, col);
}

void HelloBoot()
{
    int i, len;
	char boot[] = "Hello Boot! Greetings from C...";

	len = 0;
	while (boot[len] != 0)
	{
		len++;
	}

	for (i = 0; (i < len) && (i < MAX_OFFSET); i++)
	{
		gVideo[i].color = color;
		gVideo[i].c = boot[i];
	}
    CursorPosition(i);
}

void ClearScreen()
{
    int i;

    for (i = 0; i < MAX_OFFSET; i++)
    {
        gVideo[i].color = color;
        gVideo[i].c = 0;
    }

    CursorMove(0, 0);
}

void ClearPos(int Pos)
{
	gVideo[Pos].color = color;
	gVideo[Pos].c = 0;
}

void ClearLine(int Pos)
{
	int i, linePos;
	linePos = (Pos / MAX_COLUMNS)* MAX_COLUMNS;
	for (i = linePos; i < MAX_COLUMNS; i++)
	{
		gVideo[i].color = color;
		gVideo[i].c = 0;
	}
	CursorPosition(linePos);
}

void ScreenShiftLeft(int Pos)
{
	int i;
	while (gVideo[Pos + 1].c != 0)
	{
		gVideo[Pos].color = color;
		gVideo[Pos].c = gVideo[Pos + 1].c;
		Pos++;
	}
	gVideo[Pos].c = 0;
}

void CopyFromScreen(char* dst, int Pos)
{
	int i = 0;
	while (gVideo[Pos].c != 0)
	{
		dst[i++] = gVideo[Pos++].c;
	}
	dst[i] = gVideo[Pos].c;
}


void debugPrint(char* buf, int len)
{
	//if (DEBUG)
	{
		int i, j;
		j = GetCursorPosition();

		for (i = 0; (i < len) && (i < MAX_OFFSET); i++)
		{
			gVideo[j].color = color;
			gVideo[j].c = buf[i];
			j++;
		}
		//CursorPosition(j);
		NewLine();
		//__magic();
	}
}

void NewLine()
{
	int pos = GetCursorPosition();
	CursorPosition((pos / 80 + 1) * 80);
}

WORD GetCursorPosition(void)
{
	WORD pos = 0;
	__outbyte(0x3D4, 0x0F);
	pos |= __inbyte(0x3D5);
	__outbyte(0x3D4, 0x0E);
	pos |= ((WORD)__inbyte(0x3D5)) << 8;
	return pos;
}

WORD GetLinePosition(void)
{
	int pos = GetCursorPosition();
	pos = (pos / 80) * 80;
	return pos;
}

void PutCharAtPos(char C, int Pos)
{
	gVideo[Pos].color = color;
	gVideo[Pos].c = C;
	CursorPosition(++Pos);
}

void PutChar(char c)
{
	if (c == '\n')
	{
		NewLine();
	}
	else
	{
		PutCharAtPos(c, GetCursorPosition());
	}
}

void PutBuf(char* buf, int len)
{
	while (len-- > 0)
	{
		PutChar(*buf++, GetCursorPosition());
	}
}

void PutString(char* buf)
{
	while (*buf != '\0')
	{
		PutChar(*buf);
		buf++;
	}
}

void PutStringPreservePos(char* String)
{
	int pos, i;
	pos = GetCursorPosition();
	while (*String != '\0')
	{
		gVideo[pos++].c = *String++;
	}
}