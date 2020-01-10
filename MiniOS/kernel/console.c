#include "console.h"
#include "keyboard.h"
#include "string.h"
#include "timer.h"
#include "chars.h"
#include "rtc.h"
#include "ata.h"
#include "memory.h"

#define NUMBER_OF_EXEC	10

//static int CurrentPos = 0;
char consoleString[MAX_CONSOLE_CHARS] = { 0 };
char auxString[MAX_CONSOLE_CHARS] = { 0 };
char prompt[] = "@root: ";
char com[20] = { 0 };
char args[7][20] = { 0 };

static CONSOLE console;
static DATE d;
static QWORD time;

static EXEC_STRUCT execs[] =
{
	{"echo",		{"", "", "", "", "", "", "-h"}},
	{"get_time",	{"-c", "", "", "", "", "", "-h"}},
	{"get_date",	{"-c", "", "", "", "", "", "-h"}},
	{"sleep",		{"-m", "-s", "", "", "", "-h"}},
	{"disk",		{"-r", "-w", "-sects", "-lba", "-buf", "-bufSize", "-h"}},
	{"mem",			{"-alloc", "-free", "-p", "-b", "", "", "-h"}},
	{"div-zero",	{"", "", "", "", "", "", "-h"}},
	{"page-fault",	{"", "", "", "", "", "", "-h"}},
	{"clear",		{"", "", "", "", "", "", "-h"}},
	{"test-memory",	{"", "", "", "", "", "", "-h"}},
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
				if ((console.Ccommand->argc == 1) && 
					(!strcmp(console.Ccommand->args[0], execs[ECHO].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   echo [STRING]");
					break;
				}
				// Echo
				sprintf(console.Ccommand->response, "%s", console.Ccommand->commandString + strlen(commands[ECHO]) + 1);
				printf("%s", console.Ccommand->response);
				break;
			}
			case GET_TIME:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[GET_TIME].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   get_time      # without arguments and returns current time\n");
					printf("   get_time -c   # show current time in continous mode\n");
					break;
				}
				if (console.Ccommand->argc > 0)
				{
					if (!strcmp(console.Ccommand->args[0], execs[GET_TIME].args[0]))
					{
						while (console.StateMachine == EXECUTE_COMMAND)
						{
							// Get Current Time in continous mode	"-c"
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
					// Get Current Time
					time = SysGetTime();
					sprintf(console.Ccommand->response, "%d", time);
					printf("%s", console.Ccommand->response);
				}
				break;
			}
			case GET_DATE:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[GET_DATE].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   get_date      # without arguments and returns current date\n");
					printf("   get_date -c   # show current date in continous mode\n");
					break;
				}
				//d.year = d.month = d.day = d.hour = d.minute = d.second = 0;
				if (console.Ccommand->argc > 0)
				{
					if (!strcmp(console.Ccommand->args[0], execs[GET_TIME].args[0]))
					{
						// Get current date contonously		"-c"
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
					// Get current date
					SysGetDate(&d);
					sprintf(console.Ccommand->response, "%d/%d/%d %d:%d:%d", d.year, d.month, d.day, d.hour, d.minute, d.second);
					printf("%s", console.Ccommand->response);
				}
				break;
			}
			case SLEEP:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[SLEEP].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   sleep -m [MILLISECONDS]  # Sleeps for MILLISECONDS milliseconds\n");
					printf("   sleep -s [SECONDS]       # Sleeps for SECONDS seconds\n");
					break;
				}
				if (console.Ccommand->argc == 2)
				{
					if (!strcmp(console.Ccommand->args[0], execs[SLEEP].args[0]))
					{
						// Sleep for milliseconds	"-m"
						QWORD time = convertToULong(console.Ccommand->args[1]);
						mSleep(time);
						break;
					}
					else
					if (!strcmp(console.Ccommand->args[0], execs[SLEEP].args[1]))
					{
						// Sleep for seconds	"-s"
						QWORD time = convertToULong(console.Ccommand->args[1]);
						Sleep(time * 1000);
						break;
					}
				}
				break;
			}
			case DISK:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[DISK].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   disk -r [SECTORS] [LBA] [BUFFER] [BUF_SIZE] # read SECTORS sectors starting from LBA intro BUFFER address\n");
					printf("   disk -w [SECTORS] [LBA] [BUFFER] {BUF_SIZE] # write SECTORS sectors starting from LBA from BUFFER address\n");	
					break;
				}
				/* {"disk",		{"-r", "-w", "-sects", "-lba", "-buf", "-bufSize", "-h"}} */
				if (console.Ccommand->argc != 5)
				{
					printf("Not enough arguments, %d \n", console.Ccommand->argc);
					break;
				}
				if (!strcmp(console.Ccommand->args[0], execs[DISK].args[0]))
				{
					// Disk read sectors	"-r"
					BYTE sectors = convertToByte(console.Ccommand->args[1]);
					unsigned int lba = convertToUInt(console.Ccommand->args[2]);
					/*QWORD bufferAddr = convertToQWord(console.Ccommand->args[3]);*/
					QWORD bufferAddr = converHexToQWORD(console.Ccommand->args[3]);
					WORD* buffer = bufferAddr;
					printf("bufferAddr: %x\n", buffer);
					unsigned int bufferSize = convertToUInt(console.Ccommand->args[4]);
					ide_read_sectors(0, sectors, lba, buffer, bufferSize);
					break;
				}
				if (!strcmp(console.Ccommand->args[0], execs[DISK].args[1]))
				{
					// Disk write sectors	"-w"
					BYTE sectors = convertToByte(console.Ccommand->args[1]);
					unsigned int lba = convertToUInt(console.Ccommand->args[2]);
					QWORD bufferAddr = convertToQWord(console.Ccommand->args[3]);
					WORD* buffer = bufferAddr;
					printf("bufferAddr: %x\n", buffer);
					unsigned int bufferSize = convertToUInt(console.Ccommand->args[4]);
					ide_write_sectors(0, sectors, lba, buffer, bufferSize);
					break;
				}
				break;
			}
			case MEMORY:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[MEMORY].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   mem -alloc -p [PAGES]  # Allocate PAGES pages. Returns pointer to space.\n");
					printf("   mem -alloc -b [SIZE]   # Allocate a block of SIZE dimension. Returns pointer to space.\n");
					printf("   mem -free -p [PAGE]    # Frees page with number PAGE.\n");
					printf("   mem -free -b [ADDRESS] # Frees the block in which the address is found.\n");
					break;
				}
				/* {"mem",			{"-alloc", "-free", "-p", "-b", "", "", "-h"}}, */
				if (console.Ccommand->argc != 3)
				{
					printf("Not enough arguments, %d \n", console.Ccommand->argc);
					break;
				}
				if (!strcmp(console.Ccommand->args[0], execs[MEMORY].args[0]))
				{
					// Allocate memory
					if (!strcmp(console.Ccommand->args[1], execs[MEMORY].args[2]))
					{
						// Alloc pages
						int numberOfPages = convertToUInt(console.Ccommand->args[2]);
						QWORD* ref = PageAlloc(numberOfPages);
						if (NULL == ref)
						{
							printf("No page allocated\n");
							break;
						}
						printf("%d pages allocated. First page begins at address: %x.\n", numberOfPages, ref);
						break;
					}
					if (!strcmp(console.Ccommand->args[1], execs[MEMORY].args[3]))
					{
						// Alloc blocks
						int size = convertToUInt(console.Ccommand->args[2]);
						QWORD* ref = MemBlockAlloc(size);
						if (NULL == ref)
						{
							printf("Can not allocate a block of size: %d.\n", size);
							break;
						}
						printf("%dB allocated at %x.\n", size, ref);
						break;
					}
					break;
				}
				if (!strcmp(console.Ccommand->args[0], execs[MEMORY].args[1]))
				{
					// Free memory
					if (!strcmp(console.Ccommand->args[1], execs[MEMORY].args[2]))
					{
						// Free page
						int pageNumber = convertToUInt(console.Ccommand->args[2]);
						PageFree(pageNumber);
						printf("Page %d freed\n", pageNumber);
						break;
					}
					if (!strcmp(console.Ccommand->args[1], execs[MEMORY].args[3]))
					{
						// Free blocks
						QWORD* addr;
						addr = converHexToQWORD(console.Ccommand->args[2]);
						printf("Address %x\n", addr);
						MemBlockFree(addr);
						break;
					}
					break;
				}
				break;
			}
			case ZERO_DIV:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[ZERO_DIV].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   div-zero  # Divide a number with zero and rise an exception.\n");
					break;
				}
				int a = divideByZero(0);
				printf("res %d\n", a);
				break;
			}
			case PAGE_FAULT:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[PAGE_FAULT].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   page-fault  # Access an address that is not mapped and rise an exception.\n");
					break;
				}
				int* a;
				a = 0x2000000;
				(*a)++;
				printf("%x\n", *a);
				break;
			}
			case CLEAR_SCREEN:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[CLEAR_SCREEN].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   clear  # Clear screen.\n");
					break;
				}
				ClearScreen();
				break;
			}
			case TEST_MEMORY:
			{
				if ((console.Ccommand->argc == 1) &&
					(!strcmp(console.Ccommand->args[0], execs[TEST_MEMORY].args[6])))
				{
					// Help
					printf("Command usage:\n");
					printf("   test-memory  # Execute a memory test. In the test 4 blocks of memory will be allocated:\n    b0: size: 1    B\n    b1: size: 10   B\n    b2: size: 4200 B\n    b3: size: 1000 B\n The value at location b2 + 10 will be moddified to 10 and then all the blocks will be freed.\n");
					break;
				}
				printf("Alloc block of size 1: ->  ");
				QWORD* address0 = MemBlockAlloc(1);
				if (address0 != NULL)
					printf("Memory Address = %x\n", address0);
				else
					printf("NULL\n");

				printf("Alloc block of size 10: ->  ");
				QWORD* address1 = MemBlockAlloc(10);
				if (address1 != NULL)
					printf("Memory Address = %x\n", address1);
				else
					printf("NULL\n");

				printf("Alloc block of size 4200: ->  ");
				QWORD* address2 = MemBlockAlloc(4200);
				if (address2 != NULL)
					printf("Memory Address = %x\n", address2);
				else
					printf("NULL\n");

				printf("Acccess address in last block: %x\n", address2 + 10);
				printf("  -> Value before: %d\n", (int)(*(address2 + 10)));
				(int)(*(address2 + 10)) = 10;
				printf("  -> Value after: %d\n", (int)(*(address2 + 10)));

				printf("Alloc block of size 1000: ->  ");
				QWORD* address3 = MemBlockAlloc(1000);
				if (address3 != NULL)
					printf("Memory Address = %x\n", address3);
				else
					printf("NULL\n");
				
				//printf("BitMap[0] addr: %x\n", (BitMap.bits[0]));
				printf("free %x\n", address2);
				MemBlockFree(address2);
				//printf("BitMap[0] after free: %x, addr: %x\n", address2, (BitMap.bits[0]));
				printf("free %x\n", address1);
				MemBlockFree(address1);
				//printf("BitMap[0] after free: %x, addr: %x\n", address1, (BitMap.bits[0]));
				printf("free %x\n", address0);
				MemBlockFree(address0);
				//printf("BitMap[0] after free: %x, addr: %x\n", address0, (BitMap.bits[0]));
				printf("free %x\n", address3);
				MemBlockFree(address3);
				break;
			}
			default:
			{
				printf("Command \"%s\" not found.", console.Ccommand->command);
				printf("Available commands:\n    ");
				for (i = 0; i < NUMBER_OF_EXEC; i++)
				{
					printf("%s\n    ", execs[i].name);
				}
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
		cmd->args[cmd->argc][x] = 0;
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

int divideByZero(int param)
{
	int n;
	n = 10 / param;
	//printf("Div  0 = %d\n", 10 / param);
	//return param;
	return n;
}