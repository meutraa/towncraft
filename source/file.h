/** @file file.h 
 *  @brief file related functions.
 */

#ifndef FILE_H
#define FILE_H

#include "status.h"

/** @fn int count_valid_drawables(char* layout_file)
 *  @brief Tests the validility of a layout file and 
 *  @param layout_file a string representing the relative file path of the file.
 *  @return 0 if the file is invalid, the number of lines other wise. 
 */
int count_valid_drawables(char* layout_file);

#endif
