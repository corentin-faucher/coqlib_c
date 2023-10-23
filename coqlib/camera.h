//
//  camera.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef camera_h
#define camera_h

#include <stdio.h>
#include "maths.h"
#include "smpos.h"

typedef struct {
    SmoothPos pos[3];
    SmoothPos up[3];
    SmoothPos center[3];
} Camera;

extern Vector3 camera_def_pos;

void camera_init(Camera *c, float lambda);

void matrix4_initAsLookAtWithCamera(Matrix4 *m, Camera *c);

#endif /* camera_h */
