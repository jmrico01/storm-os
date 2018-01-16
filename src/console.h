#ifndef CONSOLE_H
#define CONSOLE_H

#define CONSOLE_BUFFER_SIZE 512

void ConsoleInit();

int GetChar();
void PutChar(char c);
void PutStr(const char* str);
void ResetCursor();

#endif