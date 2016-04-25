/*! \file options.h
	\brief provides access to global option values.
*/

#ifndef OPTIONS_H
#define OPTIONS_H

/*!
	\brief All ui elements will scale their width and height by this value.
*/
extern float ui_scale;

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
	\brief Defines the render scale quality

	"nearest" nearest pixel sampling.
	"linear" linear filtering (supported by OpenGL and Direct3D).
	"best" anisotropic filtering (supported by Direct3D).
*/
extern char render_scale_quality[];

/*!
	\brief Defines the zoom mode.

	0 always zoom on the center of the screen.
	1 zoom and keep the tile on the mouse pointer at the same position as the mouse pointer.
	2 zoom centered on the mouse pointer.
*/
extern int zoom_mode;

/*!
	\brief percent of the screen that will scroll each frame.
*/
extern float scroll_speed;

/*!
	\brief Screen width resolution for use in fullscreen 0 & 1
*/
extern int resolution_width;

/*!
	\brief Screen height resolution for use in fullscreen 0 & 1
*/
extern int resolution_height;

#endif
