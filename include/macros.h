/*! \file macros.h
    \brief questionable practices.
*/

#ifndef MACROS_H
#define MACROS_H

#define SETEQ(a, b, c, d) if(!strcmp(a, b)) c = d;
#define forXY(a, b) for(int y = a; y < b; y++) for(int x = a; x < b; x++)

#endif
