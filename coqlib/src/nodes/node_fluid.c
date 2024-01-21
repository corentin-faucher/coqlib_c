//
//  fluid.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-16.
//

#include "nodes/node_fluid.h"

float Fluid_defaultFadeInDelta = 2.2f;

void fluid_init_(Fluid* f, float lambda) {
    fl_array_init(&f->sx, &f->n.sx, COQ_FLUID_DIMS_N, lambda);
    // Fonction de positionnement pour les node smooth.
    if(f->n.flags & flag_fluidOpenFlags)
        f->n.openOpt = fluid_open_;
    if(f->n.flags & flag_fluidCloseFlags)
        f->n.closeOpt = fluid_close_;
    if(f->n.flags & flag_fluidReshapeFlags)
        f->n.reshapeOpt = fluid_reshape_;
}
Fluid* Fluid_create(Node* const refOpt, float x, float y, float w, float h,
                              float lambda, flag_t flags, uint8_t node_place) {
    Fluid* s = Node_createEmptyOfType_(node_type_n_fluid, sizeof(Fluid),
                                       flags, refOpt, node_place);
    s->n.x = x;
    s->n.y = y;
    s->n.w = w;
    s->n.h = h;
    fluid_init_(s, lambda);
    return s;
}
Fluid* node_asFluidOpt(Node* n) {
    if(n->_type & node_type_flag_fluid)
        return (Fluid*)n;
    return NULL;
}

Vector3 fluid_pos(Fluid* s) {
    return fl_array_toVec3(&s->x);
}

void    fluid_setXY(Fluid* f, Vector2 xy) {
    fl_set(&f->x, xy.x); fl_set(&f->y, xy.y);
    f->n.x = xy.x;       f->n.y = xy.y;
}
void    fluid_setXYScales(Fluid* f, Box box) {
    fl_set(&f->x, box.c_x); fl_set(&f->y, box.c_y);
    f->n.x = box.c_x;       f->n.y = box.c_y;
    fl_set(&f->sx, box.Dx); fl_set(&f->sy, box.Dy);
    f->n.sx = box.Dx;       f->n.sy = box.Dy;
}
void    fluid_setX(Fluid* f, float x, bool fix) {
    if(fix) fl_fix(&f->x, x); else fl_set(&f->x, x);
    f->n.x = x;
}
void    fluid_setXrelToDef(Fluid* f, float x, bool fix) {
    if(fix) fl_fix(&f->x, f->x.def + x);
    else    fl_set(&f->x, f->x.def + x);
    f->n.x = f->x.def + x;
}
void    fluid_setY(Fluid* f, float y, bool fix) {
    if(fix) fl_fix(&f->y, y); else fl_set(&f->y, y);
    f->n.y = y;
}
void    fluid_setYrelToDef(Fluid* f, float y, bool fix) {
    if(fix) fl_fix(&f->y, f->y.def + y);
    else    fl_set(&f->y, f->y.def + y);
    f->n.y = f->y.def + y;
}
void    fluid_setScales(Fluid* f, Vector2 scales, bool fix) {
    if(fix) {
        fl_fix(&f->sx, scales.x);
        fl_fix(&f->sy, scales.y);
    } else {
        fl_set(&f->sx, scales.x);
        fl_set(&f->sy, scales.y);
    }
    f->n.sx = scales.x;  f->n.sy = scales.y;
}

void    fluid_setRelatively_(Fluid* const f, const bool fix) {
    Node* parent = f->n.parent;
    if(parent == NULL) return;
    flag_t flags = f->n.flags;
    float x = 0;
    float y = 0;
    if(flags & flag_fluidRelativeToRight)
        x =  0.5f * parent->w;
    else if(flags & flag_fluidRelativeToLeft)
        x = -0.5f * parent->w;
    if(flags & flag_fluidRelativeToTop)
        y =  0.5f * parent->h;
    else if(flags & flag_fluidRelativeToBottom)
        y = -0.5f * parent->h;
    if(flags & flag_fluidJustifiedRight)
        x -= node_deltaX((Node*)f);
    else if(flags & flag_fluidJustifiedLeft)
        x += node_deltaX((Node*)f);
    if(flags & flag_fluidJustifiedTop)
        y -= node_deltaY((Node*)f);
    else if(flags & flag_fluidJustifiedBottom)
        y += node_deltaY((Node*)f);
    x += f->x.def;
    y += f->y.def;
    if(fix) {
        fl_fix(&f->x, x);
        fl_fix(&f->y, y);
    } else {
        fl_set(&f->x, x);
        fl_set(&f->y, y);
    }
    f->n.x = x;
    f->n.y = y;
}
void    fluid_open_(Node* const node) {
    Fluid* s = node_asFluidOpt(node);
    if(!s) { printerror("Not smooth."); return; }
    if(node->flags & flag_fluidRelativeFlags)
        fluid_setRelatively_(s, true);
    if(!(node->flags & flag_show) && (node->flags & flag_fluidFadeInRight))
        fl_fadeIn(&s->x, Fluid_defaultFadeInDelta);
}
void    fluid_close_(Node* const node) {
    Fluid* s = node_asFluidOpt(node);
    if(!s) { printerror("Not smooth."); return; }
    fl_fadeOut(&s->x, Fluid_defaultFadeInDelta);
}
void    fluid_reshape_(Node* const node) {
    Fluid* s = node_asFluidOpt(node);
    if(!s) { printerror("Not smooth."); return; }
    fluid_setRelatively_(s, false);
}

/// superflu ?
//void    fluid_setDefPos(Fluid* const f, Vector3 defPos) {
//    f->x.def = defPos.x;
//    f->y.def = defPos.y;
//    f->z.def = defPos.z;
//}
//void    fluid_setX(Fluid* const f, float x) {
//    fl_set(&f->x, x);
//    f->n.x = x;
//}
//void    fluid_setY(Fluid* const f, float y) {
//    fl_set(&f->y, y);
//    f->n.y = y;
//}
//void    fluid_setZ(Fluid* const f, float z) {
//    fl_set(&f->z, z);
//    f->n.z = z;
//}
//void    fluid_setScaleX(Fluid* const f, float sx) {
//    fl_set(&f->sx, sx);
//    f->n.sx = sx;
//}
//void    fluid_setScaleY(Fluid* const f, float sy) {
//    fl_set(&f->sy, sy);
//    f->n.sy = sy;
//}
//void    fluid_setXRelToDef(Fluid* const f, float xShift) {
//    float x = f->x.def + xShift;
//    fl_set(&f->x, x);
//    f->n.x = x;
//}
//void    fluid_setYRelToDef(Fluid* const f, float yShift) {
//    float y = f->y.def + yShift;
//    fl_set(&f->y, y);
//    f->n.y = y;
//}
//void    fluid_setZRelToDef(Fluid* const f, float zShift) {
//    float z = f->z.def + zShift;
//    fl_set(&f->z, z);
//    f->n.z = z;
//}
