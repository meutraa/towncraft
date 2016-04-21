#include "functions.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

int get_function_index(char* function_name)
{
	for(int i = 0; i < FUNCTION_COUNT; i++)
	{
		if(0 == strcmp(function_name, function_strings[i]))
		{
			return i;
		}
	}
	return -1;
}

static Status fun_quit(int n, ...)
{
	return 0 != n ? NORMAL : QUIT_PROGRAM;
}

static Status fun_options(int n, ...)
{
	return 0 != n ? NORMAL : SWITCHTO_OPTIONS;
}

static Status fun_main(int n, ...)
{
	return 0 != n ? NORMAL : SWITCHTO_MAINMENU;
}

static Status fun_options_video(int n, ...)
{
	return 0 != n ? NORMAL : SWITCHTO_OPTIONS_VIDEO;
}

static Status fun_options_audio(int n, ...)
{
	return 0 != n ? NORMAL : SWITCHTO_OPTIONS_AUDIO;
}

static Status fun_options_controls(int n, ...)
{
	return 0 != n ? NORMAL : SWITCHTO_OPTIONS_CONTROLS;
}

const char* function_strings[FUNCTION_COUNT] =
{
	"button_quit",
	"button_options",
	"button_main",
	"button_options_video",
	"button_options_audio",
	"button_options_controls",
};

const function function_pointers[FUNCTION_COUNT] =
{
	fun_quit,
	fun_options,
	fun_main,
	fun_options_video,
	fun_options_audio,
	fun_options_controls,
};
