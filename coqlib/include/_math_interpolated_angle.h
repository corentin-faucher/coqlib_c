//
//  _math_interpolated_angle.h
//  Position (angulaire) interpolé avec les moindres carrés.
//
//  Created by Corentin Faucher on 2023-11-26.
//

#ifndef _coq_math_interpolated_angle_h
#define _coq_math_interpolated_angle_h

#include "_math_chrono.h"
#include "_math_.h"

#define INT_ANGLE_SIZE 20

typedef struct _IntAngle {
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
