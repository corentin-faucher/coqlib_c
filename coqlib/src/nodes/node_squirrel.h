//
//  node_squirrel.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-13.
//

#ifndef coq_node_squirrel_h
#define coq_node_squirrel_h
#include "node_base.h"
#include <stdio.h>

typedef struct Squirrel {
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

void sq_init(Squirrel *sq, Node *ref, enum SquirrelScaleType scale);
void sq_initWithRelPos(Squirrel *sq, Node *ref, Vector2 vRel,
                       enum SquirrelScaleType scale);
/// Le squirrel sq est dans la hitbox de sq.pos (noeud où il est).
bool sq_isIn(Squirrel *sq);

/// Aller à droite, i.e. au littleBro.
bool sq_goRight(Squirrel *sq);
/// Aller au littleBro, si absent, aller à l'ainé.
bool sq_goRightLoop(Squirrel *sq);
/// Utile ?
void sq_goRightForced(Squirrel *sq, Node *(*createNew)(void *), void *parOpt);
/// Aller au premier littleBro sans de flag "flag".
bool sq_goRightWithout(Squirrel *sq, flag_t flag);

bool sq_goLeft(Squirrel *sq);
bool sq_goLeftLoop(Squirrel *sq);
void sq_goLeftForced(Squirrel *sq, Node *(*createNew)(void *), void *parOpt);
bool sq_goLeftWithout(Squirrel *sq, flag_t flag);

bool sq_goDown(Squirrel *sq);
void sq_goDownForced(Squirrel *sq, Node *(*createNew)(void *), void *parOpt);
bool sq_goDownWithout(Squirrel *sq, flag_t flag);
bool sq_goDownLast(Squirrel *sq);
bool sq_goDownLastWithout(Squirrel *sq, flag_t flag);
bool sq_goDownP(Squirrel *sq);
bool sq_goDownPS(Squirrel *sq);

bool sq_goUp(Squirrel *sq);
bool sq_goUpP(Squirrel *sq);
bool sq_goUpPS(Squirrel *sq);

bool sq_goToNext(Squirrel *sq);
bool sq_renderer_goToNextToDisplay(Squirrel *sq);

bool sq_throwToGarbageThenGoToBroOrUp(Squirrel *sq);

Vector2 vector2_inReferentialOfSquirrel(Vector2 v, Squirrel *sq);

#endif /* node_squirrel_h */
