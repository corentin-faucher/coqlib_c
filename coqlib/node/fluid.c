//
//  fluid.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-16.
//

#include "fluid.h"

float Fluid_defaultFadeInDelta = 2.2f;

void _fluid_init(Fluid* f, float lambda) {
    sp_array_init(&f->sx, &f->n.sx, _NODESMOOTH_DIMS_N, lambda);
    // Fonction de positionnement pour les node smooth.
    if(f->n.flags & flag_fluidOpenFlags)
        f->n.open = fluid_open;
    if(f->n.flags & flag_fluidCloseFlags)
        f->n.close = fluid_close;
    if(f->n.flags & flag_fluidReshapeFlags)
        f->n.reshape = fluid_reshape;
}
Fluid* Fluid_create(Node* const refOpt, float x, float y, float w, float h,
                              float lambda, flag_t flags, uint8_t node_place) {
    Fluid* s = _Node_createEmptyOfType(node_type_smooth, sizeof(Fluid),
                                       flags, refOpt, node_place);
    s->n.x = x;
    s->n.y = y;
    s->n.w = w;
    s->n.h = h;
    _fluid_init(s, lambda);
    return s;
}
Fluid* node_asFluidOpt(Node* n) {
    if(n->_type & node_type_flag_smooth)
        return (Fluid*)n;
    return NULL;
}

void    fluid_setDefPos(Fluid* const f, Vector3 defPos) {
    f->x.def = defPos.x;
    f->y.def = defPos.y;
    f->z.def = defPos.z;
}
void    fluid_setX(Fluid* const f, float x, Bool fix) {
    sp_set(&f->x, x, fix);
    f->n.x = x;
}
void    fluid_setY(Fluid* const f, float y, Bool fix) {
    sp_set(&f->y, y, fix);
    f->n.y = y;
}
void    fluid_setZ(Fluid* const f, float z, Bool fix) {
    sp_set(&f->z, z, fix);
    f->n.z = z;
}
void    fluid_setScaleX(Fluid* const f, float sx, Bool fix) {
    sp_set(&f->sx, sx, fix);
    f->n.sx = sx;
}
void    fluid_setScaleY(Fluid* const f, float sy, Bool fix) {
    sp_set(&f->sy, sy, fix);
    f->n.sy = sy;
}
void    fluid_setXRelToDef(Fluid* const f, float xShift, Bool fix) {
    float x = f->x.def + xShift;
    sp_set(&f->x, x, fix);
    f->n.x = x;
}
void    fluid_setYRelToDef(Fluid* const f, float yShift, Bool fix) {
    float y = f->y.def + yShift;
    sp_set(&f->y, y, fix);
    f->n.y = y;
}
void    fluid_setZRelToDef(Fluid* const f, float zShift, Bool fix) {
    float z = f->z.def + zShift;
    sp_set(&f->z, z, fix);
    f->n.z = z;
}
Vector3 fluid_pos(Fluid* s) {
    return sp_array_toVec3(&s->x);
}

void    _fluid_setRelatively(Fluid* const f, const Bool fix) {
    Node* parent = f->n.parent;
    if(parent == NULL) return;
    flag_t flags = f->n.flags;
    float xDec = 0;
    float yDec = 0;
    if(flags & flag_fluidRelativeToRight)
        xDec =  0.5f * parent->w;
    else if(flags & flag_fluidRelativeToLeft)
        xDec = -0.5f * parent->w;
    if(flags & flag_fluidRelativeToTop)
        yDec =  0.5f * parent->h;
    else if(flags & flag_fluidRelativeToBottom)
        yDec = -0.5f * parent->h;
    if(flags & flag_fluidJustifiedRight)
        xDec -= node_deltaX((Node*)f);
    else if(flags & flag_fluidJustifiedLeft)
        xDec += node_deltaX((Node*)f);
    if(flags & flag_fluidJustifiedTop)
        yDec -= node_deltaY((Node*)f);
    else if(flags & flag_fluidJustifiedBottom)
        yDec += node_deltaY((Node*)f);
    fluid_setXRelToDef(f, xDec, fix);
    fluid_setYRelToDef(f, yDec, fix);
}
void    fluid_open(Node* const node) {
    Fluid* s = node_asFluidOpt(node);
    if(!s) { printerror("Not smooth."); return; }
    if(node->flags & flag_fluidRelativeFlags)
        _fluid_setRelatively(s, true);
    if(!(node->flags & flag_show) && (node->flags & flag_fluidFadeInRight))
        sp_fadeIn(&s->x, Fluid_defaultFadeInDelta);
}
void    fluid_close(Node* const node) {
    Fluid* s = node_asFluidOpt(node);
    if(!s) { printerror("Not smooth."); return; }
    sp_fadeOut(&s->x, Fluid_defaultFadeInDelta);
}
void    fluid_reshape(Node* const node) {
    Fluid* s = node_asFluidOpt(node);
    if(!s) { printerror("Not smooth."); return; }
    _fluid_setRelatively(s, false);
}
