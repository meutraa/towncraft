/*! \file file.h
	\brief file related functions.
*/

#ifndef FILE_H
#define FILE_H

/*! \fn int count_lines(char* path)
	\brief Counts how many '\n' characters are in a file.
	\param path a string representing the relative file path of the file.
	\return 0 if the file could not be read, else the number of new line characters found
*/
int count_lines(char* path);

/*! \fn int file_exists(char* path)
	\brief Check to see if a file is readable.
	\param path a string representing the relative file path of the file.
	\return -1 if the file could not be read, else 0;
*/
int file_exists(char* path);

#endif
