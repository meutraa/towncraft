#include "functions.h"
#include <string.h>

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

static Return fun_quit(int a, ...)
{
	return QUIT_PROGRAM;
}

static Return fun_options(int a, ...)
{
	return SWITCHTO_OPTIONS;
}

const char* function_strings[FUNCTION_COUNT] = 
{
	"quit",
	"options",
};

const function function_pointers[FUNCTION_COUNT] = 
{
	fun_quit,
	fun_options,
};
