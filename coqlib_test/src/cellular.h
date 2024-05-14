//
//  cellular.h
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 3/5/24.
//
#ifndef cellular_h
#define cellular_h

#include "utils/util_base.h"
#include "nodes/node_drawable.h"
#include "coq_timer.h"

enum {
    cg_0 = 0x0000,
    cg_r = 0x0001,
    cg_u = 0x0002,
    cg_l = 0x0004,
    cg_d = 0x0008,
    cg_A = 0x0010,
    cg_B = 0x0020,
};

typedef struct CelGrid {
    uint32_t m;
    uint32_t n;
    bool     now01;
    uint8_t  *cels0, *cels1;
} CelGrid;

void     celgrid_init(CelGrid* cg, uint32_t m, uint32_t n);
void     celgrid_deinit(CelGrid* cg);
void     celgrid_update(CelGrid* cg);
void     celgrid_drawToPixels(const CelGrid* const cg, PixelBGRA* const pixels);

typedef struct CelGridNode {
    union {
        Node     n;
        Drawable d;
    };
    CelGrid      cg;
    Timer*       timer;
    PixelBGRA    pixels[1];
} CelGridNode;

CelGridNode* CelGridNode_create(Node* parent, float x, float y, float height, uint32_t m, uint32_t n);


#endif /* cellular_h */
