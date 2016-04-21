#include <stdio.h>
#include <string.h>

int count_lines(char* path)
{
	int ch, lines = 0;
	FILE *file = fopen(path, "r");
	while (EOF != (ch = fgetc(file)))
	{
    	if(ch == '\n') lines++;
	}
	if(NULL != file) fclose(file);
	return lines;
}

int file_exists(char* path)
{
	FILE *file = fopen(path, "r");
	if(NULL == file)
	{
		return -1;
	}
	fclose(file);
	return 0;
}
