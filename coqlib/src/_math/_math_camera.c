//
//  camera.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#include "_math/_math_camera.h"

Vector3 camera_def_pos = {{0, 0, 4}};

static float _up[3] = {0, 1, 0};

void camera_init(Camera *c, float lambda) {
    fl_array_init(c->pos, camera_def_pos.f_arr, 3, lambda);
    fl_array_init(c->up, _up, 3, lambda);
    fl_array_init(c->center, vector3_zeros.f_arr, 3, lambda);
}

void matrix4_initAsLookAtWithCameraAndYshift(Matrix4 *m, Camera *c, float yShift) {
    Vector3 eye = fl_array_toVec3(c->pos);
    eye.y += yShift;
//    eye.z += 4.f;
    Vector3 center = fl_array_toVec3(c->center);
    center.y += yShift;
    matrix4_initAsLookAt(m, eye, center, fl_array_toVec3(c->up));
}
