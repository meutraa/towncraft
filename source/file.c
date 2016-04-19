#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "status.h"
#include "math.h"
#include "constants.h"

static int file_exists(char* path);
static int valid_number(char* a);

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
				c = strtok(NULL, ":\n");  // Get next field
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
			int len = strlen(lines[i]);
			
			if(NULL == lines[0])
			{
				break;
			}
			if(0 != len)
			{
				lines[i][len-- - 1] = '\0';
			}
			if(i > 0 && NULL == lines[i])
			{
				fprintf(stderr, "Unexpected end of file. Last line read: %s\n", lines[i - 1]);
				return FILE_UNEXPECTED_EOF;
			}
			if(MAX_LINE_LENGTH == len + 1)
			{
				fprintf(stderr, "Contents of line *may* have surpassed MAX_LINE_LENGTH(%d): %s\n", MAX_LINE_LENGTH, lines[i]);
				return FILE_BUFFER_OVERFLOW;
			}
			if(i != 4 && 0 == len)
			{
				fprintf(stderr, "Line %d is empty and should not be.\n", total_lines);
				return ERROR;
			}
			total_lines++;
		}
		
		/* LINE TWO */
		if(NULL != strstr(lines[1], "images/"))
		{
			/* Check the file exists. */
			int exists = file_exists(lines[1]);
			if(0 != exists)
			{
				fprintf(stderr, "File \"%s\" does not exist or is not readable.\n", lines[1]);
				return FILE_READ_ERROR;
			}
		}
		else if(NULL != strstr(lines[1], "fonts/"))
		{
			/* Font file check and value sanity check. */
			char *c;
			c = strtok(lines[1], " ");
			if(NULL == c)
			{
				fprintf(stderr, "Could not find a ' ' after the font path: %s\n", lines[1]);
				return ERROR;
			}
			else if(0 == strlen(c))
			{
				fprintf(stderr, "Font path empty on line: %s\n", lines[1]);
				return ERROR;
			}
			int exists = file_exists(c);
			if(0 != exists)
			{
				fprintf(stderr, "Font file \"%s\" does not exist or is not readable.\n", c);
				return FILE_READ_ERROR;
			}
			
			c = strtok(NULL, " ");
			/* Test for font parameters. */
			for(int i = 0; i < 5; i ++)
			{
				if(0 != valid_number(c))
				{
					fprintf(stderr, "%d: Could not parse font's parameters.\n", total_lines - 3);
					return ERROR;
				}
				c = strtok(NULL, " \n");
			}
		}
		else
		{
			fprintf(stderr, "Resource format not recognised: %s\n", lines[1]);
			return ERROR;
		}
		
		/* LINE THREE & FOUR */
		
		for(int i = 2; i < 4; i ++)
		{
			char *c = strtok(lines[i], " ");
			char *d = strtok(NULL, "\n");
			if(NULL != strchr(d, ' '))
			{
				fprintf(stderr, "Numbers do not contain spaces! %s", d);
				return ERROR;
			}
			char* e[2] = {c, d};
			for (int j = 0; j < 2; j++)
			{
				if(0 != valid_number(e[j]))
				{
					fprintf(stderr, "%d: Could not parse drawables position.\n", total_lines - 2 + j);
					return ERROR;	
				}
			}
		}
		
		/* LINE FIVE */
		if(0 != strlen(lines[4]))
		{
			fprintf(stderr, "Line %d should be empty and may not contain whitespace.\n", total_lines);
			return ERROR;
		}
	}
	return NORMAL;
}

static int valid_number(char* a)
{
	if(NULL == a || 0 == strlen(a))
	{
		return -1;
	}
	if(1 == strlen(a) && a[0] == '0')
	{
		return 0;
	}
	else
	{
		long test = strtol(a, NULL, 10);
		if(0L == test)
		{
			return -1;
		}
	}
	return 0;
}

static int file_exists(char* path)
{
	FILE *file = fopen(path, "r");
	if(NULL == file)
	{
		return -1;
	}
	fclose(file);
	return 0;
}
