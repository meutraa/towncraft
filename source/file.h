#ifndef FILE_H
#define FILE_H

/** @file file.h 
 *  @brief file related functions.
 */
 
/** @def MAX_SETTING_LENGTH
 *  @brief The max length to be read of a line in the settings file.
 */
 #define MAX_SETTING_LENGTH 128

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

#endif
