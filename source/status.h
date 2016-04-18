/** @file status.h 
 *  @brief function return codes to be used program-wide.
 */

#ifndef STATUS_H
#define STATUS_H

typedef enum Return {
	FATAL_ERROR,
	ERROR,
	NORMAL,
	QUIT_PROGRAM,
} Return;

#endif
