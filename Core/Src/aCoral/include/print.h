#ifndef ACORAL_PRINT_H
#define ACORAL_PRINT_H
#include<print.h>
#include<string.h>
#include <stdio.h>
#include <stdarg.h>
int acoral_print(const char *fmt, ...);
char acoral_putchar(char c);

void vprint(const char *fmt, va_list argp);
void my_printf(const char *fmt, ...); // custom printf() function


#endif
