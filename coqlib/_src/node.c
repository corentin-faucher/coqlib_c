//
//  node.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include <stdlib.h>
#include "node.h"
#include "node_smooth.h"
#include "node_surface.h"
#include "node_flags.h"
#include "node_squirrel.h"
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
    Node* const parent = parentOpt;
    // Dans tout les cas, on a le parent:
    node->parent = parent;
    
    Node* oldParentFirstChild = parent->firstChild;
    Node* oldParentLastChild = parent->lastChild;
    // Cas parent pas d'enfants
    if(oldParentLastChild == NULL || oldParentFirstChild == NULL) {
        parent->firstChild = node;
        parent->lastChild = node;
        return;
    }
    // Ajout au début
    if(asElder) {
        // Insertion
        node->littleBro = oldParentFirstChild;
        // Branchement
        oldParentFirstChild->bigBro = node;
        parent->firstChild = node;
    } else { // Ajout a la fin.
        // Insertion
        node->bigBro = oldParentLastChild;
        // Branchement
        oldParentLastChild->littleBro = node;
        parent->lastChild = node;
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

/*-- Garbage --------------------------------*/
void _node_tree_burnDown(Node* const node) {
    if(node->firstChild == NULL) {
        free(node);
        return;
    }
#warning Verifier...
    Node* pos = node;
    Node* toDelete;
    while(true) {
        if(pos->firstChild) {
            pos = pos->firstChild;
            continue;
        }
        while(!pos->littleBro) {
            toDelete = pos;
            pos = pos->parent;
            free(toDelete);
            if(pos == NULL) {
                printerror("No root.");
                return;
            }
            if(pos == node) {
                free(pos);
                return;
            }
        }
        toDelete = pos;
        pos = pos->littleBro;
        free(toDelete);
    }
}

// Allocation d'un noeud de type quelconque.
Node* _Node_createEmptyOfType(const uint8_t type, const size_t size, const flag_t flags,
                              Node* const refOpt, const uint8_t node_place) {
    if(size < sizeof(Node)) {
        printerror("size < sizeof(Node).");
        return NULL;
    }
    Node* nd = calloc(1, size);
    nd->flags = flags;
    nd->_type = type;
    nd->_size = size;
    nd->piu = piu_default;
    nd->scales = vector2_ones;
    if(refOpt) node_simpleMoveTo(nd, refOpt, node_place);
    
    return nd;
}
Node* Node_createEmpty(void)
{
    return _Node_createEmptyOfType(node_type_bare, sizeof(Node), 0, NULL, 0);
}
// Constructeur usuel.
Node* Node_create(Node* const ref, const float x, const float y, const float w, const float h, const flag_t flags,
                  const uint8_t node_place) {
    Node *node = _Node_createEmptyOfType(node_type_bare, sizeof(Node), flags, ref, node_place);
    node->x = x;
    node->y = y;
    node->w = w;
    node->h = h;
    return node;
}
Node* Node_createCopy(Node* const other) {
//    if(other == NULL) { printerror("nothing to copy."); return NULL; }
    Node *nd = malloc(sizeof(other->_size));
    // Copie toute les donnees a priori.
    memcpy(nd, other, sizeof(other->_size));
    // Mais on remet les pointeurs sur null.
    nd->parent = NULL;
    nd->firstChild = NULL;
    nd->lastChild = NULL;
    nd->bigBro = NULL;
    nd->littleBro = NULL;
    return nd;
}

struct _Garbage {
    Node** container;
    uint   count;
    uint   index;
};
void _garbage_putNode(struct _Garbage* garbage, Node* toDelete) {
    if(garbage->index >= garbage->count) {
        garbage->count += 32;
        garbage->container = realloc(garbage->container, sizeof(Node*) * garbage->count);
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
void  node_destroy(Node *nodeToDelete) {
    nodeToDelete->flags |= _flag_deleted;
    _node_disconnect(nodeToDelete);
    if(nodeToDelete->deinit)
        nodeToDelete->deinit(nodeToDelete);
    if(_garbageA_active)
        _garbage_putNode(&_garbageA, nodeToDelete);
    else
        _garbage_putNode(&_garbageB, nodeToDelete);
}
void  node_garbage_burn(void) {
    if(_garbageA_active)
        _garbage_burnDown(&_garbageB);
    else
        _garbage_burnDown(&_garbageA);
    _garbageA_active = !_garbageA_active;
}

//Vector3 node_pos(Node* const node) {
//    return node->pos;
//}
//Vector2 node_wh(Node* const node) {
//    return node->wh;
//}
//Vector3 node_scales(Node* const node) {
//    return (Vector3){ node->sx, node->sy, 1.f };
//}
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
    if(node->_type & node_type_surface) {
        return smtrans_isActive(((NodeSurf*)node)->trShow);
    }
    return node->flags & (flag_show|flag_rootOfToDisplay);
}

void    node_setX(Node* const nd, float x, Bool fix){
    if(nd->_type & node_type_smooth)
        sp_set(&((NodeSmooth*)nd)->x, x, fix);
    nd->x = x;
}
void    node_setY(Node* const nd, float y, Bool fix) {
    if(nd->_type & node_type_smooth)
        sp_set(&((NodeSmooth*)nd)->y, y, fix);
    nd->y = y;
}
void    node_setScaleX(Node* const nd, float sx, Bool fix) {
    if(nd->_type & node_type_smooth)
        sp_set(&((NodeSmooth*)nd)->sx, sx, fix);
    nd->sx = sx;
}
void    node_setScaleY(Node* const nd, float sy, Bool fix) {
    if(nd->_type & node_type_smooth)
        sp_set(&((NodeSmooth*)nd)->sy, sy, fix);
    nd->sy = sy;
}

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

void   node_updateModelMatrixWithParentReferential(Node* const nd, Node* const parent) {
    Matrix4* ref = &parent->piu.model;
    Matrix4* m = &nd->piu.model;
    float sx, sy;
    Vector3 pos;
    if(nd->_type & node_type_smooth) {
        NodeSmooth* ns = (NodeSmooth*)nd;
        sx = sp_pos(&ns->sx);
        sy = sp_pos(&ns->sy);
        pos.x = sp_pos(&ns->x);
        pos.y = sp_pos(&ns->y);
        pos.z = nd->z;
    } else {
        sx = nd->sx;
        sy = nd->sy;
        pos = nd->pos;
    }
    m->v0 = ref->v0 * sx;
    m->v1 = ref->v1 * sy;
    m->v2 = ref->v2;
    m->v3 = (Vector4) {
        ref->v3.x + ref->v0.x * pos.x + ref->v1.x * pos.y + ref->v2.x * pos.z,
        ref->v3.y + ref->v0.y * pos.x + ref->v1.y * pos.y + ref->v2.y * pos.z,
        ref->v3.z + ref->v0.z * pos.x + ref->v1.z * pos.y + ref->v2.z * pos.z,
        ref->v3.w,
    };
}

Vector2 vector2_toVec2InReferentialOfNode(const Vector2 v, Node* const nodeOpt) {
    if(nodeOpt == NULL) return v;
    Squirrel sq;
    sq_init(&sq, nodeOpt, sq_scale_scales);
    while (sq_goUpPS(&sq));
    return vector2_inReferentialOfSquirrel(v, &sq);
}
