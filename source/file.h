/** @file file.h 
 *  @brief file related functions.
 */

#ifndef FILE_H
#define FILE_H

#include "status.h"

/** @fn int count_lines(char* path)
 *  @brief Counts how many '\n' characters are in a file.
 *  @param path a string representing the relative file path of the file.
 *  @return 0 if the file could not be read, else the number of new line characters found
 */
int count_lines(char* path);

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

/** @fn int count_resources(char* layout_file)
 *  @brief Counts how many set of five lines there are in a file.
 *  @param path a string representing the relative file path of the file.
 *  @warning this function does not check the file for readability, and should only be used after is_valid_layout()
 *  @return the number of lines in a file divided by five, or if it is not divisible by five, 0.
 */
int count_resources(char* layout_file);

/** @fn int count_textures(char* layout_file, int drawable_count)
 *  @brief Counts how many unique image textures there are in a file.
 *  @param path a string representing the relative file path of the file.
 *  @param drawable_count providing this saves a reread of the file. And you probably already have this count.
 *  @warning this function does not check the file for readability, and should only be used after is_valid_layout()
 *  @return the number of unique resource paths found.
 */
int count_textures(char* layout_file, int drawable_count);

Return is_valid_layout(char* layout_file);

#endif
