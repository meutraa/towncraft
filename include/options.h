/*! \file options.h
    \brief provides access to global option values.
*/

#ifndef OPTIONS_H
#define OPTIONS_H

/*!
    \brief Whether or not the renderer should use vsync.
*/
extern int vsync;

/*!
    \brief Defines the fullscreen setting.

    0 use windowed custom resolution.
    1 use fullscreen at fallback/window resolution.
    2 use fullscreen at current desktop resolution.
*/
extern int fullscreen;

/*!
    \brief percent of the screen that will scroll each frame.
*/
int scroll_speed;

/*!
    \brief Screen width resolution for use in fullscreen 0 & 1
*/
extern int resolution_width;

/*!
    \brief Screen height resolution for use in fullscreen 0 & 1
*/
extern int resolution_height;

#endif
