#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "status.h"
#include "math.h"
#include "constants.h"

int count_valid_settings(char* path)
{
	FILE* file = fopen(path, "r");
	if(NULL == file) return 0;
	
	char buf[MAX_LINE_LENGTH + 1];   // +1 for the terminating null-character.
	char* c;
	int total = 0;
	 
	while(NULL != fgets(buf, MAX_LINE_LENGTH + 1, file))
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

Return is_valid_layout(char* layout_file)
{
	FILE* file = fopen(layout_file, "r");
	if(NULL == file) return FILE_READ_ERROR;
		
	int len = MAX_LINE_LENGTH + 1;
	char lines[5][len]; /* One for each line of a layout drawable. */
	int total_lines = 0;
	
	while(0 == feof(file))
	{
		for(int i = 0; i < (int) LENGTH(lines); i++)
		{
			fgets(lines[i], len, file);
			if(NULL == lines[0])
			{
				break;
			}
			else if(i > 0 && NULL == lines[i])
			{
				fprintf(stderr, "Unexpected end of file. Last line read: %s\n", lines[i - 1]);
				return FILE_UNEXPECTED_EOF;
			}
			else if(128 == strlen(lines[i]))
			{
				fprintf(stderr, "Contents of line *may* have surpassed MAX_LINE_LENGTH(%d): %s\n", MAX_LINE_LENGTH, lines[i]);
				return FILE_BUFFER_OVERFLOW;
			}
			else if(i != 4 && 0 == strlen(lines[i]))
			{
				fprintf(stderr, "Line %d is empty and should not be.\n", total_lines);
				return ERROR;
			}
			else if(i == 4 && 0 != strcmp(lines[4], "\n"))
			{
				fprintf(stderr, "Line %d should be empty and may not contain whitespace.\n", total_lines);
				return ERROR;
			}
			total_lines++;
		}
		/* All lines should not be NULL, within 128 chars and filled now. */
		if(0 == strlen(lines[0]))
		{
			
		}
	}
	return NORMAL;
}
