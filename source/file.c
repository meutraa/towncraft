#include <stdio.h>
#include <string.h>

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
