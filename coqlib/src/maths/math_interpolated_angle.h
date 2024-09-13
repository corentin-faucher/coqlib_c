//
//  _math_interpolated_angle.h
//  Position (angulaire) interpolé avec les moindres carrés.
//
//  Created by Corentin Faucher on 2023-11-26.
//
#ifndef COQ_MATH_INTERPOLATED_ANGLE_H
#define COQ_MATH_INTERPOLATED_ANGLE_H

#include "math_chrono.h"
#include "math_base.h"

#define INT_ANGLE_SIZE 20

typedef struct InterpolatedAngle {
    float    pos;
    float    slope;
    uint32_t indexLast;
    uint32_t indexCurrent;
    float    vX[INT_ANGLE_SIZE];
    int64_t  vT[INT_ANGLE_SIZE];
} InterpolatedAngle;

void intangle_init(InterpolatedAngle* ia, float pos);
void intangle_push(InterpolatedAngle* ia, float newPos);

#endif /* interpolated_angle_h */
