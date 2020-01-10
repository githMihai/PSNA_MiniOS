#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "screen.h"
#include "keyboard.h"

#define MAX_CONSOLE_CHARS 8000

#pragma pack(push)
#pragma pack(1)
typedef enum _COMMANDS
{
	ECHO			= 0,
	GET_TIME		= 1,
	GET_DATE		= 2,
	SLEEP			= 3,
	DISK			= 4,
	MEMORY			= 5,
	ZERO_DIV		= 6,
	PAGE_FAULT		= 7,
	CLEAR_SCREEN	= 8,
	TEST_MEMORY		= 9,
}COMMANDS;

typedef enum _WAITING_COMMAND_STATE_MACHINE
{
	WAIT				= 0,
	PROCESS_CHAR		= 1,
	PUT_CHAR_IN_COMMAND = 2,
	PUT_CHAR_IN_CONSOLE = 3,
	MOVE_CURSOR_RIGHT	= 4,
	MOVE_CURSOR_LEFT	= 5,
	BACKSPACE			= 6,
	DELETE				= 7,
}WAITING_COMMAND_STATE_MACHINE;

typedef enum _CONSOLE_STATE_MACHINE
{
	NEW_COMMAND				= 0,
	WAITING_FOR_COMMAND		= 1,
	PROCESS_COMMAND			= 2,
	EXECUTE_COMMAND			= 3,
	EXIT_COMMAND			= 4,
}CONSOLE_STATE_MACHINE;

typedef struct _COMMAND_STRUCT
{
	char commandString[255];
	char response[1000];
	char command[20];
	char args[7][20];
	int argc;
	int length;
	int CurrentPos;
	int EndPos;
	int CStartPos;
	int CEndPos;
}COMMAND_STRUCT;

typedef struct _EXEC_STRUCT
{
	char name[20];
	char args[7][20];
}EXEC_STRUCT;

typedef struct _CONSOLE
{
	int width;
	int height;
	int fontSize;
	int CurrentPos;
	int EndPos;
	int LineOffset;
	BYTE Color;
	char *consoleString;
	char *auxString;
	char** comands;
	char* prompt;
	BYTE PromptColor;
	int StateMachine;
	int WaitCommandSM;
	KeyStatus keyState;
	BOOLEAN Signal;
	COMMAND_STRUCT* commands;
	int commandIdx;
	int NumberOfCommands;
	COMMAND_STRUCT* Ccommand;
}CONSOLE;
#pragma pack(pop)

void InitConsole();
void BackSpace(COMMAND_STRUCT* cmd);
void Delete(COMMAND_STRUCT* cmd);
void Left();
void Right();
void Up();
void Down();

void nextCommand();
void prevCommand();
void newCommand();

void processCommand(COMMAND_STRUCT* cmd);

void ClearCmdLine();
void CommandToScreen(COMMAND_STRUCT* cmd);

void WriteIntoComand(char c, COMMAND_STRUCT* cmd);

void ExecuteConsole();

int divideByZero(int param);

#endif // _CONSOLE_H_