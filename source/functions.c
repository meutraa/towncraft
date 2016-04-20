#include "functions.h"

static int fun_quit(int a, ...)
{
	return -1;
}

static int fun_options(int a, ...)
{
	return 1;
}

static const function pointer_quit = fun_quit;
static const function pointer_options = fun_options;

const char* function_strings[FUNCTION_COUNT] = 
{
	"quit",
	"options",
};

const function* function_pointers[FUNCTION_COUNT] = 
{
	&pointer_quit,
	&pointer_options,
};
