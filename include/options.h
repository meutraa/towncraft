/*! \file options.h
	\brief provides access to global option values.
*/

#ifndef OPTIONS_H
#define OPTIONS_H

/*! \var extern float ui_scale
	\brief All ui elements will scale their width and height by this value.
*/
extern float ui_scale;

/*! \var extern int vsync
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

/*! /var extern int zoom_mode
	\brief Defines the zoom mode.

	0 always zoom on the center of the screen.
	1 zoom and keep the tile on the mouse pointer at the same position as the mouse pointer.
	2 zoom centered on the mouse pointer.
*/
extern int zoom_mode;

/*! \var extern float scroll_speed
	\brief percent of the screen that will scroll each frame.
*/
extern float scroll_speed;

/*! \var extern int resolution_width
	\brief Screen width resolution for use in fullscreen 0 & 1
*/
extern int resolution_width;

/*! \var extern int resolution_height
	\brief Screen height resolution for use in fullscreen 0 & 1
*/
extern int resolution_height;

#endif
