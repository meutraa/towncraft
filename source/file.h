/** @file file.h 
 *  @brief file related functions.
 */

#ifndef FILE_H
#define FILE_H

#include "status.h"

/** @fn int count_valid_settings(char* path)
 *  @brief Counts how many valid settings it finds in a file.
 *
 *  Settings are stored in the following format:\n
 *      setting_id_string:localised_string:value\n
 *  @warning Each of the three strings must not be empty else they will be ignored.
 *
 *  @param path a string representing the relative file path of the settings file.
 *  @return -1 if the file does not exist or is not readable, else the number of
 *          valid settings parsed.
 */
int count_valid_settings(char* path);

/** @fn int count_valid_drawables(char* layout_file)
 *  @brief Tests the validility of a layout file and 
 *  @param layout_file a string representing the relative file path of the file.
 *  @return 0 if the file is invalid, the number of lines other wise. 
 */
int count_valid_drawables(char* layout_file);

#endif
