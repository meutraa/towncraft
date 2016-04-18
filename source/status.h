/** @file status.h 
 *  @brief function return codes to be used program-wide.
 */

#ifndef STATUS_H
#define STATUS_H

/**
 *  @brief A bunch of return codes that can be used to give more meaning to returning ints from functions. 
 */
typedef enum Return {
	FATAL_ERROR,	/**< Program should free up resources and quit. */
	ERROR,			/**< Program should at least print out an error occured. */
	NORMAL,			/**< Everything went hunky dory. */
	QUIT_PROGRAM,	/**< Everything is fine. But end the program already. */
} Return;

#endif
