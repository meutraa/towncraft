/*! \file macros.h
    \brief questionable practices.
*/

#ifndef MACROS_H
#define MACROS_H

#define SETEQ(a, b, c, d) if(!strcmp(a, b)) c = d;
#define forXY(a, b) for(int y = a; y < b; y++) for(int x = a; x < b; x++)
#define add_triangle(a, b, c) v[i++] = a.x; v[i++] = a.y; \
                              v[i++] = b.x; v[i++] = b.y; \
                              v[i++] = c.x; v[i++] = c.y;
#define add_triangle_color(a, b, g) c[j++] = a; c[j++] = b; c[j++] = g; \
                           c[j++] = a; c[j++] = b; c[j++] = g; \
                           c[j++] = a; c[j++] = b; c[j++] = g;
#define SCROLL(a, b, c, d, e) if(key_status[a] || (fullscreen && b == c)) \
        { \
            dx += scroll_speed * scale * d; \
            dy += scroll_speed * scale * e; \
            render = 1; \
        }
#define hxy heightmap[x][y]
#define hx1y heightmap[x + 1][y]
#define hxy1 heightmap[x][y + 1]
#define h1xy heightmap[x + 1][y + 1]

#endif
