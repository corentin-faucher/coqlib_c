//
//  node_squirrel.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-13.
//

#ifndef node_squirrel_h
#define node_squirrel_h

#include <stdio.h>
#include "_node.h"

typedef struct _Squirrel {
    Node *pos;
    Vector2 v;
    Vector2 s;
    Node *root; // Utile ?
} Squirrel;

enum SquirrelScaleType {
    sq_scale_ones,
    sq_scale_scales,
    sq_scale_deltas,
};

void sq_init(Squirrel *sq, Node* ref, enum SquirrelScaleType scale);
void sq_initWithRelPos(Squirrel *sq, Node* ref, Vector2 vRel, enum SquirrelScaleType scale);
/// Le squirrel sq est dans la hitbox de sq.pos (noeud où il est).
Bool sq_isIn(Squirrel *sq);

/// Aller à droite, i.e. au littleBro.
Bool sq_goRight(Squirrel *sq);
/// Aller au littleBro, si absent, aller à l'ainé.
Bool sq_goRightLoop(Squirrel *sq);
/// S'il n'y a pas de littleBro, on le crée en clonant ref.
/// Si copyNodeOpt est absent (NULL) on utilise la method par defaut Node_default_createCopy.
void sq_goRightForced(Squirrel *sq, Node *ref, Node* (*copyNodeOpt)(const Node*));
/// Aller au premier littleBro sans de flag "flag".
Bool sq_goRightWithout(Squirrel *sq, flag_t flag);

Bool sq_goLeft(Squirrel *sq);
Bool sq_goLeftLoop(Squirrel *sq);
void sq_goLeftForced(Squirrel *sq, Node *ref, Node* (*copyNodeOpt)(const Node*));
Bool sq_goLeftWithout(Squirrel *sq, flag_t flag);

Bool sq_goDown(Squirrel *sq);
void sq_goDownForced(Squirrel *sq, Node *ref, Node* (*copyNodeOpt)(const Node*));
Bool sq_goDownWithout(Squirrel *sq, flag_t flag);
Bool sq_goDownLast(Squirrel *sq);
Bool sq_goDownLastWithout(Squirrel *sq, flag_t flag);
Bool sq_goDownP(Squirrel *sq);
Bool sq_goDownPS(Squirrel *sq);

Bool sq_goUp(Squirrel *sq);
Bool sq_goUpP(Squirrel *sq);
Bool sq_goUpPS(Squirrel *sq);

Bool sq_goToNext(Squirrel *sq);
Bool sq_goToNextToDisplay(Squirrel *sq);

Bool sq_throwToGarbageThenGoToBroOrUp(Squirrel *sq, int toLittle);

Vector2 vector2_inReferentialOfSquirrel(Vector2 v, Squirrel *sq);

#endif /* node_squirrel_h */
