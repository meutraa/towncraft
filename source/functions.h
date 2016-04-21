/*! \file functions.h
	\brief functions with function pointers.
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "status.h"

/*! \def FUNCTION_COUNT
	\brief The number of function pointers in the function_pointers array.
*/
#define FUNCTION_COUNT 4

/*!
	\brief A typedef that makes using a function pointer legible.
	\note This function pointer is of the strict type Return function(int a, ...).
*/
typedef Return (*function)(int, ...);

/*! \var extern const char* function_strings[FUNCTION_COUNT]
	\brief An array of strings that map to the function pointers in function_pointers array.
*/
extern const char* function_strings[FUNCTION_COUNT];

/*! \var extern const function function_pointers[FUNCTION_COUNT]
	\brief An array of function pointers to be used by drawables on click.
*/
extern const function function_pointers[FUNCTION_COUNT];

/*! \fn int get_function_index(char* function_name)
	\brief gets the index of the function pointer that is mapped to the function string.
	\param function_name the unique function name from the array function_strings[].
	\return -1 if function does not exist, the index otherwise.
*/
int get_function_index(char* function_name);

#endif
