//
//  camera.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#include "math_camera.h"

Vector3 camera_def_pos = {{0, 0, 10}};

static float _up[3] = {0, 1, 0};

void camera_init(Camera *c, float lambda) {
    fl_array_init(c->pos, camera_def_pos.f_arr, 3, lambda);
    fl_array_init(c->up, _up, 3, lambda);
    fl_array_init(c->center, vector3_zeros.f_arr, 3, lambda);
}
