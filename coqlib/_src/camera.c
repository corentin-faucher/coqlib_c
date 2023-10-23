//
//  camera.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#include "camera.h"

Vector3 camera_def_pos = {0, 0, 4};

static float _up[3] = {0, 1, 0};

void camera_init(Camera *c, float lambda) {
    sp_array_init(c->pos, (float*)&camera_def_pos, 3, lambda);
    sp_array_init(c->up, _up, 3, lambda);
    sp_array_init(c->center, (float*)&vector3_zeros, 3, lambda);
}

void matrix4_initAsLookAtWithCamera(Matrix4 *m, Camera *c) {
    matrix4_initAsLookAt(m, sp_array_toVec3(c->pos),
                         sp_array_toVec3(c->center), sp_array_toVec3(c->up));
}
