#include "functions.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

int get_function_index(char* function_name)
{
	for(int i = 0; i < FUNCTION_COUNT; i++)
	{
		if(0 == strncmp(function_name, function_strings[i], strlen(function_strings[i])))
		{
			return i;
		}
	}
	return -1;
}

static Return fun_quit(int n, ...)
{
	return (0 != n ? NORMAL : QUIT_PROGRAM);
}

static Return fun_options(int n, ...)
{
	return (0 != n ? NORMAL : SWITCHTO_OPTIONS);
}

static Return fun_print(int n, ...)
{
	va_list args;
	va_start(args, n);
	printf("%s\n", va_arg(args, char*));
	return NORMAL;
}


const char* function_strings[FUNCTION_COUNT] = 
{
	"button_quit",
	"button_options",
	"print",
};

const function function_pointers[FUNCTION_COUNT] = 
{

	fun_quit,
	fun_options,
	fun_print,
};
