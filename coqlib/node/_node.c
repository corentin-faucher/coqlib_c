//
//  node.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include <stdlib.h>
#include "_node.h"
#include "fluid.h"
#include "drawable.h"
#include "_node_flags.h"
#include "node_squirrel.h"
#include "node_tree.h"
#include "utils.h"

void _node_disconnect(Node * const node) {
    // Retrait
    if(node->bigBro)
        node->bigBro->littleBro = node->littleBro;
    else if(node->parent) // Pas de grand frère -> probablement l'ainé.
        node->parent->firstChild = node->littleBro;
    if(node->littleBro)
        node->littleBro->bigBro = node->bigBro;
    else if(node->parent)  // Pas de petit frère -> probablement le cadet.
        node->parent->lastChild = node->bigBro;
    // Deconnexion (superflu ??)
//    node->parent = NULL;
//    node->littleBro = NULL;
//    node->bigBro = NULL;
}
/** Connect au parent. (Doit être fullyDeconnect.) */
void _node_connectToParent(Node* const node, Node * const parentOpt, const Bool asElder) {
    if(parentOpt == NULL) return;
    if(parentOpt->_type & node_type_flag_noChildren) {
        printerror("Adding child to leaf node.");
        return;
    }
    // Dans tout les cas, on a le parent:
    node->parent = parentOpt;
    
    Node* oldParentFirstChild = parentOpt->firstChild;
    Node* oldParentLastChild = parentOpt->lastChild;
    // Cas parentOpt pas d'enfants
    if(oldParentLastChild == NULL || oldParentFirstChild == NULL) {
        parentOpt->firstChild = node;
        parentOpt->lastChild = node;
        return;
    }
    // Ajout au début
    if(asElder) {
        // Insertion
        node->littleBro = oldParentFirstChild;
        // Branchement
        oldParentFirstChild->bigBro = node;
        parentOpt->firstChild = node;
    } else { // Ajout a la fin.
        // Insertion
        node->bigBro = oldParentLastChild;
        // Branchement
        oldParentLastChild->littleBro = node;
        parentOpt->lastChild = node;
    }
}
void _node_connectToBro(Node* const node, Node* const broOpt, const Bool asBigbro) {
    if(broOpt == NULL) return;
    Node * const bro = broOpt;
    Node * const parent = bro->parent;
    if(parent == NULL) { printerror("Boucle sans parent."); return; }
    node->parent = parent;
    
    if(asBigbro) {
        // Insertion
        node->littleBro = bro;
        node->bigBro = bro->bigBro;
        // Branchement
        bro->bigBro = node;
        if(node->bigBro != NULL)
            node->bigBro->littleBro = node;
        else
            node->parent->firstChild = node;
    } else {
        // Insertion
        node->littleBro = bro->littleBro;
        node->bigBro = bro;
        // Branchement
        bro->littleBro = node;
        if(node->littleBro != NULL)
            node->littleBro->bigBro = node;
        else
            node->parent->lastChild = node;
    }
}

Node* _node_last_created = NULL;
// Allocation d'un noeud de type quelconque.
void* _Node_createEmptyOfType(const uint32_t type, size_t size, const flag_t flags,
                              Node* const refOpt, const uint8_t node_place) {
    if(size < sizeof(Node)) {
        printwarning("size < sizeof(Node).");
        size = sizeof(Node);
    }
    Node* nd = coq_calloc(1, size);
    nd->flags = flags;
    nd->_type = type;
    nd->_size = size;
    nd->piu = piu_default;
    nd->scales = vector2_ones;
    if(refOpt) {
        node_simpleMoveTo(nd, refOpt, node_place);
    }
    // Les boutons et scrollable sont "selectionnable".
    if(type & node_type_flag_button)
        node_tree_addRootFlag(nd, flag_parentOfButton);
    if(type & node_type_flag_scrollable)
        node_tree_addRootFlag(nd, flag_parentOfScrollable);
    if(!(type & node_type_flag_noChildren))
        _node_last_created = nd;
    return nd;
}
Node* Node_createEmpty(void)
{
    return _Node_createEmptyOfType(node_type_bare, sizeof(Node), 0, NULL, 0);
}
// Constructeur usuel.
Node* Node_create(Node* const ref, const float x, const float y, const float w, const float h,
                  const flag_t flags, const uint8_t node_place) {
    Node *node = _Node_createEmptyOfType(node_type_bare, sizeof(Node), flags, ref, node_place);
    node->x = x;
    node->y = y;
    node->w = w;
    node->h = h;
    return node;
}
Node* Node_default_createCopy(const Node* const other) {
    if(other->_type & node_type_flag_notCopyable) {
        printerror("Node of type %x is not copyable.", other->_type);
        return NULL;
    }
    Node* nd = coq_malloc(sizeof(other->_size));
    // Copie toute les donnees a priori.
    memcpy(nd, other, sizeof(other->_size));
    // Mais on remet les pointeurs sur null.
    nd->parent = NULL;
    nd->firstChild = NULL;
    nd->lastChild = NULL;
    nd->bigBro = NULL;
    nd->littleBro = NULL;
    if(!(nd->_type & node_type_flag_noChildren))
        _node_last_created = nd;
    return nd;
}

/*-- Garbage --------------------------------*/
void _node_tree_burnDown(Node* const node) {
    if(node->firstChild == NULL) {
        if(node->deinit) node->deinit(node);
        coq_free(node);
        return;
    }
    Node* pos = node;
    Node* toDelete;
go_down:
    while(pos->firstChild)
        pos = pos->firstChild;
set_toDelete:
    toDelete = pos;
    if(pos->littleBro) {
        pos = pos->littleBro;
        if(toDelete->deinit) toDelete->deinit(toDelete);
        coq_free(toDelete);
        goto go_down;
    }
    pos = pos->parent;
    if(toDelete->deinit) toDelete->deinit(toDelete);
    coq_free(toDelete);
    // Stop ?
    if(pos == NULL) {
        printerror("Root not found.");
        return;
    }
    if(pos == node) {
        if(node->deinit) node->deinit(node);
        coq_free(node);
        return;
    }
    goto set_toDelete;
}
struct _Garbage {
    Node** container;
    uint   count;
    uint   index;
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
        _node_tree_burnDown(*ptr);
    garbage->index = 0;
}
static Bool   _garbageA_active = true;
static struct _Garbage _garbageA = {NULL, 0, 0};
static struct _Garbage _garbageB = {NULL, 0, 0};
void  node_tree_throwToGarbage(Node* const nd) {
    nd->flags |= _flag_toDelete;
    Node* pos = nd -> firstChild;
    if(pos == NULL)
        goto put_to_garbage;
add_flag_toDelete:
    pos->flags |= _flag_toDelete;
    if(pos->firstChild) {
        pos = pos->firstChild;
        goto add_flag_toDelete;
    }
check_littleBro:
    if(pos->littleBro) {
        pos = pos->littleBro;
        goto add_flag_toDelete;
    }
    pos = pos->parent;
    if(pos == nd)
        goto put_to_garbage;
    if(pos == NULL) {
        printerror("Root not found.");
        return;
    }
    goto check_littleBro;
put_to_garbage:
    _node_disconnect(nd);
    if(_garbageA_active)
        _garbage_putNode(&_garbageA, nd);
    else
        _garbage_putNode(&_garbageB, nd);
}
void  NodeGarbage_burn(void) {
    if(_garbageA_active)
        _garbage_burnDown(&_garbageB);
    else
        _garbage_burnDown(&_garbageA);
    _garbageA_active = !_garbageA_active;
}


/*-- Getters -------------------------------------------*/
Vector2 node_deltas(Node* const node) {
    Vector2 ds = {node->w * node->sx / 2.f, node->h * node->sy / 2.f};
    return ds;
}
float   node_deltaX(Node* const node) {
    return node->w * node->sx / 2.f;
}
float   node_deltaY(Node* const node) {
    return node->h * node->sy / 2.f;
}
int     node_isDisplayActive(Node* const node) {
    if(node->_type & node_type_flag_drawable) {
        return smtrans_isActive(((Drawable*)node)->trShow);
    }
    return node->flags & (flag_show|flag_parentOfToDisplay);
}
Vector3 node_pos(Node *node) {
    Fluid* s = node_asFluidOpt(node);
    if(s)
        return sp_array_toVec3(&s->x);
    else
        return node->pos;
}
Vector2 node_scales(Node *node) {
    Fluid* s = node_asFluidOpt(node);
    if(s)
        return sp_array_toVec2(&s->sx);
    else
        return node->scales;
}

/*-- Setters -------------------------------------------*/
void    node_setX(Node* const nd, float x, Bool fix){
    Fluid* s = node_asFluidOpt(nd);
    if(s) sp_set(&s->x, x, fix);
    nd->x = x;
}
void    node_setY(Node* const nd, float y, Bool fix) {
    Fluid* s = node_asFluidOpt(nd);
    if(s) sp_set(&s->y, y, fix);
    nd->y = y;
}
void    node_setScaleX(Node* const nd, float sx, Bool fix) {
    Fluid* s = node_asFluidOpt(nd);
    if(s) sp_set(&s->sx, sx, fix);
    nd->sx = sx;
}
void    node_setScaleY(Node* const nd, float sy, Bool fix) {
    Fluid* s = node_asFluidOpt(nd);
    if(s) sp_set(&s->sy, sy, fix);
    nd->sy = sy;
}

/*-- Moving node ---------------------------------------------*/
void    node_simpleMoveTo(Node* const node, Node* const destOpt, const uint8_t node_place) {
    _node_disconnect(node);
    if(node_place & node_place_asBro)
        _node_connectToBro(node, destOpt, node_place & node_place_asElderBig);
    else
        _node_connectToParent(node, destOpt, node_place & node_place_asElderBig);
}
void    node_simpleMoveToBro(Node* const node, Node* const broOpt, const Bool asBig) {
    _node_disconnect(node);
    _node_connectToBro(node, broOpt, asBig);
}
void    node_simpleMoveToParent(Node* const node, Node* const parentOpt, const Bool asElder) {
    _node_disconnect(node);
    _node_connectToParent(node, parentOpt, asElder);
}

void    node_moveWithinBroAsElder(Node* const node, Bool asElder) {
    if(asElder && (node->bigBro == NULL)) return;
    if(!asElder && (node->littleBro == NULL)) return;
    Node* const parent = node->parent;
    if(!parent) { printerror("No parent."); return; }
    // Retrait
    if(node->bigBro)
        node->bigBro->littleBro = node->littleBro;
    else  // Pas de grand frère -> probablement l'ainé.
        parent->firstChild = node->littleBro;
    if(node->littleBro)
        node->littleBro->bigBro = node->bigBro;
    else  // Pas de petit frère -> probablement le cadet.
        parent->lastChild = node->bigBro;
    
    if(asElder) {
        // Insertion
        node->littleBro = parent->firstChild;
        node->bigBro = NULL;
        // Branchement
        if(parent->firstChild)
            parent->firstChild->bigBro = node;
        parent->firstChild = node;
    } else {
        // Insertion
        node->littleBro = NULL;
        node->bigBro = parent->lastChild;
        // Branchement
        if(parent->lastChild)
            parent->lastChild->littleBro = node;
        parent->lastChild = node;
    }
}


/*-- Vector, Matrix... --------------------------------------*/


/// Mise à jour ordinaire de la matrice modèle pour se placer dans le référentiel du parent.
void    node_updateModelMatrixWithParentModel(Node* const n, const Matrix4* const pm, float alpha) {
    Matrix4* m = &n->piu.model;
    Vector3 pos = node_pos(n);
    Vector2 scl = node_scales(n);
    m->v0.v = pm->v0.v * scl.x * alpha;
    m->v1.v = pm->v1.v * scl.y * alpha;
    m->v2 =   pm->v2;  // *scl.z ... si on veut un scaling en z...?
    m->v3 = (Vector4) {
        pm->v3.x + pm->v0.x * pos.x + pm->v1.x * pos.y + pm->v2.x * pos.z,
        pm->v3.y + pm->v0.y * pos.x + pm->v1.y * pos.y + pm->v2.y * pos.z,
        pm->v3.z + pm->v0.z * pos.x + pm->v1.z * pos.y + pm->v2.z * pos.z,
        pm->v3.w,
    };
}
/// Donne la position du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du noeud.
Vector2 node_posInParentReferential(Node* n, const Node* const parentOpt) {
    Squirrel sq;
    sq_init(&sq, n, sq_scale_ones);
    do {
        if(sq.pos->parent == parentOpt)
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
        if(sq.pos->parent == parentOpt)
            return (Box){ .center = sq.v, .deltas = sq.s };
    } while (sq_goUpP(&sq));
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
