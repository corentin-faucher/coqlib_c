//
//  camera.h
//  Structure pratique pour un point de vue.
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef _coq_math_camera_h
#define _coq_math_camera_h

#include "_math_flpos.h"

typedef struct {
    FluidPos pos[3];
    FluidPos up[3];
    FluidPos center[3];
} Camera;

extern Vector3 camera_def_pos;

void camera_init(Camera *c, float lambda);

void matrix4_initAsLookAtWithCameraAndYshift(Matrix4 *m, Camera *c, float yShift);

#endif /* camera_h */
