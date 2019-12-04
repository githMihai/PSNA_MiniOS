#include "console.h"
#include "keyboard.h"
#include "string.h"
#include "timer.h"
#include "chars.h"
#include "rtc.h"

#define NUMBER_OF_EXEC	4

//static int CurrentPos = 0;
char consoleString[MAX_CONSOLE_CHARS] = { 0 };
char auxString[MAX_CONSOLE_CHARS] = { 0 };
char prompt[] = "@root: ";
char com[20] = { 0 };
char args[5][20] = { 0 };

static CONSOLE console;
static DATE d;
static QWORD time;

static EXEC_STRUCT execs[] =
{
	{"echo", { 0 }},
	{"get_time", {"-c", "", "", "", ""}},
	{"get_date", {"-c", "", "", "", ""}},
	{"sleep", {"-m", "-s", "", ""}}
};


char commandStrings[100][255];
COMMAND_STRUCT commandStructs[100];

const char* commands[] =
{
	"echo",
	"get_time",
	"get_date"
};

int GetExec(COMMAND_STRUCT* cmd)
{
	int com;
	for (com = 0; com < NUMBER_OF_EXEC; com++)
	{
		if (!strcmp(execs[com].name, cmd->command))
		{
			return com;
		}
	}
	return -1;
}

PFUNC function(KeyStatus k)
{
	switch (console.StateMachine)
	{
	/*case NEW_COMMAND:
	{
		console.StateMachine = WAITING_FOR_COMMAND;
	}*/
	case WAITING_FOR_COMMAND:
	{
		int foundPos;
		BYTE key = 0;
		if (k.Pressed && !k.Released)
		{
			switch (k.key)
			{
			case KEY_LEFT:		{ Left(); break; }
			case KEY_RIGHT:		{ Right(); break; }
			case KEY_UP:		{ Up(); break; }
			case KEY_DOWN:		{Down(); break; }
			case KEY_DELETE:	{Delete(console.commands + console.commandIdx); break; }
			case KEY_KP_ENTER:
			{
				console.StateMachine = PROCESS_COMMAND;
				NewLine();
				//newCommand();
				break;
			}
			default:
			{
				switch (scancode[k.key])
				{
				case KEY_BACKSPACE:	
				{
					BackSpace(console.commands + console.commandIdx); 
					break;
				}
				default:
				{
					if (scancode[k.key] == KEY_C && k.Ctrl)
					{
						console.StateMachine = EXIT_COMMAND;
						return;
					}
					foundPos = findChar(unshifted, scancode[k.key]);
					if (foundPos >= 0)
					{
						if (k.Shift)
						{
							key = shifted[foundPos];
							if (k.CapsLock)
							{
								key = toLower(key);
							}
						}
						else
						{
							if (k.CapsLock)
							{
								key = toUpper(key);
							}
							key = unshifted[foundPos];
						}
						WriteIntoComand(key, console.commands + console.commandIdx);
					}
				}
				}
			}
			}
		}
		break;
	}
	case PROCESS_COMMAND:
	{
		if (scancode[k.key] == KEY_C && k.Ctrl)
		{
			console.StateMachine = EXIT_COMMAND;
			return;
		}
	}
	case EXECUTE_COMMAND:
	{
		if (scancode[k.key] == KEY_C && k.Ctrl)
		{
			console.StateMachine = EXIT_COMMAND;
			return;
		}
	}
	case EXIT_COMMAND:
	{
		if (scancode[k.key] == KEY_C && k.Ctrl)
		{
			console.StateMachine = EXIT_COMMAND;
			return;
		}
	}
	}
}

void InitConsole()
{
	int i;
	console.width = MAX_COLUMNS;
	console.height = MAX_LINES;
	console.fontSize = 1;
	console.CurrentPos = 0;
	console.EndPos = 0;
	console.LineOffset = 0;
	console.Color = COLOR_WHITE;
	console.consoleString = consoleString;
	console.auxString = auxString;
	console.prompt = prompt;
	console.PromptColor = COLOR_LIGHT_GREEN;
	console.StateMachine = NEW_COMMAND;
	/*for (i = 0; i < 100; i++)
	{
		commandStructs[i].commandString = commandStrings[i];
	}*/
	console.commands = commandStructs;
	console.commandIdx = -1;
	console.NumberOfCommands = 0;
	//newCommand();
	KeyboardSubscribe(function);
}

void nextCommand()
{
	if (console.commandIdx < console.NumberOfCommands)
	{
		console.commandIdx++;
		console.Ccommand = &console.commands[console.commandIdx];
	}
}

void prevCommand()
{
	if (console.commandIdx > 0)
	{
		console.commandIdx--;
		console.Ccommand = &console.commands[console.commandIdx];
	}
}

void newCommand()
{
	console.commandIdx++;
	console.NumberOfCommands++;
	console.Ccommand = &console.commands[console.commandIdx];
	console.Ccommand->CurrentPos = 0;
	console.Ccommand->EndPos = 0;
	console.Ccommand->argc = 0;
	console.Ccommand->CStartPos = console.CurrentPos + strlen(console.prompt);
	console.Ccommand->CEndPos = console.CurrentPos + strlen(console.prompt);
	CommandToScreen(console.Ccommand);
}

void BackSpace(COMMAND_STRUCT* cmd)
{
	int i;
	int pos;
	if (cmd->CurrentPos > 0)
	{
		pos = GetCursorPosition() - 1;
		removeChar(cmd->commandString, cmd->CurrentPos--);
		cmd->EndPos--;
		CommandToScreen(cmd);
		CursorPosition(pos);
	}
}

void Delete(COMMAND_STRUCT* cmd)
{
	int i;
	int pos;
	if (cmd->CurrentPos < cmd->EndPos)
	{
		pos = GetCursorPosition();
		removeChar(cmd->commandString, cmd->CurrentPos+1);
		cmd->EndPos--;
		CommandToScreen(cmd);
		CursorPosition(pos);
	}
}

void Left()
{
	if (console.Ccommand->CurrentPos > 0)
	{
		CursorPosition(GetCursorPosition() - 1);
		console.Ccommand->CurrentPos--;
	}
}

void Right()
{
	if (console.Ccommand->CurrentPos < console.Ccommand->EndPos)
	{
		CursorPosition(GetCursorPosition() + 1);
		console.Ccommand->CurrentPos++;
	}
}

void Up()
{
	prevCommand();
	CursorPosition(GetCursorPosition() + console.Ccommand->EndPos);
	CommandToScreen(console.Ccommand);
}

void Down()
{
	nextCommand();
	CursorPosition(GetCursorPosition() + console.Ccommand->EndPos);
	CommandToScreen(console.Ccommand);
}

void ExecuteConsole()
{
	int i, j, commandId, pos;
	while (TRUE)
	{
		switch (console.StateMachine)
		{
		case NEW_COMMAND:
		{
			newCommand();
			console.StateMachine = WAITING_FOR_COMMAND;
			break;
		}
		case WAITING_FOR_COMMAND:
		{
			mSleep(10);
			break;
		}
		case PROCESS_COMMAND:
		{
			processCommand(console.Ccommand);
			console.StateMachine = EXECUTE_COMMAND;
			break;
		}
		case EXECUTE_COMMAND:
		{
			commandId = GetExec(console.Ccommand);

			switch (commandId)
			{
			case ECHO:
			{
				sprintf(console.Ccommand->response, "%s", console.Ccommand->commandString + strlen(commands[ECHO]) + 1);
				printf("%s", console.Ccommand->response);
				break;
			}
			case GET_TIME:
			{
				if (console.Ccommand->argc > 0)
				{
					if (!strcmp(console.Ccommand->args[0], execs[GET_TIME].args[0]))
					{
						while (console.StateMachine == EXECUTE_COMMAND)
						{
							pos = GetCursorPosition();
							time = SysGetTime();
							sprintf(console.Ccommand->response, "%d", time);
							printf("%s", console.Ccommand->response);
							CursorPosition(pos);
							Sleep(1);
						}
					}
				}
				else
				{
					time = SysGetTime();
					sprintf(console.Ccommand->response, "%d", time);
					printf("%s", console.Ccommand->response);
				}
				break;
			}
			case GET_DATE:
			{
				//d.year = d.month = d.day = d.hour = d.minute = d.second = 0;
				if (console.Ccommand->argc > 0)
				{
					if (!strcmp(console.Ccommand->args[0], execs[GET_TIME].args[0]))
					{
						while (console.StateMachine == EXECUTE_COMMAND)
						{
							pos = GetCursorPosition();
							SysGetDate(&d);
							sprintf(console.Ccommand->response, "%d/%d/%d %d:%d:%d", d.year, d.month, d.day, d.hour, d.minute, d.second);
							printf("%s", console.Ccommand->response);
							CursorPosition(pos);
							mSleep(1000);
						}
					}
				}
				else
				{
					SysGetDate(&d);
					sprintf(console.Ccommand->response, "%d/%d/%d %d:%d:%d", d.year, d.month, d.day, d.hour, d.minute, d.second);
					printf("%s", console.Ccommand->response);
				}
				break;
			}
			case SLEEP:
			{
				if (console.Ccommand->argc == 2)
				{
					if (!strcmp(console.Ccommand->args[0], execs[SLEEP].args[0]))
					{
						QWORD time = convertToULong(console.Ccommand->args[1]);
						mSleep(time);
						break;
					}
					else
					if (!strcmp(console.Ccommand->args[0], execs[SLEEP].args[1]))
					{
						QWORD time = convertToULong(console.Ccommand->args[1]);
						Sleep(time * 1000);
						break;
					}
				}
			}
			default:
			{
				printf("Command \"%s\" not found.", console.Ccommand->command);
				break;
			}
			}
			console.StateMachine = EXIT_COMMAND;
			break;
		}
		case EXIT_COMMAND:
		{
			NewLine();
			console.StateMachine = NEW_COMMAND;
			break;
		}
		}
	}
}

void ClearCmdLine()
{
	ClearLine(console.commands[console.commandIdx].CurrentPos);
	SetColor(console.PromptColor);
	PutString(console.prompt);
	SetColor(console.Color);
}

void WriteIntoComand(char c, COMMAND_STRUCT* cmd)
{
	if (cmd->CurrentPos < cmd->EndPos)
	{
		insertChar(c, cmd->commandString, cmd->CurrentPos);
		cmd->CurrentPos++;
		cmd->EndPos = strlen(cmd->commandString);
		CommandToScreen(cmd);
	}
	else
	{
		cmd->commandString[cmd->CurrentPos] = c;
		cmd->CurrentPos++;
		cmd->EndPos = strlen(cmd->commandString);
		CommandToScreen(cmd);
	}
}

void processCommand(COMMAND_STRUCT* cmd)
{
	int i, j, x;
	i = j = x = 0;
	while (cmd->commandString[i] != ' ' && cmd->commandString[i] != 0)
	{
		cmd->command[i] = cmd->commandString[i];
		i++;
	}
	cmd->command[i] = 0;
	for (j = i + 1; j < cmd->EndPos; j++)
	{
		x = 0;
		while (cmd->commandString[j] != ' ' && cmd->commandString[j] != 0)
		{
			cmd->args[cmd->argc][x] = cmd->commandString[j];
			x++;
			j++;
		}
		cmd->args[cmd->argc][j - x] = 0;
		cmd->argc++;
	}
}

void CommandToScreen(COMMAND_STRUCT* cmd)
{
	int i, pos, eol;
	CursorPosition(GetCursorPosition() - cmd->CurrentPos);
	pos = GetLinePosition();
	CursorPosition(pos);
	eol = (pos + cmd->CurrentPos + strlen(console.prompt) / 80 + 1) * 80 - 1;
	for (i = pos; i < eol; i++)
	{
		ClearPos(i);
	}
	SetColor(console.PromptColor);
	PutString(console.prompt);
	SetColor(console.Color);
	PutString(cmd->commandString);
	CursorPosition(GetCursorPosition() - (cmd->EndPos - cmd->CurrentPos));
}