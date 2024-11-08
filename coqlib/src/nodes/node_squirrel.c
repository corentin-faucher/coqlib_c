//
//  node_squirrel.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-13.
//
#include "node_squirrel.h"
#include <stdio.h>

#include "../utils/util_base.h"
#include "node_drawable.h"

void sq_init(Squirrel *const sq, Node *const ref,
             enum SquirrelScaleType scale) {
  sq->pos = ref;
  sq->root = ref;
  sq->v = (Vector2){{ref->x, ref->y}};
  switch (scale) {
  case sq_scale_scales:
    sq->s = ref->scales;
    break;
  case sq_scale_deltas:
    sq->s = node_deltas(ref);
    break;
  default:
    sq->s = vector2_ones;
    break; // sq_scale_ones
  }
}
void sq_initWithRelPos(Squirrel *const sq, Node *const ref, const Vector2 vRel,
                       const enum SquirrelScaleType scale) {
  sq->pos = ref;
  sq->root = ref;
  sq->v = vRel;
  switch (scale) {
  case sq_scale_scales:
    sq->s = ref->scales;
    break;
  case sq_scale_deltas:
    sq->s = node_deltas(ref);
    break;
  default:
    sq->s = vector2_ones;
    break; // sq_scale_ones
  }
}
bool sq_isIn(Squirrel *sq) {
  return fabsf(sq->v.x - sq->pos->x) <= node_deltaX(sq->pos) &&
         fabsf(sq->v.y - sq->pos->y) <= node_deltaY(sq->pos);
}
bool sq_goRight(Squirrel *sq) {
  Node* const bro = sq->pos->_littleBro;
  if (bro) {
    sq->pos = bro;
    return true;
  }
  return false;
}
bool sq_goRightLoop(Squirrel *sq) {
  if (sq->pos->_littleBro != NULL) {
    sq->pos = sq->pos->_littleBro;
    return true;
  }
  if (sq->pos->_parent == NULL)
    return false;
  sq->pos = sq->pos->_parent->_firstChild;
  return true;
}
void sq_goRightForced(Squirrel *sq, Node *(*createNew)(void *), void *parOpt) {
  if (sq->pos->_littleBro != NULL) {
    sq->pos = sq->pos->_littleBro;
    return;
  }
  Node *newNode = createNew(parOpt);
  node_simpleMoveToBro(newNode, sq->pos, 0);
  sq->pos = newNode;
}
bool sq_goRightWithout(Squirrel *sq, flag_t flag) {
  do {
    if (!sq_goRight(sq))
      return false;
  } while (sq->pos->flags & flag);
  return true;
}
bool sq_goLeft(Squirrel *sq) {
  if (sq->pos->_bigBro == NULL)
    return false;
  sq->pos = sq->pos->_bigBro;
  return true;
}
bool sq_goLeftLoop(Squirrel *sq) {
  if (sq->pos->_bigBro != NULL) {
    sq->pos = sq->pos->_bigBro;
    return true;
  }
  if (sq->pos->_parent == NULL)
    return false;
  sq->pos = sq->pos->_parent->_lastChild;
  return true;
}
void sq_goLeftForced(Squirrel *sq, Node *(*createNew)(void *), void *parOpt) {
  if (sq->pos->_bigBro != NULL) {
    sq->pos = sq->pos->_bigBro;
    return;
  }
  Node *newNode = createNew(parOpt);
  node_simpleMoveToBro(newNode, sq->pos, 1);
  sq->pos = newNode;
}
bool sq_goLeftWithout(Squirrel *sq, flag_t flag) {
  do {
    if (!sq_goLeft(sq))
      return false;
  } while (sq->pos->flags & flag);
  return true;
}
bool sq_goDown(Squirrel *sq) {
  Node* const child = sq->pos->_firstChild;
  if(child) {
    sq->pos = child;
    return true;
  }
  return false;
}
void sq_goDownForced(Squirrel *sq, Node *(*createNew)(void *), void *parOpt) {
  if (sq->pos->_firstChild != NULL) {
    sq->pos = sq->pos->_firstChild;
    return;
  }
  Node *newNode = createNew(parOpt);
  node_simpleMoveToParent(newNode, sq->pos, 1);
  sq->pos = newNode;
}
bool sq_goDownWithout(Squirrel *sq, flag_t flag) {
  if (sq->pos->_firstChild == NULL)
    return false;
  sq->pos = sq->pos->_firstChild;
  while (sq->pos->flags & flag) {
    if (!sq_goRight(sq))
      return false;
  }
  return true;
}
bool sq_goDownLast(Squirrel *sq) {
  if (sq->pos->_lastChild == NULL)
    return false;
  sq->pos = sq->pos->_lastChild;
  return true;
}
bool sq_goDownLastWithout(Squirrel *sq, flag_t flag) {
  if (sq->pos->_lastChild == NULL)
    return false;
  sq->pos = sq->pos->_lastChild;
  while (sq->pos->flags & flag) {
    if (!sq_goLeft(sq))
      return false;
  }
  return true;
}
bool sq_goDownP(Squirrel *sq) {
  if (sq->pos->_firstChild == NULL)
    return false;
  sq->v.x = (sq->v.x - sq->pos->x) / sq->pos->sx;
  sq->v.y = (sq->v.y - sq->pos->y) / sq->pos->sy;
  sq->pos = sq->pos->_firstChild;
  return true;
}
bool sq_goDownPS(Squirrel *sq) {
  if (sq->pos->_firstChild == NULL)
    return false;
  sq->v.x = (sq->v.x - sq->pos->x) / sq->pos->sx;
  sq->v.y = (sq->v.y - sq->pos->y) / sq->pos->sy;
  sq->s.x /= sq->pos->sx;
  sq->s.y /= sq->pos->sy;
  sq->pos = sq->pos->_firstChild;
  return true;
}
bool sq_goUp(Squirrel *sq) {
  Node* const parent = sq->pos->_parent;
  if(parent) {
    sq->pos = parent;
    return true;
  }
  return false;
}
bool sq_goUpP(Squirrel *sq) {
  if (sq->pos->_parent == NULL)
    return false;
  sq->pos = sq->pos->_parent;
  sq->v.x = sq->v.x * sq->pos->sx + sq->pos->x;
  sq->v.y = sq->v.y * sq->pos->sy + sq->pos->y;
  return true;
}
bool sq_goUpPS(Squirrel *sq) {
  if (sq->pos->_parent == NULL)
    return false;
  sq->pos = sq->pos->_parent;
  sq->v.x = sq->v.x * sq->pos->sx + sq->pos->x;
  sq->v.y = sq->v.y * sq->pos->sy + sq->pos->y;
  sq->s.x *= sq->pos->sx;
  sq->s.y *= sq->pos->sy;
  return true;
}
bool sq_goToNext(Squirrel *sq) {
  if (sq_goDown(sq))
    return true;
  while (!sq_goRight(sq)) {
    if (!sq_goUp(sq)) {
      printerror("pas de root.");
      return false;
    }
    if (sq->pos == sq->root)
      return false;
  }
  return true;
}
bool sq_renderer_goToNextToDisplay(Squirrel *sq) {
  // 1. Aller en profondeur, pause si branche à afficher.
  if (sq->pos->_firstChild && 
      ((sq->pos->flags & flag_show ) || (sq->pos->_iu.render_flags & renderflag_toDraw)))
  {
    sq->pos->_iu.render_flags &= ~renderflag_toDraw;
    sq_goDown(sq);
    return true;
  }
  // 2. Redirection (voisin, parent).
  do {
    // Si le noeud présent est encore actif -> le parent doit l'être aussi.
    Node* const parent = sq->pos->_parent;
    if(parent) if((sq->pos->flags & flag_show ) || (sq->pos->_iu.render_flags & renderflag_toDraw)) {
        parent->_iu.render_flags |= renderflag_toDraw;
    }
    if (sq_goRight(sq))
      return true;
  } while (sq_goUp(sq));
  return false;
}
bool sq_throwToGarbageThenGoToBroOrUp(Squirrel *sq) {
  Node *toDelete = sq->pos;
  Node *bro = sq->pos->_littleBro;
  //    Node *bro = toLittle ? sq->pos->littleBro : sq->pos->bigBro;
  if (bro) {
    sq->pos = bro;
    node_throwToGarbage_(toDelete);
    return true;
  }
  if (sq->pos->_parent) {
    sq->pos = sq->pos->_parent;
    node_throwToGarbage_(toDelete);
    return false;
  }
  printerror("Ne peut deconnecter, nul part ou aller.");
  return false;
}

Vector2 vector2_inReferentialOfSquirrel(Vector2 v, Squirrel *sq) {
  return (Vector2){{(v.x - sq->v.x) / sq->s.x, (v.y - sq->v.y) / sq->s.y}};
}
