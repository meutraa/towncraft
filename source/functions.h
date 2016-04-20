/** @file functions.h 
 *  @brief functions with function pointers.
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define FUNCTION_COUNT 2

typedef int (*function)(int, ...);

extern const char* function_strings[FUNCTION_COUNT];

extern const function* function_pointers[FUNCTION_COUNT];

#endif
