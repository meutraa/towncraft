/** @file functions.h 
 *  @brief functions with function pointers.
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "status.h"

#define FUNCTION_COUNT 3

typedef Return (*function)(int, ...);

extern const char* function_strings[FUNCTION_COUNT];

extern const function function_pointers[FUNCTION_COUNT];

/** @fn int get_function_index(char* function_name)
 *  @brief gets the index of the function pointer that is mapped to the function string.
 *  @param function_name the unique function name from the array function_strings[].
 *  @return -1 if function does not exist, the index otherwise.
 */
int get_function_index(char* function_name);

#endif
