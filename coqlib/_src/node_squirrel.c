//
//  node_squirrel.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-13.
//

#include "node_squirrel.h"
#include "utils.h"

void sq_init(Squirrel* const sq, Node* const ref, enum SquirrelScaleType scale) {
    sq->pos = ref;
    sq->root = ref;
    sq->v = (Vector2) {ref->x, ref->y};
    switch(scale) {
        case sq_scale_scales: sq->s = ref->scales; break;
        case sq_scale_deltas: sq->s = node_deltas(ref); break;
        default:              sq->s = vector2_ones; break; // sq_scale_ones
    }
}
void sq_initWithRelPos(Squirrel* const sq, Node* const ref, const Vector2 vRel, const enum SquirrelScaleType scale) {
    sq->pos = ref;
    sq->root = ref;
    sq->v = vRel;
    switch(scale) {
        case sq_scale_scales: sq->s = ref->scales; break;
        case sq_scale_deltas: sq->s = node_deltas(ref); break;
        default:              sq->s = vector2_ones; break; // sq_scale_ones
    }
}
Bool sq_isIn(Squirrel *sq) {
    return fabsf(sq->v.x - sq->pos->pos.x) <= node_deltaX(sq->pos)
        && fabsf(sq->v.y - sq->pos->pos.y) <= node_deltaY(sq->pos);
}
Bool sq_goRight(Squirrel *sq) {
    if(sq->pos->littleBro == NULL) return false;
    sq->pos = sq->pos->littleBro;
    return true;
}
Bool sq_goRightLoop(Squirrel *sq) {
    if(sq->pos->littleBro != NULL) {
        sq->pos = sq->pos->littleBro;
        return true;
    }
    if(sq->pos->parent == NULL)
        return false;
    sq->pos = sq->pos->parent->firstChild;
    return true;
}
void sq_goRightForced(Squirrel *sq, Node *ref) {
    if(sq->pos->littleBro != NULL) {
        sq->pos = sq->pos->littleBro;
        return;
    }
    Node* newNode = Node_createCopy(ref);
    node_simpleMoveToBro(newNode, sq->pos, 0);
    sq->pos = newNode;
}
Bool sq_goRightWithout(Squirrel *sq, flag_t flag) {
    do {
        if(!sq_goRight(sq)) return false;
    } while(sq->pos->flags & flag);
    return true;
}
Bool sq_goLeft(Squirrel *sq) {
    if(sq->pos->bigBro == NULL) return false;
    sq->pos = sq->pos->bigBro;
    return true;
}
Bool sq_goLeftLoop(Squirrel *sq) {
    if(sq->pos->bigBro != NULL) {
        sq->pos = sq->pos->bigBro;
        return true;
    }
    if(sq->pos->parent == NULL)
        return false;
    sq->pos = sq->pos->parent->lastChild;
    return true;
}
void sq_goLeftForced(Squirrel *sq, Node *ref) {
    if(sq->pos->bigBro != NULL) {
        sq->pos = sq->pos->bigBro;
        return;
    }
    Node* newNode = Node_createCopy(ref);
    node_simpleMoveToBro(newNode, sq->pos, 1);
    sq->pos = newNode;
}
Bool sq_goLeftWithout(Squirrel *sq, flag_t flag) {
    do {
        if(!sq_goLeft(sq)) return false;
    } while(sq->pos->flags & flag);
    return true;
}
Bool sq_goDown(Squirrel *sq) {
    if(sq->pos->firstChild == NULL) return false;
    sq->pos = sq->pos->firstChild;
    return true;
}
void sq_goDownForced(Squirrel *sq, Node *ref) {
    if(sq->pos->firstChild != NULL) {
        sq->pos = sq->pos->firstChild;
        return;
    }
    Node* newNode = Node_createCopy(ref);
    node_simpleMoveToParent(newNode, sq->pos, 1);
    sq->pos = newNode;
}
Bool sq_goDownWithout(Squirrel *sq, flag_t flag) {
    if(sq->pos->firstChild == NULL) return false;
    sq->pos = sq->pos->firstChild;
    while(sq->pos->flags & flag) {
        if(!sq_goRight(sq)) return false;
    }
    return true;
}
Bool sq_goDownLast(Squirrel *sq) {
    if(sq->pos->lastChild == NULL) return false;
    sq->pos = sq->pos->lastChild;
    return true;
}
Bool sq_goDownLastWithout(Squirrel *sq, flag_t flag) {
    if(sq->pos->lastChild == NULL) return false;
    sq->pos = sq->pos->lastChild;
    while(sq->pos->flags & flag) {
        if(!sq_goLeft(sq)) return false;
    }
    return true;
}
Bool sq_goDownP(Squirrel *sq) {
    if(sq->pos->firstChild == NULL) return false;
    sq->v.x = (sq->v.x - sq->pos->x) / sq->pos->sx;
    sq->v.y = (sq->v.y - sq->pos->y) / sq->pos->sy;
    sq->pos = sq->pos->firstChild;
    return true;
}
Bool sq_goDownPS(Squirrel *sq) {
    if(sq->pos->firstChild == NULL) return false;
    sq->v.x = (sq->v.x - sq->pos->x) / sq->pos->sx;
    sq->v.y = (sq->v.y - sq->pos->y) / sq->pos->sy;
    sq->s.x /= sq->pos->sx;
    sq->s.y /= sq->pos->sy;
    sq->pos = sq->pos->firstChild;
    return true;
}
Bool sq_goUp(Squirrel *sq) {
    if(sq->pos->parent == NULL) return false;
    sq->pos = sq->pos->parent;
    return true;
}
Bool sq_goUpP(Squirrel *sq) {
    if(sq->pos->parent == NULL) return false;
    sq->pos = sq->pos->parent;
    sq->v.x = sq->v.x * sq->pos->sx + sq->pos->x;
    sq->v.y = sq->v.y * sq->pos->sy + sq->pos->y;
    return true;
}
Bool sq_goUpPS(Squirrel *sq) {
    if(sq->pos->parent == NULL) return false;
    sq->pos = sq->pos->parent;
    sq->v.x = sq->v.x * sq->pos->sx + sq->pos->x;
    sq->v.y = sq->v.y * sq->pos->sy + sq->pos->y;
    sq->s.x *= sq->pos->sx;
    sq->s.y *= sq->pos->sy;
    return true;
}
Bool sq_goToNext(Squirrel *sq) {
    if(sq_goDown(sq)) return true;
    while(!sq_goRight(sq)) {
        if(!sq_goUp(sq)) {
            printerror("pas de root.");
            return false;
        }
        if(sq->pos == sq->root) return false;
    }
    return true;
}
Bool sq_goToNextToDisplay(Squirrel *sq) {
    // 1. Aller en profondeur, pause si branche à afficher.
    if(sq->pos->firstChild && (sq->pos->flags & (flag_show|flag_rootOfToDisplay))) {
        sq->pos->flags &= ~flag_rootOfToDisplay;
        sq_goDown(sq);
        return true;
    }
    // 2. Redirection (voisin, parent).
    do {
        // Si le noeud présent est encore actif -> le parent doit l'être aussi.
        if(node_isDisplayActive(sq->pos) && sq->pos->parent) {
            sq->pos->parent->flags |= flag_rootOfToDisplay;
        }
        if(sq_goRight(sq)) return true;
    } while(sq_goUp(sq));
    return false;
}
Bool sq_disconnectAndGoToBroOrUp(Squirrel *sq, int toLittle) {
    Node *toDelete = sq->pos;
    Node *bro = toLittle ? sq->pos->littleBro : sq->pos->bigBro;
    if(bro) {
        sq->pos = bro;
        node_destroy(toDelete);
        return true;
    }
    if(sq->pos->parent) {
        sq->pos = sq->pos->parent;
        node_destroy(toDelete);
        return false;
    }
    printerror("ne peut deconnecter, nul part ou aller.");
    return false;
}

Vector2 vector2_inReferentialOfSquirrel(Vector2 v, Squirrel *sq) {
    return (Vector2) { (v.x - sq->v.x) / sq->s.x, (v.y - sq->v.y) / sq->s.y };
}
