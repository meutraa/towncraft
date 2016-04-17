#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"

int count_valid_settings(char* path)
{
	FILE* file = fopen(path, "r");
	if(NULL == file) return 0;
	
	char buf[MAX_SETTING_LENGTH + 1];   // +1 for the terminating null-character.
	char* c;
	int total = 0;
	 
	while(NULL != fgets(buf, MAX_SETTING_LENGTH + 1, file))
	{
		int fields = 0;
		c = strtok(buf, ":");   // Get the first field
		while(NULL != c)
		{
			if(0 != strlen(c))
			{
				fields++;
				c = strtok(NULL, ":");  // Get next field
			}
			else
			{
				fprintf(stdout, "Empty field in settings file.");
			}
		}
		if(3 == fields)
		{
			total++;
		}
	}
	if(NULL != file) fclose(file);
	return total;
}
