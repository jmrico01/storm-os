#include "monitor.h"

#include "screen.h"
#include "console.h"

#define WHITESPACE "\t\r\n "
#define MAX_ARGS 16

#define USER_PROC_ADDR 0x10000000

struct {
    int index;
	char buf[CONSOLE_BUFFER_SIZE];
} monitor;

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func) (int argc, char** argv);
};

static int MonitorHelp(int argc, char** argv);
static int MonitorTextColor(int argc, char** argv);
static int MonitorUser(int argc, char** argv);

static struct Command commands[] = {
    { "help", "Display this list of commands", MonitorHelp },
    { "textcolor", "Change the default text color", MonitorTextColor },
    { "user", "Start the user process", MonitorUser },
    /*{ "kerninfo", "Display information about the kernel", mon_kerninfo },
    { "runproc", "Run the dummy user process", mon_start_user },*/
};
#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

static int MonitorHelp(int argc, char** argv)
{
	for (int i = 0; i < (int)NUM_COMMANDS; i++) {
		Printf("%s - %s\n", commands[i].name, commands[i].desc);
    }

	return 0;
}

static int MonitorTextColor(int argc, char** argv)
{
    const char* colorNames[] = {
        "black",      "blue",           "green",        "cyan",
        "red",        "magenta",        "brown",        "bright-grey",
        "grey",       "bright-blue",    "bright-green", "bright-cyan",
        "bright-red", "bright-magenta", "yellow",       "white"
    };
    char spaces[16];

    if (argc != 2) {
        Printf("Usage: textcolor <color>\n");
        Printf("Available colors:");
        for (int i = 0; i < 16; i++) {
            int colorNameLen = StringLenN(colorNames[i], 16);
            for (int j = 0; j < 16 - colorNameLen; j++) {
                spaces[j] = ' ';
            }
            spaces[16 - colorNameLen] = '\0';

            if (i % 4 == 0) {
                Printf("\n");
            }
            PrintfColor(i, "%s%s", colorNames[i], spaces);
        }
        Printf("\n");
        return 1;
    }

    int color = -1;
    for (int i = 0; i < 16; i++) {
        if (StringCmp(argv[1], colorNames[i]) == 0) {
            color = i;
            break;
        }
    }
    if (color == -1) {
        Printf("Invalid color.\n");
        Printf("Available colors:");
        for (int i = 0; i < 16; i++) {
            int colorNameLen = StringLenN(colorNames[i], 16);
            for (int j = 0; j < 16 - colorNameLen; j++) {
                spaces[j] = ' ';
            }
            spaces[16 - colorNameLen] = '\0';

            if (i % 4 == 0) {
                Printf("\n");
            }
            PrintfColor(i, "%s%s", colorNames[i], spaces);
        }
        Printf("\n");
        return 1;
    }
    else {
        SetTextColor(color);
    }

    return 0;
}

static int MonitorUser(int argc, char** argv)
{
    ClearScreen(COLOR_WHITE);
    ResetCursor();
    uint32 pid = CreateProcess((void*)USER_PROC_ADDR, 10000);
    ForceRunProcess(pid);

    return 0;
}

static int RunCommand(char* buf)
{
	char* argv[MAX_ARGS];

	// Parse the command buffer into whitespace-separated arguments
	int argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && StringFindChar(WHITESPACE, *buf)) {
			*buf++ = 0;
        }
		if (*buf == 0) {
			break;
        }

		// save and scan past next arg
		if (argc == MAX_ARGS - 1) {
			Printf("Too many arguments (max %d)\n", MAX_ARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !StringFindChar(WHITESPACE, *buf)) {
			buf++;
        }
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0) {
		return 0;
    }
	for (int i = 0; i < (int)NUM_COMMANDS; i++) {
		if (StringCmp(argv[0], commands[i].name) == 0) {
			return commands[i].func(argc, argv);
        }
	}

	Printf("Unknown command: %s\n", argv[0]);
	return 0;
}

void MonitorRun()
{
    const char spaces[] = "           ";

    int ch;
    monitor.index = 0;
    ClearScreen(COLOR_BLUE);
    ResetCursor();
    PrintfColor(COLOR_BMAGENTA,
        "%s==========================================================%s\n",
        spaces, spaces);
    PrintfColor(COLOR_BMAGENTA,
        "%s|                       Welcome to                       |%s\n",
        spaces, spaces);
    PrintfColor(COLOR_BMAGENTA,
        "%s|                       Storm O.S.                       |%s\n",
        spaces, spaces);
    PrintfColor(COLOR_BMAGENTA,
        "%s|                                                        |%s\n",
        spaces, spaces);
    PrintfColor(COLOR_BMAGENTA,
        "%s|                   Main Command Prompt                  |%s\n",
        spaces, spaces);
    PrintfColor(COLOR_BMAGENTA,
        "%s==========================================================%s\n\n",
        spaces, spaces);
    
    Printf("> ");
    while (1) {
        ch = GetChar();
        if (ch != 0) {
            Printf("%c", (char)ch);

            if (ch == '\n') {
                monitor.buf[monitor.index] = '\0';
                RunCommand(monitor.buf);

                monitor.index = 0;
                Printf("> ");
            }
            else if (ch == '\b') {
                monitor.index--;
                if (monitor.index < 0) {
                    monitor.index = 0;
                }
            }
            else {
                if (monitor.index < CONSOLE_BUFFER_SIZE - 1) {
                    monitor.buf[monitor.index++] = (char)ch;
                }
            }
        }
    }
}