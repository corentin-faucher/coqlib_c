//
//  fluid.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-16.
//

#include "node_fluid.h"

#include "../utils/util_base.h"

void fluid_renderer_defaultUpdateInstanceUniforms_(Node *const n) {
    // Cas feuille, skip...
    if(n->_firstChild == NULL) {
        return;
    }
    Fluid *const f = (Fluid*)n;
    const Matrix4* const pm = node_parentModel(n);
    Matrix4* const m = &n->renderIU.model;
    Vector3 const pos = fl_array_toVec3(&f->x);
    Vector3 const scl = fl_array_toVec3(&f->sx);
    // Equivalent de :
//    *m = *pm;
//    matrix4_translate(m, pos);
//    matrix4_scale(m, scl);
    m->v0.v = pm->v0.v * scl.x;
    m->v1.v = pm->v1.v * scl.y;
    m->v2.v = pm->v2.v * scl.z; 
    m->v3.v = pm->v3.v + pm->v0.v * pos.x + pm->v1.v * pos.y + pm->v2.v * pos.z;
}

float  Fluid_defaultFadeInDelta = 2.2f;
void (*Fluid_renderer_defaultUpdateInstanceUniforms)(Node*) = fluid_renderer_defaultUpdateInstanceUniforms_;

void fluid_init(Fluid* f, float lambda) {
    f->n._type |= node_type_fluid;
    f->n.renderer_updateInstanceUniforms = Fluid_renderer_defaultUpdateInstanceUniforms;
    fl_array_init(&f->sx, &f->n.sx, COQ_FLUID_DIMS_N, lambda);
    // Fonction de positionnement pour les node smooth.
    if(f->n.flags & flags_fluidOpen)
        f->n.openOpt = fluid_open_;
    if(f->n.flags & flags_fluidClose)
        f->n.closeOpt = fluid_close_;
    if(f->n.flags & flags_fluidReshape)
        f->n.reshapeOpt = fluid_reshape_;
}
Fluid* Fluid_create(Node* const refOpt, float x, float y, float w, float h,
                              float lambda, flag_t flags, uint8_t node_place) {
    Fluid* f = coq_callocTyped(Fluid);
    node_init(&f->n, refOpt, x, y, w, h, flags, node_place);
    fluid_init(f, lambda);
    return f;
}


Box     fluid_referential(Fluid *const f) {
    return (Box) {
        .center = fl_array_toVec2(&f->x),
        .deltas = fl_array_toVec2(&f->sx),
    };
}


void    fluid_setXY(Fluid* f, Vector2 xy) {
    fl_set(&f->x, xy.x); fl_set(&f->y, xy.y);
    f->n.x = xy.x;       f->n.y = xy.y;
}
void    fluid_fixXY(Fluid* f, Vector2 xy) {
    fl_fix(&f->x, xy.x); fl_fix(&f->y, xy.y);
    f->n.x = xy.x;       f->n.y = xy.y;
}
void    fluid_setXYScales(Fluid* f, Box box) {
    fl_set(&f->x, box.c_x); fl_set(&f->y, box.c_y);
    f->n.x = box.c_x;       f->n.y = box.c_y;
    fl_set(&f->sx, box.Dx); fl_set(&f->sy, box.Dy);
    f->n.sx = box.Dx;       f->n.sy = box.Dy;
}
void    fluid_setX(Fluid* f, float x) {
    fl_set(&f->x, x); f->n.x = x;
}
void    fluid_fixX(Fluid *f, float x) {
    fl_fix(&f->x, x); f->n.x = x;
}
void    fluid_setY(Fluid* f, float y) {
    fl_set(&f->y, y); f->n.y = y;
}
void    fluid_fixY(Fluid* f, float y) {
    fl_fix(&f->y, y); f->n.y = y;
}
void    fluid_setXrelToDef(Fluid* f, float x, bool fix) {
    if(fix) fl_fix(&f->x, f->x.def + x);
    else    fl_set(&f->x, f->x.def + x);
    f->n.x = f->x.def + x;
}

void    fluid_setYrelToDef(Fluid* f, float y, bool fix) {
    if(fix) fl_fix(&f->y, f->y.def + y);
    else    fl_set(&f->y, f->y.def + y);
    f->n.y = f->y.def + y;
}
void    fluid_setZrelToDef(Fluid* f, float z, bool fix) {
    if(fix) fl_fix(&f->z, f->z.def + z);
    else    fl_set(&f->z, f->z.def + z);
    f->n.z = f->z.def + z;
}
void   fluid_setScales(Fluid *const f, Vector3 const scales) {
    fl_set(&f->sx, scales.x);
    fl_set(&f->sy, scales.y);
    fl_set(&f->sz, scales.z);
    f->n.scales = scales;
}
void   fluid_fixScales(Fluid *const f, Vector3 const scales) {
    fl_fix(&f->sx, scales.x);
    fl_fix(&f->sy, scales.y);
    fl_fix(&f->sz, scales.z);
    f->n.scales = scales;
}

const PopingInfo popinginfo_default = {
    {{ 0.0, 0.0, -0.4, -0.2 }},
    {{ 0.0, 1.0,  0.0,  0.0 }},
    10, 25, 10, 25,
};
const PopingInfo popinginfo_zeros = { 0 };

/// Applique un effet d'apparition. *Non compatible avec les `flags_fluidOpen` et `flags_fluidClose`*,
/// i.e. Soit on utilise le fluid pour setter relativement, soint on l'utilies pour l'effet `poping`.
/// Si "init" -> On set les position `def` des fluid aux position du noeud.
void    fluid_popIn(Fluid* f, PopingInfo popInfo) {
    // Calcul des position initiales et finales (relatif aux position par défaut).
    Node* n = &f->n;
    float const twoDy = f->sy.def * n->h;
    Box box0 = {{  // (x, y, sx, sy)
        f->x.def  + popInfo.initRelShift.c_x * twoDy,
        f->y.def  + popInfo.initRelShift.c_y * twoDy,
        f->sx.def * (1.f + popInfo.initRelShift.Dx),
        f->sx.def * (1.f + popInfo.initRelShift.Dy),
    }};
    Box box1 = {{
        f->x.def  + popInfo.endRelShift.c_x * twoDy,
        f->y.def  + popInfo.endRelShift.c_y * twoDy,
        f->sx.def * (1.f + popInfo.endRelShift.Dx),
        f->sx.def * (1.f + popInfo.endRelShift.Dy),
    }};
    // Init/Fix à position init et paramètres gamma/k.
    fl_initGammaK(&f->sx, box0.Dx,  popInfo.gammaScale, popInfo.kScale, false);
    fl_initGammaK(&f->sy, box0.Dy,  popInfo.gammaScale, popInfo.kScale, false);
    fl_initGammaK(&f->x,  box0.c_x, popInfo.gammaPos,   popInfo.kPos, false);
    fl_initGammaK(&f->y,  box0.c_y, popInfo.gammaPos,   popInfo.kPos, false);
    fl_initGammaK(&f->z,  n->z,     popInfo.gammaPos,   popInfo.kPos, false);
    // Set à position final.
    fl_set(&f->sx, box1.Dx);
    fl_set(&f->sy, box1.Dy);
    fl_set(&f->x,  box1.c_x); 
    fl_set(&f->y,  box1.c_y);
    n->sx = box1.Dx; 
    n->sy = box1.Dy;
    n->x = box1.c_x;
    n->y = box1.c_y;
}
/// Pour popOut, seulement la box `endRelShift` est utilisé.
void    fluid_popOut(Fluid* f, PopingInfo popInfo) {
    // Calcul des position finales
    Node* n = &f->n;
    float const twoDy = f->sy.def * n->h;
    Box box1 = {{
        f->x.def  + popInfo.endRelShift.c_x * twoDy,
        f->y.def  + popInfo.endRelShift.c_y * twoDy,
        f->sx.def * (1.f + popInfo.endRelShift.Dx),
        f->sx.def * (1.f + popInfo.endRelShift.Dy),
    }};
    // Update des par. gamma/k.
    fl_updateToConstants(&f->sx, popInfo.gammaScale, popInfo.kScale);
    fl_updateToConstants(&f->sy, popInfo.gammaScale, popInfo.kScale);
    fl_updateToConstants(&f->x,  popInfo.gammaPos, popInfo.kPos);
    fl_updateToConstants(&f->y,  popInfo.gammaPos, popInfo.kPos);
    fl_updateToConstants(&f->z,  popInfo.gammaPos, popInfo.kPos);
    // Set à position final.
    fl_set(&f->sx, box1.Dx);
    fl_set(&f->sy, box1.Dy);
    fl_set(&f->x,  box1.c_x); 
    fl_set(&f->y,  box1.c_y);
    n->sx = box1.Dx; 
    n->sy = box1.Dy;
    n->x = box1.c_x;
    n->y = box1.c_y;
}

void    fluid_open_(Node* const node) {
    node_setXYrelatively(node, (uint32_t)node->flags, true);
}
void    fluid_close_(Node* const node) {
    Fluid* s = node_asFluidOpt(node);
    if(!s) { printerror("Not smooth."); return; }
    fl_fadeOut(&s->x, Fluid_defaultFadeInDelta);
}
void    fluid_reshape_(Node* const node) {
    node_setXYrelatively(node, (uint32_t)node->flags, false);
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
