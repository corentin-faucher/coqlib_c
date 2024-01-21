//
//  interpolated_angle.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-11-26.
//

#include "maths/math_interpolated_angle.h"

void intangle_init(InterpolatedAngle* ia, float pos) {
    memset(ia->vT, 0, sizeof(ia->vT));
    for(int i = 0; i < INT_ANGLE_SIZE; i++) {
        ia->vX[i] = pos;
    }
}

void intangle_push(InterpolatedAngle* ia, float newPos) {
    int64_t time = _CR_elapsedMS;
    if(time == ia->vT[ia->indexLast]) return;
    newPos = float_toNormalizedAngle(newPos);
    ia->vX[ia->indexCurrent] = newPos;
    ia->vT[ia->indexCurrent] = time; // (en ms)
    // Pos et temps relatifs.
    float   vXr[INT_ANGLE_SIZE];
    float   vTr[INT_ANGLE_SIZE];  // (en sec)
    for(int i = 0; i < INT_ANGLE_SIZE; i++) {
        vXr[i] = float_toNormalizedAngle(ia->vX[i] - newPos);
        vTr[i] = ((float)(ia->vT[i] - time)) / 1000.f;
    }
    float sumPrTX = 0.f;
    float sumT = 0.f;
    float sumX = 0.f;
    float sumT2 = 0.f;
    for(int i = 0; i < INT_ANGLE_SIZE; i++) {
        sumPrTX += vXr[i] * vTr[i];
        sumT += vTr[i];
        sumX += vXr[i];
        sumT2 += vTr[i]*vTr[i];
    }
    float det = INT_ANGLE_SIZE * sumT2 - sumT*sumT;
    if(det == 0.f || sumT == 0.f) {
        printerror("Error interpolation."); return;
    }
    // Interpolation
    ia->slope = (INT_ANGLE_SIZE * sumPrTX - sumT * sumX) / det;
    ia->pos =   (sumPrTX - ia->slope * sumT2) / sumT + newPos;
    
    ia->indexLast = ia->indexCurrent;
    ia->indexCurrent = (ia->indexCurrent + 1) % INT_ANGLE_SIZE;
}
