#ifndef CONSOLE_H
#define CONSOLE_H

#define CONSOLE_BUFFER_SIZE 512

void ConsoleInit();

int GetChar();
void PutChar(char c, enum Color color);
void PutStr(const char* str, enum Color color);
void ResetCursor();

#endif