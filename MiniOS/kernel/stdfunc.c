#include "stdfunc.h"
#include "stdlib.h"
#include "screen.h"

int printf(const char* format, ...)
{
	char* traverse;
	unsigned int c;
	char* s;

	va_list arg;
	va_start(arg, format);

	for (traverse = format; *traverse != '\0'; traverse++)
	{
		while (*traverse != '%')
		{
			if (*traverse == '\0')
			{
				return;
			}
			PutChar(*traverse);
			traverse++;
		}

		traverse++;

		switch (*traverse)
		{
			case CHAR:		c = va_arg(arg, int);
				PutChar(c);
				break;
			case INTEGER:	c = va_arg(arg, int);
				if (c < 0)
				{
					c = -c;
					PutChar('-');
				}
				PutString(convert(c, 10));
				break;
			case OCTAL:		c = va_arg(arg, unsigned int);
				PutString(convert(c, 8));
				break;
			case HEXA:		c = va_arg(arg, unsigned int);
				PutString("0x");
				PutString(convert(c, 16));
				break;
			case STRING:	s = va_arg(arg, char*);
				PutString(s);
				break;
		}
	}

	va_end(arg);
}

int sprintf(char* dst, const char* format, ...)
{
	char* traverse;
	unsigned int c;
	char* s;
	int i;

	va_list arg;
	va_start(arg, format);
	i = 0;
	for (traverse = format; *traverse != '\0'; traverse++)
	{
		while (*traverse != '%')
		{
			if (*traverse == '\0')
			{
				dst[i] = 0;
				return;
			}
			dst[i++] = *traverse++;
		}

		traverse++;

		switch (*traverse)
		{
		case CHAR:		c = va_arg(arg, int);
			dst[i++] = c;
			break;
		case INTEGER:	c = va_arg(arg, int);
			if (c < 0)
			{
				c = -c;
				dst[i++] = '-';
			}
			strcpy(dst + i, convert(c, 10));
			i += strlen(convert(c, 10));
			break;
		case OCTAL:		c = va_arg(arg, unsigned int);
			strcpy(dst + i, convert(c, 8));
			i += strlen(convert(c, 8));
			break;
		case HEXA:		c = va_arg(arg, unsigned int);
			strcpy(dst + i, "0x");
			i += 2;
			strcpy(dst + i, convert(c, 16));
			i += strlen(convert(c, 16));
			break;
		case STRING:	s = va_arg(arg, char*);
			strcpy(dst + i, s);
			i += strlen(s);
			break;
		}
	}

	va_end(arg);
}

char* convert(unsigned int num, int base)
{
	static char Representation[] = "0123456789ABCDEF";
	static char buffer[50];
	char* ptr;

	ptr = &buffer[49];
	*ptr = '\0';

	do
	{
		*--ptr = Representation[num % base];
		num /= base;
	} while (num != 0);

	return(ptr);
}

void clearString(char *str)
{
	while (*str != 0)
	{
		*str++ = 0;
	}
}

void insertChar(char c, char* str, int pos)
{
	char aux[1000];
	int len = strlen(str);

	strcpy(aux, str + pos);
	str[pos] = c;
	strcpy(str + pos + 1, aux);
}

void removeChar(char* str, int pos)
{
	char aux[1000];

	strcpy(aux, str + pos);
	strcpy(str + pos - 1, aux);
}

int findChar(char* str, char c)
{
	int i;
	i = 0;
	while (str[i] != 0)
	{
		if (str[i] == c)
		{
			return i;
		}
		i++;
	}
	return - 1;
}

char toUpper(char c)
{
	if (c >= 'a' && c <= 'z')
	{
		return c - 'a' + 'A';
	}
	return c;
}

char toLower(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c - 'A' + 'a';
	}
	return c;
}

BOOLEAN isDigit(char c)
{
	if ('0' <= c && '9' >= c) { return TRUE; }
	return FALSE;
}

unsigned long convertToULong(char* str)
{
	int i;
	int length = strlen(str);
	unsigned long number = 0;
	for (i = 0; i < length; i++)
	{
		if (isDigit(str[i]))
		{
			number = number * 10 + (str[i] - '0');
		}
		else
		{
			return 0;
		}
	}
	return number;
}