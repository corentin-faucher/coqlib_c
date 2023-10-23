//
//  node_smooth.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-16.
//

#include "node_smooth.h"

float NodeSmooth_defaultFadeInDelta = 2.2f;

void _nodesmooth_init(NodeSmooth* ns, float lambda) {
    sp_array_init(&ns->sx, &ns->nd.sx, 4, lambda);
    // Fonction de positionnement pour les node smooth.
    if(ns->nd.flags & (flag_smoothRelativeFlags|flag_smoothFadeInRight))
        ns->nd.open = node_smooth_open_default;
    if(ns->nd.flags & flag_smoothFadeInRight)
        ns->nd.close = node_smooth_close_fadeOut;
    if(ns->nd.flags & flag_smoothRelativeFlags)
        ns->nd.reshape = node_smooth_reshape_relatively;
}

NodeSmooth* NodeSmooth_create(Node* const refOpt, float x, float y, float w, float h,
                              float lambda, flag_t flags, uint8_t node_place) {
    Node *nd = _Node_createEmptyOfType(node_type_smooth, sizeof(NodeSmooth),
                                       flags, refOpt, node_place);
    nd->x = x;
    nd->y = y;
    nd->w = w;
    nd->h = h;
    NodeSmooth *ns = (NodeSmooth*)nd;
    _nodesmooth_init(ns, lambda);
    return ns;
}

void    nodesmooth_setX(NodeSmooth* const ns, float x, Bool fix) {
    sp_set(&ns->x, x, fix);
    ns->nd.x = x;
}
void    nodesmooth_setY(NodeSmooth* const ns, float y, Bool fix) {
    sp_set(&ns->y, y, fix);
    ns->nd.y = y;
}
void    nodesmooth_setScaleX(NodeSmooth* const ns, float sx, Bool fix) {
    sp_set(&ns->sx, sx, fix);
    ns->nd.sx = sx;
}
void    nodesmooth_setScaleY(NodeSmooth* const ns, float sy, Bool fix) {
    sp_set(&ns->sy, sy, fix);
    ns->nd.sy = sy;
}
void    nodesmooth_setXRelToDef(NodeSmooth* const ns, float xShift, Bool fix) {
    float x = ns->x.def + xShift;
    sp_set(&ns->x, x, fix);
    ns->nd.x = x;
}
void    nodesmooth_setYRelToDef(NodeSmooth* const ns, float yShift, Bool fix) {
    float y = ns->y.def + yShift;
    sp_set(&ns->y, y, fix);
    ns->nd.y = y;
}

void    _nodesmooth_setRelatively(NodeSmooth* const ns, const Bool fix) {
    Node* parent = ns->nd.parent;
    if(parent == NULL) return;
    flag_t flags = ns->nd.flags;
    float xDec = 0;
    float yDec = 0;
    if(flags & flag_smoothRelativeToRight)
        xDec =  0.5f * parent->w;
    else if(flags & flag_smoothRelativeToLeft)
        xDec = -0.5f * parent->w;
    if(flags & flag_smoothRelativeToTop)
        yDec =  0.5f * parent->h;
    else if(flags & flag_smoothRelativeToBottom)
        yDec = -0.5f * parent->h;
    if(flags & flag_smoothJustifiedRight)
        xDec -= node_deltaX((Node*)ns);
    else if(flags & flag_smoothJustifiedLeft)
        xDec += node_deltaX((Node*)ns);
    if(flags & flag_smoothJustifiedTop)
        yDec -= node_deltaY((Node*)ns);
    else if(flags & flag_smoothJustifiedBottom)
        yDec += node_deltaY((Node*)ns);
    nodesmooth_setXRelToDef(ns, xDec, fix);
    nodesmooth_setYRelToDef(ns, yDec, fix);
}
void    node_smooth_open_default(Node* const node) {
    if(node->flags & flag_smoothRelativeFlags)
        _nodesmooth_setRelatively((NodeSmooth*)node, true);
    if(!(node->flags & flag_show) && (node->flags & flag_smoothFadeInRight))
        sp_fadeIn(&((NodeSmooth*)node)->x, NodeSmooth_defaultFadeInDelta);
}
void    node_smooth_close_fadeOut(Node* const node) {
    sp_fadeOut(&((NodeSmooth*)node)->x, NodeSmooth_defaultFadeInDelta);
}
void    node_smooth_reshape_relatively(Node* const node) {
    _nodesmooth_setRelatively((NodeSmooth*)node, false);
}
