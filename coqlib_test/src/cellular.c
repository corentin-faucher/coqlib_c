//
//  cellular.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 3/5/24.
//

#include "cellular.h"

uint8_t   cel_eval(uint8_t c, uint8_t prev, uint8_t next, uint8_t down, uint8_t up) {
    uint8_t v = (prev & cg_r) | (next & cg_l) | (down & cg_u) | (up & cg_d);
    if(v == 0) {
//        if((rand() % 10) == 0) return 1 << (rand() % 4);
        return v;
    }
    uint8_t n = (v & 0x05) + ((v & 0x0a) >> 1);
    n = (n & 0x03) + ((n & 0x0c) >> 2);
    if(n == 1) return v;
    uint8_t new = 1 << (rand() % 4);
    if(new & v) return new;
    // Sinon on essaie encore... (cas rare de toute façon ?)
    new <<= new;
    if(!new) new = cg_d;
    if(new & v) return new;
    return cg_r;
}
PixelBGRA cel_toPixel(uint8_t c) {
    if(c == cg_r) return (PixelBGRA){0xF0FF2020};
    if(c == cg_l) return (PixelBGRA){0xF020FF20};
    if(c == cg_u) return (PixelBGRA){0xF0FF8020};
    if(c == cg_d) return (PixelBGRA){0xF02020FF};
    if(c == cg_A) return (PixelBGRA){0xFFFFFF00};
    if(c == cg_B) return (PixelBGRA){0xFF00FFFF};
    return (PixelBGRA){0x80808080};
}

void     celgrid_init(CelGrid* cg, uint32_t m, uint32_t n) {
    uint32_t const mn = m*n;
    if(mn == 0) { printerror("Empty grid."); return; }
    if(cg->cels0 || cg->cels1) { printerror("cels already alloc ?"); return; }
    cg->cels0 = coq_calloc(mn, sizeof(uint8_t));
    cg->cels1 = coq_calloc(mn, sizeof(uint8_t));
    cg->m = m;
    cg->n = n;
    // Boucle sur les cellules pour init.
    uint8_t            (*line)[m] = (uint8_t(*)[m])&cg->cels0[0];
    uint8_t (* const line_end)[m] = (uint8_t(*)[m])&cg->cels0[mn];
    bool odd_line = false;
    uint32_t i = 0, j = 0;
    while(line < line_end) {
        uint8_t *cel = (uint8_t*)line;
        line ++;
        uint8_t const *cel_end = (uint8_t*)line;
        bool odd_cel = false;
        i = 0;
        while(cel < cel_end) {
            if((i < 7*m / 16 || i > 9*m / 16) || (j < 7*n / 16 || j > 9*n / 16))
                *cel = odd_line ? ( odd_cel ? cg_r : cg_u ) : ( odd_cel ? cg_d : cg_l );
            else
                *cel = 0; // 1 << (rand() % 4);
            cel ++;
            
            odd_cel = !odd_cel;

            i ++;
        }
        j ++;
        odd_line = !odd_line;
    }
}
void     celgrid_deinit(CelGrid* cg) {
    coq_free(cg->cels0);
    coq_free(cg->cels1);
}
void     celgrid_update(CelGrid* cg) {
    uint32_t const m = cg->m, n = cg->n;
    uint8_t const (* const line_beg)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels1[0]   : &cg->cels0[0]);
    uint8_t const (* const line_end)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels1[m*n] : &cg->cels0[m*n]);
    uint8_t const            (*line)[m] = line_beg;
    uint8_t const        (*lineNext)[m] = line_beg + 1;
    uint8_t const        (*linePrev)[m] = line_beg + (n - 1); // On considère que ça loop.
    uint8_t               (*lineOut)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels0[0]   : &cg->cels1[0]);
    while(line < line_end) {
        uint8_t const *cel = (uint8_t*)line;
        uint8_t *celOut = (uint8_t*)lineOut;
        uint8_t* const cel_end = (uint8_t*)(line + 1);
        uint8_t const *celNext = cel + 1;
        uint8_t const *celPrev = cel + (m - 1); // (loop)
        uint8_t const *celUp = (uint8_t*)lineNext;
        uint8_t const *celDown = (uint8_t*)linePrev;
        while(cel < cel_end) {
            *celOut = cel_eval(*cel, *celPrev, *celNext, *celDown, *celUp);
            celPrev = cel;
            cel ++; celOut ++; celDown++; celUp++;
            celNext = cel + 1;
            if(celNext >= cel_end) celNext = (uint8_t*)line;
        }
        linePrev = line;
        line ++; lineOut ++;
        lineNext = line + 1;
        if(lineNext >= line_end) lineNext = line_beg; // (loop)
    }
    // Swap !
    cg->now01 = !cg->now01;
}

void     celgrid_drawToPixels(const CelGrid* const cg, PixelBGRA* const pixels) {
    uint32_t const mn = cg->m * cg->n;
    uint8_t const *       cel     = cg->now01 ?  cg->cels1     :  cg->cels0;
    uint8_t const * const cel_end = cg->now01 ? &cg->cels1[mn] : &cg->cels0[mn]; 
    PixelBGRA* pixel =            pixels;
    while(cel < cel_end) {
        *pixel = cel_toPixel(*cel);
        cel++; pixel++;
    }
}


void celgridnode_callback_(Node* n) {
    CelGridNode* cgn = (CelGridNode*)n;
    celgrid_update(&cgn->cg);
    celgrid_drawToPixels(&cgn->cg, cgn->pixels);
    texture_engine_updatePixels(cgn->d._tex, cgn->pixels);
}
void celgridnode_open_(Node* n) {
    CelGridNode* cgn = (CelGridNode*)n;
    timer_scheduled(&cgn->timer, 1, true, &cgn->n, celgridnode_callback_);
}
void celgridnode_close_(Node* n) {
    CelGridNode* cgn = (CelGridNode*)n;
    timer_cancel(&cgn->timer);
}
CelGridNode* CelGridNode_create(Node* parent, float x, float y, float height, uint32_t m, uint32_t n) {
    uint32_t count = m*n;
    if(count == 0) { printerror("No cells."); return NULL; }
    CelGridNode* cgn = coq_calloc(1, sizeof(CelGridNode) + sizeof(PixelBGRA) * (count - 1));
    node_init_(&cgn->n, parent, x, y, height, height, node_type_n_drawable, 0, 0);
    cgn->n.openOpt =  celgridnode_open_;
    cgn->n.closeOpt = celgridnode_close_;
    celgrid_init(&cgn->cg, m, n);
    celgrid_drawToPixels(&cgn->cg, cgn->pixels);
    drawable_init_(&cgn->d, Texture_createWithPixels(cgn->pixels, m, n, false, false), mesh_sprite, 0, height, 0);
    
    return cgn;
}
