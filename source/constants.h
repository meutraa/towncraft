/** @file constants.h 
 *  @brief program-wide constants.
 */

#ifndef OUR_CONSTANTS_H
#define OUR_CONSTANTS_H

/** @def MAX_LINE_LENGTH
 *  @brief the maximum number of chars that will be read from a line on a layout file.
 */
#define MAX_LINE_LENGTH 256

/** @def DESIGN_WIDTH
 *  @brief The horizontal pixel position that all functions will treat as 100% of any other resolution.
 *  This means that if the user has a horizontal resolution of 1920, any SDL_Rects set to render at 1280, will 
 *  really render at 1920.
 */
#define DESIGN_WIDTH 1280

/** @def DESIGN_HEIGHT
 *  @brief The vertical pixel position that all functions will treat as 100% of any other resolution.
 *  This means that if the user has a vertical resolution of 2160, any SDL_Rects set to render at 720, will 
 *  really render at 2160.
 */
#define DESIGN_HEIGHT 720

#endif
