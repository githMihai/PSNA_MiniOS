#ifndef _STDFUNC_H_
#define _STDFUNC_H_

#define INTEGER 'd'
#define CHAR	'c'
#define OCTAL	'o'
#define STRING	's'
#define HEXA	'x'
#define LONG	'l'

#include "stdarg.h"
#include "main.h"

int printf(const char* format, ...);
int sprintf(char* dst, const char* format, ...);
char* convert(unsigned int num, int base);
void clearString(char* str);
void insertChar(char c, char* str, int pos);
void removeChar(char* str, int pos);
int findChar(char* str, char c);
char toUpper(char c);
char toLower(char c);
BOOLEAN isDigtc(char c);
unsigned long convertToULong(char* str);

#endif // _STDFUNC_H_