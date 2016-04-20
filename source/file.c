#include <stdio.h>
#include <string.h>

#include "constants.h"

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

int count_valid_drawables(char* layout_file)
{
	FILE* file = fopen(layout_file, "r");
	if(NULL == file) return 0;
		
	int len = MAX_LINE_LENGTH + 1;
	
	/* variables to fill for each item. */
	char line[len], name[len], path[len];
	int wx, wy, mx, my;
	int r, g, b, a, font_size;
	
	int status = 0, lines = 0, ret = 1;
	
	while(NULL != fgets(line, len, file))
	{
		if(NULL != strstr(line, "images/")) 
		{
			status = sscanf(line, "%256[^;];%256[^;];%d;%d;%d;%d", name, path, &wx, &wy, &mx, &my);
			if(6 != status) { ret = 0; break; }
		}
		else if(NULL != strstr(line, "fonts/"))
		{
			status = sscanf(line, "%256[^;];%256[^;];%d;%d;%d;%d;%d;%d;%d;%d;%d", 
					name, path, &font_size, &r, &g, &b, &a, &wx, &wy, &mx, &my);
			if(11 != status){ ret = 0; break; }
		}
		else{ ret = 0; break; }
		
		if(-1 == file_exists(path)) { ret = 0; break; }
		lines++;
	}
	/* Close file. */
	if(NULL != file) fclose(file);
	
	/* If no lines have been read. */
	return 0 == status || 0 == ret? 0 : lines;
}
