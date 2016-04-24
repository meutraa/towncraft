/*! \file file.h
	\brief file related functions.
*/

#ifndef FILE_H
#define FILE_H

/*! \fn int file_exists(char* path)
	\brief Check to see if a file is readable.
	\param path a string representing the relative file path of the file.
	\return -1 if the file could not be read, else 0;
*/
int file_exists(char* path);

#endif
