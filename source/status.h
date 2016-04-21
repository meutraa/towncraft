/*! \file status.h
	\brief function return codes to be used program-wide.
*/

#ifndef STATUS_H
#define STATUS_H

/*!
	\brief A bunch of return codes that can be used to give more meaning to returning ints from functions.
*/
typedef enum Status {
	FATAL_ERROR,	/*!< Program should free up resources and quit. */
	ERROR,			/*!< Program should at least print out an error occured. */
	NORMAL,			/*!< Everything went hunky dory. */
	QUIT_PROGRAM,	/*!< Everything is fine. But end the program already. */
	FILE_READ_ERROR, 		/*!< When a file could not be opened. */
	FILE_BUFFER_OVERFLOW,	/*!< A line in the file may not have been fully read and you should fix this. */
	FILE_UNEXPECTED_EOF,	/*!< The end of the file was encountered in the wrong place. */
	SWITCHTO_OPTIONS,		/*!< Program should now load the options menu. */
	SHOW_OPTIONS_VIDEO,
	SHOW_OPTIONS_AUDIO,
	SHOW_OPTIONS_CONTROLS,
	NONE,
	SWITCHTO_MAINMENU,		/*!< Program should now load the main menu. */
} Status;

#endif
