#ifndef CONSTANTS_H
#define CONSTANTS_H

/* default window settings */
#define WINDOW_TITLE  "simulake"
#define WINDOW_DEFAULT_WIDTH  800
#define WINDOW_DEFAULT_HEIGHT 600

/* fullscreen quad (two triangles) for the vertex shader since raymarching is
 * done entirely in fragent shader */
const float FS_QUAD[] = {
  -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
  -1.0f,  1.0f, 0.0f,
   1.0f,  1.0f, 0.0f
};

/* define opengl function params for fullscreen quad */
#define FS_QUAD_VERTEX_ATTRIB_INDEX 0
#define FS_QUAD_DRAW_ARRAY_PARAMS GL_TRIANGLE_STRIP, 0, 4
#define FS_QUAD_VERTEX_ATTRIB_PARAMS FS_QUAD_VERTEX_ATTRIB_INDEX, \
    3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0

#endif /* ifndef CONSTANTS_H */
