//
//  node.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//
#include "node_base.h"

#include "node_fluid.h"
#include "node_squirrel.h"
#include "node_tree.h"

#include "../utils/util_base.h"
#include "../coq_timer.h"


#pragma mark -- Constructors... ----------------------------------

void node_disconnect_(Node * const node) {
    // Retrait
    if(node->_bigBro)
        node->_bigBro->_littleBro = node->_littleBro;
    else if(node->_parent) // Pas de grand frère -> probablement l'ainé.
        node->_parent->_firstChild = node->_littleBro;
    if(node->_littleBro)
        node->_littleBro->_bigBro = node->_bigBro;
    else if(node->_parent)  // Pas de petit frère -> probablement le cadet.
        node->_parent->_lastChild = node->_bigBro;
    // Deconnexion (superflu ??)
//    node->parent = NULL;
//    node->littleBro = NULL;
//    node->bigBro = NULL;
}
/** Connect au parent. (Doit être fullyDeconnect.) */
void node_connectToParent_(Node* const node, Node * const parentOpt, const bool asElder) {
    if(parentOpt == NULL) return;
    // Dans tout les cas, on a le parent:
    node->_parent = parentOpt;

    Node* oldParentFirstChild = parentOpt->_firstChild;
    Node* oldParentLastChild = parentOpt->_lastChild;
    // Cas parentOpt pas d'enfants
    if(oldParentLastChild == NULL || oldParentFirstChild == NULL) {
        parentOpt->_firstChild = node;
        parentOpt->_lastChild = node;
        return;
    }
    // Ajout au début
    if(asElder) {
        // Insertion
        node->_littleBro = oldParentFirstChild;
        node->_bigBro = NULL;
        // Branchement
        oldParentFirstChild->_bigBro = node;
        parentOpt->_firstChild = node;
    } else { // Ajout a la fin.
        // Insertion
        node->_bigBro = oldParentLastChild;
        node->_littleBro = NULL;
        // Branchement
        oldParentLastChild->_littleBro = node;
        parentOpt->_lastChild = node;
    }
}
void node_connectToBro_(Node* const node, Node* const broOpt, const bool asBigbro) {
    if(broOpt == NULL) return;
    Node * const bro = broOpt;
    Node * const parent = bro->_parent;
    if(parent == NULL) { printerror("Boucle sans parent."); return; }
    node->_parent = parent;

    if(asBigbro) {
        // Insertion
        node->_littleBro = bro;
        node->_bigBro = bro->_bigBro;
        // Branchement
        bro->_bigBro = node;
        if(node->_bigBro != NULL)
            node->_bigBro->_littleBro = node;
        else
            node->_parent->_firstChild = node;
    } else {
        // Insertion
        node->_littleBro = bro->_littleBro;
        node->_bigBro = bro;
        // Branchement
        bro->_littleBro = node;
        if(node->_littleBro != NULL)
            node->_littleBro->_bigBro = node;
        else
            node->_parent->_lastChild = node;
    }
}

Node* node_last_nonDrawable = NULL;
#ifdef DEBUG
static uint32_t Node_currentId_ = 0;
#endif

void node_init_(Node* n, Node* const refOpt,
                 float x, float y, float width, float height,
                 const uint32_t type, const flag_t flags, const uint8_t node_place) {
    n->_piu =    piu_default;
    n->w =      width;
    n->h =      height;
    n->scales = vector2_ones;\
    n->x =      x;
    n->y =      y;
    n->flags =  flags;
//    n->_size =  size;
    n->_type =  type;
    n->updateModel = node_updateModelMatrixDefault;
#ifdef DEBUG
    n->_nodeId = Node_currentId_;
    Node_currentId_ ++;
#endif
    if(refOpt) {
        node_simpleMoveTo(n, refOpt, node_place);
    } else if(!(flags & flag_noParent) && !(type & node_type_flag_root)) {
        printwarning("Node in void (non root without parent or brother ref).");
    }
    if(!(type & node_type_flag_drawable))
        node_last_nonDrawable = n;
}
/// Allocation d'un noeud de type quelconque.
/// Obsolete : utiliser `coq_calloc` + `node_init_`...
void* Node_createOfType_Obsolete_(const uint32_t type, size_t size, const flag_t flags,
                              Node* const refOpt, const uint8_t node_place) {
    if(size < sizeof(Node)) {
        printwarning("size < sizeof(Node).");
        size = sizeof(Node);
    }
    Node* n = coq_calloc(1, size);
    node_init_(n, refOpt, 0, 0, 1, 1, type, flags, node_place);
    return n;
}
Node* Node_createEmpty(void)
{
    Node* n = coq_calloc(1, sizeof(Node));
    node_init_(n, NULL, 0, 0, 1, 1, node_type_node, 0, 0);
    return n;
}
// Constructeur usuel.
Node* Node_create(Node* const refOpt, const float x, const float y, const float w, const float h,
                  const flag_t flags, const uint8_t node_place) {
    Node* n = coq_calloc(1, sizeof(Node));
    node_init_(n, refOpt, x, y, w, h, node_type_node, flags, node_place);
    return n;
}
//Node* Node_default_createCopy(const Node* const other) {
//    if(other->_type & node_type_flag_notCopyable) {
//        printerror("Node of type %x is not copyable.", other->_type);
//        return NULL;
//    }
//    Node* nd = coq_malloc(sizeof(other->_size));
//    // Copie toute les donnees a priori.
//    memcpy(nd, other, sizeof(other->_size));
//    // Mais on remet les pointeurs sur null.
//    nd->parent = NULL;
//    nd->firstChild = NULL;
//    nd->lastChild = NULL;
//    nd->bigBro = NULL;
//    nd->littleBro = NULL;
//    if(!(nd->_type & node_type_flag_leaf))
//        node_last_nonLeaf = nd;
//    return nd;
//}

#pragma mark - Garbage, emplacement temporaire avant d être deallocated.

void node_tree_burnDown_(Node* const node) {
    if(node->_firstChild == NULL) {
        if(node->deinitOpt) node->deinitOpt(node);
        coq_free(node);
        return;
    }
    Node* pos = node;
    Node* toDelete;
go_down:
    while(pos->_firstChild)
        pos = pos->_firstChild;
set_toDelete:
    toDelete = pos;
    if(pos->_littleBro) {
        pos = pos->_littleBro;
        if(toDelete->deinitOpt) toDelete->deinitOpt(toDelete);
        coq_free(toDelete);
        goto go_down;
    }
    pos = pos->_parent;
    if(toDelete->deinitOpt) toDelete->deinitOpt(toDelete);
    coq_free(toDelete);
    // Stop ?
    if(pos == NULL) {
        printerror("Root not found.");
        return;
    }
    if(pos == node) {
        if(node->deinitOpt) node->deinitOpt(node);
        coq_free(node);
        return;
    }
    goto set_toDelete;
}
struct _Garbage {
    Node** container;
    uint32_t   count;
    uint32_t   index;
};
void _garbage_putNode(struct _Garbage* garbage, Node* toDelete) {
    if(garbage->index >= garbage->count) {
        garbage->count += 32;
        garbage->container = coq_realloc(garbage->container, sizeof(Node*) * garbage->count);
    }
    garbage->container[garbage->index] = toDelete;
    garbage->index ++;
}
void _garbage_burnDown(struct _Garbage* garbage) {
    Node** end = &garbage->container[garbage->index];
    for(Node** ptr = garbage->container; ptr < end; ptr++)
        node_tree_burnDown_(*ptr);
    garbage->index = 0;
}
static bool   _garbageA_active = true;
static struct _Garbage _garbageA = {NULL, 0, 0};
static struct _Garbage _garbageB = {NULL, 0, 0};
void  node_tree_throwToGarbage(Node* const node) {
    node->flags |= flag_toDelete_;
    Node* pos = node->_firstChild;
    if(pos == NULL)
        goto put_to_garbage;
add_flag_toDelete:
    pos->flags |= flag_toDelete_;
    if(pos->_firstChild) {
        pos = pos->_firstChild;
        goto add_flag_toDelete;
    }
check_littleBro:
    if(pos->_littleBro) {
        pos = pos->_littleBro;
        goto add_flag_toDelete;
    }
    pos = pos->_parent;
    if(pos == node)
        goto put_to_garbage;
    if(pos == NULL) {
        printerror("Root not found.");
        return;
    }
    goto check_littleBro;
put_to_garbage:
    node_disconnect_(node);
    if(_garbageA_active)
        _garbage_putNode(&_garbageA, node);
    else
        _garbage_putNode(&_garbageB, node);
}
void  NodeGarbage_burn(void) {
    if(_garbageA_active)
        _garbage_burnDown(&_garbageB);
    else
        _garbage_burnDown(&_garbageA);
    _garbageA_active = !_garbageA_active;
}

void  noderef_fastDestroyAndNull(Node** const nodeOptRef) {
    if(*nodeOptRef == NULL) return;
    node_tree_throwToGarbage(*nodeOptRef);
    *nodeOptRef = NULL;
}
void  noderef_slowDestroyAndNull(Node** const nodeOptRef, int64_t deltaTimeMSOpt) {
    if(*nodeOptRef == NULL) return;
    if(deltaTimeMSOpt == 0) deltaTimeMSOpt = 400;
    node_tree_close(*nodeOptRef);
    timer_scheduled(NULL, deltaTimeMSOpt, false, *nodeOptRef, node_tree_throwToGarbage);
    *nodeOptRef = NULL;
}


#pragma mark -- Getters -------------------------------------------

Vector2 node_deltas(Node* const node) {
    Vector2 ds = {{node->w * node->sx / 2.f, node->h * node->sy / 2.f}};
    return ds;
}
float   node_deltaX(Node* const node) {
    return node->w * node->sx / 2.f;
}
float   node_deltaY(Node* const node) {
    return node->h * node->sy / 2.f;
}
Vector3 node_pos(Node *node) {
    Fluid* s = node_asFluidOpt(node);
    if(s) return fl_array_toVec3(&s->x);
    else  return node->pos;
}
Vector2 node_scales(Node *node) {
    Fluid* s = node_asFluidOpt(node);
    if(s) return fl_array_toVec2(&s->sx);
    else  return node->scales;
}


#pragma mark -- Setters -------------------------------------------
void    node_setXYrelatively(Node* node, uint32_t relative_flags, bool open) {

    Node* parent = node->_parent;
    Fluid* f = node_asFluidOpt(node);
    float x, y;
    // Positions a priori.
    if(f) {
        x = f->x.def; y = f->y.def;
    } else {
        x = node->x;  y = node->y;
    }
    // Ajustement par rapport au parent
    if(parent) {
        if(relative_flags & flag_fluidRelativeToRight)
            x +=  0.5f * parent->w;
        else if(relative_flags & flag_fluidRelativeToLeft)
            x -=  0.5f * parent->w;
        if(relative_flags & flag_fluidRelativeToTop)
            y +=  0.5f * parent->h;
        else if(relative_flags & flag_fluidRelativeToBottom)
            y -=  0.5f * parent->h;
    }
    // La position est considéré à droite/gauche haut/bas ?
    if(relative_flags & flag_fluidJustifiedRight)
        x -= node_deltaX(node);
    else if(relative_flags & flag_fluidJustifiedLeft)
        x += node_deltaX(node);
    if(relative_flags & flag_fluidJustifiedTop)
        y -= node_deltaY(node);
    else if(relative_flags & flag_fluidJustifiedBottom)
        y += node_deltaY(node);
    // Mise à jour de la position...
    if(f) {
        if(open) {
            if(relative_flags & flag_fluidFadeInRight) {
                fl_fix(&f->x, x + Fluid_defaultFadeInDelta);
                fl_set(&f->x, x);
            } else {
                fl_fix(&f->x, x);
            }
            fl_fix(&f->y, y);
        } else {
            fl_set(&f->x, x);  fl_set(&f->y, y);
        }
    }
    node->x = x;
    node->y = y;
}
//void    node_setX(Node* const nd, float x){
//    Fluid* s = node_asFluidOpt(nd);
//    if(s) fl_set(&s->x, x);
//    nd->x = x;
//}
//void    node_fixX(Node* const nd, float x){
//    Fluid* s = node_asFluidOpt(nd);
//    if(s) fl_fix(&s->x, x);
//    nd->x = x;
//}
//void    node_setY(Node* const nd, float y) {
//    Fluid* s = node_asFluidOpt(nd);
//    if(s) fl_set(&s->y, y);
//    nd->y = y;
//}
//void    node_fixY(Node* const nd, float y) {
//    Fluid* s = node_asFluidOpt(nd);
//    if(s) fl_fix(&s->y, y);
//    nd->y = y;
//}
//void    node_setScaleX(Node* const nd, float sx) {
//    Fluid* s = node_asFluidOpt(nd);
//    if(s) fl_set(&s->sx, sx);
//    nd->sx = sx;
//}
//void    node_fixScaleX(Node* const nd, float sx) {
//    Fluid* s = node_asFluidOpt(nd);
//    if(s) fl_fix(&s->sx, sx);
//    nd->sx = sx;
//}
//void    node_setScaleY(Node* const nd, float sy) {
//    Fluid* s = node_asFluidOpt(nd);
//    if(s) fl_set(&s->sy, sy);
//    nd->sy = sy;
//}
//void    node_fixScaleY(Node* const nd, float sy) {
//    Fluid* s = node_asFluidOpt(nd);
//    if(s) fl_fix(&s->sy, sy);
//    nd->sy = sy;
//}


#pragma mark -- Moving node ---------------------------------------------

void    node_simpleMoveTo(Node* const node, Node* const destOpt, const uint8_t node_place) {
    node_disconnect_(node);
    if(node_place & node_place_asBro)
        node_connectToBro_(node, destOpt, node_place & node_place_asElderBig);
    else
        node_connectToParent_(node, destOpt, node_place & node_place_asElderBig);
}
void    node_simpleMoveToBro(Node* const node, Node* const broOpt, const bool asBig) {
    node_disconnect_(node);
    node_connectToBro_(node, broOpt, asBig);
}
void    node_simpleMoveToParent(Node* const node, Node* const parentOpt, const bool asElder) {
    node_disconnect_(node);
    node_connectToParent_(node, parentOpt, asElder);
}
void    node_moveWithinBro(Node* const node, bool asElder) {
    if(asElder && (node->_bigBro == NULL)) return;
    if(!asElder && (node->_littleBro == NULL)) return;
    Node* const parent = node->_parent;
    if(!parent) { printerror("No parent."); return; }
    // Retrait
    if(node->_bigBro)
        node->_bigBro->_littleBro = node->_littleBro;
    else  // Pas de grand frère -> probablement l'ainé.
        parent->_firstChild = node->_littleBro;
    if(node->_littleBro)
        node->_littleBro->_bigBro = node->_bigBro;
    else  // Pas de petit frère -> probablement le cadet.
        parent->_lastChild = node->_bigBro;

    if(asElder) {
        // Insertion
        node->_littleBro = parent->_firstChild;
        node->_bigBro = NULL;
        // Branchement
        if(parent->_firstChild)
            parent->_firstChild->_bigBro = node;
        parent->_firstChild = node;
    } else {
        // Insertion
        node->_littleBro = NULL;
        node->_bigBro = parent->_lastChild;
        // Branchement
        if(parent->_lastChild)
            parent->_lastChild->_littleBro = node;
        parent->_lastChild = node;
    }
}

void    node_setInReferentialOf_(Node* const n, Node* const ref) {
    Squirrel sqP, sqQ;
    sq_init(&sqP, n, sq_scale_ones);
    while(sq_goUpPS(&sqP)) {}
    sq_init(&sqQ, ref, sq_scale_scales);
    while(sq_goUpPS(&sqQ)) {}

    Fluid* f = node_asFluidOpt(n);
    if(f) {
        fl_newReferential(&f->x, sqP.v.x, sqQ.v.x, sqP.s.x, sqQ.s.x);
        fl_newReferential(&f->y, sqP.v.y, sqQ.v.y, sqP.s.y, sqQ.s.y);
        fl_newReferentialAsDelta(&f->sx, sqP.s.x, sqQ.s.x);
        fl_newReferentialAsDelta(&f->sy, sqP.s.y, sqQ.s.y);
    }
    n->x = (sqP.v.x - sqQ.v.x) / sqQ.s.x;
    n->y = (sqP.v.y - sqQ.v.y) / sqQ.s.y;
    n->sx = n->sx * sqP.s.x / sqQ.s.x;
    n->sy = n->sy * sqP.s.y / sqQ.s.y;
}
/** Change de noeud de place (et ajuste sa position/scaling relatif). */
void    node_moveToParent(Node* const n, Node* const new_parent, bool const asElder) {
    node_setInReferentialOf_(n, new_parent);
    node_disconnect_(n);
    node_connectToParent_(n, new_parent, asElder);
}
/** Change de noeud de place (et ajuste sa position/scaling relatif). */
void    node_moveToBro(Node* n, Node* new_bro, bool asBigBro) {
    Node* new_parent = new_bro->_parent;
    if(!new_parent) { printerror("Bro sans parent."); return; }
    node_setInReferentialOf_(n, new_parent);
    node_disconnect_(n);
    node_connectToBro_(n, new_bro, asBigBro);
}


#pragma mark -- Vector, Matrix... --------------------------------------

/// Mise à jour ordinaire de la matrice modèle pour se placer dans le référentiel du parent.
Drawable* node_updateModelMatrixDefault(Node *n) {
    // Cas feuille, skip...
    if(n->_firstChild == NULL) {
//    #ifdef DEBUG
//        printwarning("Update of leaf %s id %d without child?", node_debug_getTypeName(n), n->_nodeId);
//    #endif 
        return NULL;
    }
    const Node* const parent = n->_parent;
    if(!parent) { printwarning("Non root without parent."); return NULL; }
    const Matrix4* const pm = &parent->_piu.model;
    Matrix4* m = &n->_piu.model;
    Vector3 pos = node_pos(n);
    Vector2 scl = node_scales(n);
    m->v0.v = pm->v0.v * scl.x;
    m->v1.v = pm->v1.v * scl.y;
    m->v2 =   pm->v2;  // *scl.z ... si on veut un scaling en z...?
    m->v3 = (Vector4) {{
        pm->v3.x + pm->v0.x * pos.x + pm->v1.x * pos.y + pm->v2.x * pos.z,
        pm->v3.y + pm->v0.y * pos.x + pm->v1.y * pos.y + pm->v2.y * pos.z,
        pm->v3.z + pm->v0.z * pos.x + pm->v1.z * pos.y + pm->v2.z * pos.z,
        pm->v3.w,
    }};
    return NULL;
}
/// Donne la position du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du noeud.
Vector2 node_posInParentReferential(Node* n, const Node* const parentOpt) {
    Squirrel sq;
    sq_init(&sq, n, sq_scale_ones);
    do {
        if(sq.pos->_parent == parentOpt)
            return sq.v;
    } while (sq_goUpP(&sq));
    printerror("No parent encountered.");
    return sq.v;
}
/// Donne la position et les dimension (en demi-largeur/demi-hauteur)
/// du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du noeud.
Box     node_hitBoxInParentReferential(Node* n, const Node* parentOpt) {
    Squirrel sq;
    sq_init(&sq, n, sq_scale_deltas);
    do {
        if(sq.pos->_parent == parentOpt) {
            return (Box){ .center = sq.v, .deltas = sq.s };
        }
    } while (sq_goUpPS(&sq));

    printerror("No parent encountered.");
    return (Box){ .center = sq.v, .deltas = sq.s };
}
/// Convertie une position absolue (au niveau de la root) en une position
///  dans le réferentiel de nodeOpt (si NULL -> reste absPos).
Vector2 vector2_absPosToPosInReferentialOfNode(Vector2 const absPos, Node* nodeOpt) {
    if(nodeOpt == NULL) return absPos;
    Squirrel sq;
    sq_init(&sq, nodeOpt, sq_scale_scales);
    while (sq_goUpPS(&sq));
    return vector2_inReferentialOfSquirrel(absPos, &sq);
}


#pragma mark - Debug...

const char* node_type_names_[] = {
    "node",
    "fluid",
    "button",
    "scrollable",
    "drawable",
    "frame",
    "root",
    "view",
    "secHov",
    "drawMulti",
    "number",
};

const char* node_debug_getTypeName(const Node* const n) {
    uint32_t index = 32 - __builtin_clz(n->_type & node_type_flags_defaultTypes);
    return node_type_names_[index];
}
