//
//  node_tree.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//
#include "node_tree.h"

#include "../utils/util_base.h"

bool nodeptr_renderer_goToNextToDisplay(Node **const sq) {
    // 1. Aller en profondeur, pause si branche à afficher.
    Node *const child = (*sq)->_firstChild;
    if(child && (((*sq)->flags & flag_show ) || ((*sq)->renderIU.flags & renderflag_toDraw))) {
        (*sq)->renderIU.flags &= ~renderflag_toDraw;
        *sq = child;
        return true;
    }
    // 2. Redirection (voisin, parent).
    do {
        // Si le noeud présent est encore actif -> le parent doit l'être aussi.
        Node *const parent = (*sq)->_parent;
        if(parent && (((*sq)->flags & flag_show ) || ((*sq)->renderIU.flags & renderflag_toDraw))) {
            parent->renderIU.flags |= renderflag_toDraw;
        }
        // Allez à droite, sinon, en haut.
        Node *const littleBro = (*sq)->_littleBro;
        if(littleBro) {
            *sq = littleBro;
            return true;
        }
        if(!parent) return false;
        *sq = parent;
    } while (true);
}
void nodeptr_goRightForced(Node **const sq, Node *(*const createNew)(void *), void *const paramsOpt) {
    if (NULL != (*sq)->_littleBro) {
        *sq = (*sq)->_littleBro;
        return;
    }
    Node *const newNode = createNew(paramsOpt);
    node_simpleMoveToBro(newNode, (*sq), false);
    *sq = newNode;
}
bool nodeptr_goDownWithout(Node **const sq, flag_t flag) {
    if(NULL == (*sq)->_firstChild)
        return false;
    *sq = (*sq)->_firstChild;
    while ((*sq)->flags & flag) {
        if((*sq)->_littleBro) *sq = (*sq)->_littleBro;
        else return false;
    }
    return true;
}
bool nodeptr_goRightWithout(Node **const sq, flag_t flag) {
    do {
        if((*sq)->_littleBro) *sq = (*sq)->_littleBro;
        else return false;
    } while ((*sq)->flags & flag);
    return true;
}
bool nodeptr_throwToGarbageThenGoToBroOrUp(Node **const sq) {
  Node*const toDelete = *sq;
  Node*const bro = (*sq)->_littleBro;
  if (bro) {
    *sq = bro;
    node_throwToGarbage_(toDelete);
    return true;
  }
  if ((*sq)->_parent) {
    *sq = (*sq)->_parent;
    node_throwToGarbage_(toDelete);
    return false;
  }
  printerror("Ne peut deconnecter, nul part ou aller.");
  return false;
}

//#define DEBUG_SEARCH

void  node_tree_addFlags(Node* const nd, flag_t const flags) {
    nd->flags |= flags;
    // Version avec goto et juste un pointeur...
    Node* pos = nd->_firstChild;
    if(pos == NULL) return;
add_flag:
    pos->flags |= flags;
    if(pos->_firstChild) {
        pos = pos->_firstChild;
        goto add_flag;
    }
check_littleBro:
    if(pos->_littleBro) {
        pos = pos->_littleBro;
        goto add_flag;
    }
    pos = pos->_parent;
    if(pos == nd) return;
    if(pos == NULL) {
        printerror("Root not found.");
        return;
    }
    goto check_littleBro;
}
void  node_tree_removeFlags(Node *const nd, flag_t const flags) {
    nd->flags &= ~flags;
    Node* sq = nd->_firstChild;
    if(sq == NULL) return;
check:
    sq->flags &= ~flags;
    if(sq->_firstChild) {
        sq = sq->_firstChild;
        goto check;
    }
check_littleBro:
    if(sq->_littleBro) {
        sq = sq->_littleBro;
        goto check;
    }
    sq = sq->_parent;
    if(nd == sq) return;
    if(NULL == sq) { printerror("Root not found."); return; }
    goto check_littleBro;
}
void  node_tree_apply(Node *const nd, void (*const block)(Node*)) {
    block(nd);
    Node* pos = nd -> _firstChild;
    if(pos == NULL) return;
apply_block:
    block(pos);
    if(pos->_firstChild) {
        pos = pos->_firstChild;
        goto apply_block;
    }
check_littleBro:
    if(pos->_littleBro) {
        pos = pos->_littleBro;
        goto apply_block;
    }
    pos = pos->_parent;
    if(pos == nd) return;
    if(pos == NULL) { printerror("Root not found."); return; }
    goto check_littleBro;
}
void  node_tree_applyToTyped(Node* nd, void (*block)(Node*), uint32_t type_flag) {
    if(nd->_type & type_flag) block(nd);
    Node* sq = nd->_firstChild; if(!sq) return;
check:
    if(sq->_type & type_flag) block(sq);
    if(sq->_firstChild) {
        sq = sq->_firstChild;
        goto check;
    }
check_littleBro:
    if(sq->_littleBro) {
        sq = sq->_littleBro;
        goto check;
    }
    sq = sq->_parent;
    if(nd == sq) return;
    if(NULL == sq) { printerror("Root not found."); return; }
    goto check_littleBro;
}
//void  node_childs_applyToTyped...
void  node_tree_addRootFlags(Node *const nd, flag_t const flags) {
    Node* pos = nd->_parent;
    while(pos) {
        // (Il faut tout les flags, d'où le ==)
        if((pos->flags & flags) == flags) break;
        pos->flags |= flags;
        pos = pos->_parent;
    }
}
void  node_tree_openAndShow(Node* const nd) {
    if(nd->openOpt) nd->openOpt(nd);
    if(!(nd->flags & flag_hidden))
        nd->flags |= flag_show;
    if(!(nd->flags & flag_show)) return;
    // Version goto
    Node* pos = nd -> _firstChild;
    if(pos == NULL) return;
open_node:
    if(pos->openOpt) pos->openOpt(pos);
    if(!(pos->flags & flag_hidden))
        pos->flags |= flag_show;
    if(pos->flags & flag_show && pos->_firstChild) {
        pos = pos->_firstChild;
        goto open_node;
    }
check_littleBro:
    if(pos->_littleBro) {
        pos = pos->_littleBro;
        goto open_node;
    }
    pos = pos->_parent;
    if(pos == nd) return;
    if(pos == NULL) { printerror("Root not found."); return; }
    goto check_littleBro;
}
void  node_tree_unhideAndTryToOpen(Node *const nd) {
    nd->flags &= ~flag_hidden;
    if(!nd->_parent) return;
    if(!(nd->_parent->flags & flag_show)) return;
    node_tree_openAndShow(nd);
}
void  node_tree_close(Node *const nd) {
    if(!(nd->flags & flag_exposed))
        nd->flags &= ~flag_show;
    if(nd->closeOpt) nd->closeOpt(nd);
    Node *sq = nd->_firstChild; if(!sq) return;
check:
    if(!(sq->flags & flag_exposed))
        sq->flags &= ~flag_show;
    if(sq->closeOpt) sq->closeOpt(sq);
        
    if(sq->_firstChild) {
        sq = sq->_firstChild;
        goto check;
    }
check_littleBro:
    if(sq->_littleBro) {
        sq = sq->_littleBro;
        goto check;
    }
    sq = sq->_parent;
    if(sq == nd) return;
    if(sq == NULL) { printerror("Root not found."); return; }
    goto check_littleBro;
}
void  node_tree_hideAndTryToClose(Node* nd) {
    nd->flags |= flag_hidden;
    if(nd->flags & flag_show)
        node_tree_close(nd);
}
void  node_tree_reshape(Node*const nd) {
    if(!(nd->flags & flag_show)) return;
    if(nd->reshapeOpt) nd->reshapeOpt(nd);
    Node *sq = nd->_firstChild; if(!sq) return;
check:
    if(sq->flags & flag_show) {
        if(sq->reshapeOpt) sq->reshapeOpt(sq);
        if((sq->flags & flag_parentOfReshapable) && sq->_firstChild) {
            sq = sq->_firstChild;
            goto check;
        }
    }
check_littleBro:
    if(sq->_littleBro) {
        sq = sq->_littleBro;
        goto check;
    }
    sq = sq->_parent;
    if(sq == nd) return;
    if(sq == NULL) { printerror("Root not found."); return; }
    goto check_littleBro;
}
NodeTouch node_tree_searchActiveButtonWithPosOpt(NodeTouch const nt, Node const*const nodeToAvoidOpt) {
    if(!nt.n) { printerror("No node to search in."); return (NodeTouch) {}; }
    Vector2 posInPar;
    if(nt.n->_parent) {
        Box parRef = node_referentialInParent(nt.n->_parent, NULL);
        posInPar = vector2_referentialIn(nt.posAbs, parRef);
    } else {
        posInPar = nt.posAbs;
    }
    // Premier check avant d'aller au descendents. (noeud fourni)
    if((nt.n == nodeToAvoidOpt) || !vector2_isInBox(posInPar, node_hitbox(nt.n)) || !(nt.n->flags & flag_show)) 
        return (NodeTouch) {};
    if_let(Button*, button, node_asActiveButtonOpt(nt.n))
    return (NodeTouch) {
        .n = nt.n,
        .posAbs = nt.posAbs,
        .touchId = nt.touchId,
        ._posRelOpt = vector2_referentialIn(posInPar, node_referential(nt.n)),
        ._posRelDefined = true,
    }; 
    if_let_end
    posInPar = vector2_referentialIn(posInPar, node_referential(nt.n));
    Node* sq = nt.n->_firstChild;
    if(sq == NULL) return (NodeTouch) {};
check_node:
    if((sq->flags & flag_show) && (sq != nodeToAvoidOpt) && vector2_isInBox(posInPar, node_hitbox(sq))) {
        if(node_asActiveButtonOpt(sq)) {
            return (NodeTouch) {
                .n = sq,
                .posAbs = nt.posAbs,
                .touchId = nt.touchId,
                ._posRelOpt = vector2_referentialIn(posInPar, node_referential(sq)),
                ._posRelDefined = true,
            };
        }
        Node*const child = sq->_firstChild;
        if((sq->flags & flag_parentOfButton) && child) {
            posInPar = vector2_referentialIn(posInPar, node_referential(sq));
            sq = child;
            goto check_node;
        }
    }
check_littleBro:
    if_let(Node*, bro, sq->_littleBro)
        sq = bro;
        goto check_node;
    if_let_end
    // Remonte
    sq = sq->_parent;
    if(sq == nt.n) return (NodeTouch) {};
    if(sq == NULL) { printerror("Root not found."); return (NodeTouch) {}; }
    posInPar = vector2_referentialOut(posInPar, node_referential(sq));
    goto check_littleBro;
}
Node* node_tree_searchFirstOfTypeWithDataOpt(Node* const n, uint32_t const type, uint32_t const data0) 
{
    if(!type || (type & node_type_drawable)) {
        printerror("Bad type for search with data."); return NULL;
    }
    if((n->_type & type) && (n->nodrawData.data0.u0 == data0)) return n;
    Node* sq = n->_firstChild;
    if(!sq) return NULL;
check:
    if((sq->_type & type) && (n->nodrawData.data0.u0 == data0)) return sq;
    if(sq->_firstChild) {
        sq = sq->_firstChild;
        goto check;
    }
check_littleBro:
    if(sq->_littleBro) {
        sq = sq->_littleBro;
        goto check;
    }
    sq = sq->_parent;
    if(sq == n) return NULL;
    if(sq == NULL) { printerror("Root not found."); return NULL; }
    goto check_littleBro;
}
Node*   node_tree_searchFirstOfTypeInBranchOpt(Node* const n, uint32_t const type_flag, flag_t parentFlag) {
    if(!type_flag || !parentFlag) { printerror("No parent branch flag or no type flag."); return NULL; }
    // Cas trivial...
    if(!(n->flags & flag_show)) return NULL;
    if(n->_type & type_flag) return n;
    if(!(n->flags & parentFlag)) return NULL;
    Node *sq = n->_firstChild;
    if(!sq) return NULL;
check:
    if(sq->flags & flag_show) {
        if(sq->_type & type_flag) return sq;
        if((sq->flags & parentFlag) && sq->_firstChild) {
            sq = sq->_firstChild;
            goto check;
        }
    }
check_littleBro:
    if(sq->_littleBro) {
        sq = sq->_littleBro;
        goto check;
    }
    sq = sq->_parent;
    if(sq == n) return NULL;
    if(sq == NULL) { printerror("Root not found."); return NULL; }
    goto check_littleBro;
}

int  node_tree_alignTheChildren(Node *const nd, node_align_option const alignOpt, float const ratio, float const spacingRef) {
    Node *sq = nd;
    if(!nodeptr_goDownWithout(&sq, flag_hidden|flag_notToAlign))
        return 0;
    bool fix = ((alignOpt & node_align_fixPos) != 0);
    bool horizontal = ((alignOpt & node_align_vertically) == 0);
    bool setSecondaryToDefPos = ((alignOpt & node_align_setSecondaryToDefPos) != 0);
    bool dontSetPrimaryDefPos = ((alignOpt & node_align_dontSetPrimaryDefPos) != 0);
    bool updateSecondarySize = (alignOpt & node_align_dontUpdateSecondarySize) == 0;
    bool updatePrimarySize = (alignOpt & node_align_dontUpdateSecondarySize) == 0;
    // 1. Mesurer largeur et hauteur requises
    float w = 0;
    float h = 0;
    int n = 0;
    if(horizontal) {
        do {
            w += 2.f*spacingRef*node_deltaX(sq);
            n ++;
            float htmp = 2.f*node_deltaY(sq);
            Fluid* fluid = node_asFluidOpt(sq);
            if(fluid && setSecondaryToDefPos)
                htmp += 2*fabsf(fluid->y.def);
            if(htmp > h) h = htmp;
        } while(nodeptr_goRightWithout(&sq, flag_hidden|flag_notToAlign));
    } else {
        do {
            h += 2.f*spacingRef*node_deltaY(sq);
            n ++;
            float wtmp = 2.f*node_deltaX(sq);
            Fluid* fluid = node_asFluidOpt(sq);
            if(fluid && setSecondaryToDefPos)
                wtmp += 2*fabsf(fluid->x.def);
            if(wtmp > w) w = wtmp;
        } while(nodeptr_goRightWithout(&sq, flag_hidden|flag_notToAlign));
    }
    // 2. Calculer l'espacement requis et ajuster w/h en fonction du ratio voulu.
    float extraHalfSpace = 0;
    if ((alignOpt & node_align_respectRatio) != 0) {
        if(horizontal) {
            if  (w/h < ratio) {
                extraHalfSpace = 0.5f * (ratio * h - w) / (float)n;
                w = ratio * h;
            }
        } else {
            if (w/h > ratio) {
                extraHalfSpace = 0.5f * (w/ratio - h) / (float)n;
                h = w / ratio;
            }
        }
    }
    // 3. Mettre à jour largeur et hauteur du noeud parent.
    if(updatePrimarySize) {
        if(horizontal) nd->w = w;
        else nd->h = h;
    }
    if(updateSecondarySize) {
        if(horizontal) nd->h = h;
        else nd->w = w;
    }
    if(nd->flags & flag_giveSizeToBigbroFrame) if(updatePrimarySize || updateSecondarySize) {
        node_tryUpdatingAsFrameOfBro(nd->_bigBro, nd);
    }
    // 4. Aligner les éléments
    sq = nd;
    if(!nodeptr_goDownWithout(&sq, flag_hidden|flag_notToAlign))
        return 0;
    if(horizontal) {
        float x = -w / 2.f;
        uint32_t const relativeFlag = (alignOpt & node_align_rightBottom) ? relatives_bottom : 
                                 ((alignOpt & node_align_leftTop) ? relatives_top : 0);
        do {
            x += spacingRef * node_deltaX(sq) + extraHalfSpace;
            Fluid* fluid = node_asFluidOpt(sq);
            if(fluid) {
                if(!dontSetPrimaryDefPos) fluid->x.def = x;
                if(fix) fluid_fixX(fluid, x); else fluid_setX(fluid, x);
            } else {
                sq->x = x;
            }
            if(setSecondaryToDefPos && fluid)
                fl_setRelToDef(&fluid->y, 0);
            else
                node_setYrelatively(sq, relativeFlag, fix);
            x += spacingRef * node_deltaX(sq) + extraHalfSpace;
        } while(nodeptr_goRightWithout(&sq, flag_hidden|flag_notToAlign));
        return n;
    }
    // Aligner verticalement
    float y = +0.5f*h; // (de haut en bas)
    uint32_t const relativeFlag = (alignOpt & node_align_rightBottom) ? relatives_right : 
                                 ((alignOpt & node_align_leftTop) ? relatives_left : 0);
    do {
        y -= spacingRef * node_deltaY(sq) + extraHalfSpace;
        Fluid* fluid = node_asFluidOpt(sq);
        if(fluid) {
            if(!dontSetPrimaryDefPos) fluid->y.def = y;
            if(fix) fluid_fixY(fluid, y);
            else    fluid_setY(fluid, y);
        } else {
            sq->y = y;
        }
        if(setSecondaryToDefPos && fluid)
            fl_setRelToDef(&fluid->x, 0);
        else
            node_setXrelatively(sq, relativeFlag, fix);
        y -= spacingRef * node_deltaY(sq) + extraHalfSpace;
    } while(nodeptr_goRightWithout(&sq, flag_hidden|flag_notToAlign));
    return n;
}

// Garbage
/*
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
  sq->b.c_x = (sq->b.c_x - sq->pos->x) / sq->pos->sx;
  sq->b.c_y = (sq->b.c_y - sq->pos->y) / sq->pos->sy;
  sq->pos = sq->pos->_firstChild;
  return true;
}
bool sq_goDownPS(Squirrel *sq) {
  if (sq->pos->_firstChild == NULL)
    return false;
  sq->b.c_x = (sq->b.c_x - sq->pos->x) / sq->pos->sx;
  sq->b.c_y = (sq->b.c_y - sq->pos->y) / sq->pos->sy;
  sq->b.Dx /= sq->pos->sx;
  sq->b.Dy /= sq->pos->sy;
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
  sq->b.c_x = sq->b.c_x * sq->pos->sx + sq->pos->x;
  sq->b.c_y = sq->b.c_y * sq->pos->sy + sq->pos->y;
  return true;
}
bool sq_goUpPS(Squirrel *sq) {
  if (sq->pos->_parent == NULL)
    return false;
  sq->pos = sq->pos->_parent;
  sq->b.c_x = sq->b.c_x * sq->pos->sx + sq->pos->x;
  sq->b.c_y = sq->b.c_y * sq->pos->sy + sq->pos->y;
  sq->b.Dx *= sq->pos->sx;
  sq->b.Dy *= sq->pos->sy;
  return true;
}
bool sq_renderer_goToNextToDisplay(Squirrel *const sq) {
    // 1. Aller en profondeur, pause si branche à afficher.
    Node *const child = sq->pos->_firstChild;
    if(child && ((sq->pos->flags & flag_show ) || (sq->pos->renderIU.flags & renderflag_toDraw))) {
        sq->pos->renderIU.flags &= ~renderflag_toDraw;
        sq->pos = child;
        return true;
    }
    // 2. Redirection (voisin, parent).
    do {
        // Si le noeud présent est encore actif -> le parent doit l'être aussi.
        Node *const parent = sq->pos->_parent;
        if(parent && ((sq->pos->flags & flag_show ) || (sq->pos->renderIU.flags & renderflag_toDraw))) {
            parent->renderIU.flags |= renderflag_toDraw;
        }
        // Allez à droite, sinon, en haut.
        Node *const littleBro = sq->pos->_littleBro;
        if(littleBro) {
            sq->pos = littleBro;
            return true;
        }
        if(!parent) return false;
        sq->pos = parent;
    } while (true);
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
*/
//ButtonPosRel node_tree_searchActiveButtonWithPosOpt(Node* const n, Vector2 const pos, Node* const nodeToAvoidOpt) {
//    if(n == nodeToAvoidOpt) return (ButtonPosRel) {};
////    vector2_absPosToPosInReferentialOfNode(absPos, root);
//    Squirrel sq;
//    sq_initWithRelPos(&sq, n, (Box){ .center = pos });
//    if(!sq_isIn(&sq)) return NULL;
//    if(!(sq.pos->flags & flag_show)) return NULL;
//    Button* b = NULL;
//    // 2. Se placer au premier child
//    if(!sq_goDownP(&sq))
//        return NULL;
//    while (true) {
//#ifdef DEBUG_SEARCH
//        printf("🐰 sq at %f, %f in ", sq.v.x, sq.v.y);
//        node_print_type(sq.pos);
//        printf(" pos %f, %f, in %d, show %lu, ", sq.pos->pos.x, sq.pos->pos.y, sq_isIn(&sq), (sq.pos->flags & flag_show));
//#endif
//        if(sq_isIn(&sq) && (sq.pos->flags & flag_show)
//           && (sq.pos != nodeToAvoidOpt))
//        {
//#ifdef DEBUG_SEARCH
//            printf("In...");
//#endif
//            // 1. Possibilité trouvé
//            b = node_asActiveButtonOpt(sq.pos);
//            if(b) {
//#ifdef DEBUG_SEARCH
//                printf("button !\n");
//#endif
//                return b;
//            }
//            // 2. Aller en profondeur
//            if(sq.pos->flags & flag_parentOfButton) {
//                if(sq_goDownP(&sq)) {
//#ifdef DEBUG_SEARCH
//                    printf(" Go Down!\n");
//#endif
//                    continue;
//                }
//            }
//        }
//        // 3. Remonter, si plus de petit-frère
//#ifdef DEBUG_SEARCH
//        printf(" out, next...");
//#endif
//        while(!sq_goRight(&sq)) {
//#ifdef DEBUG_SEARCH
//            printf(" up ");
//#endif
//            if(!sq_goUpP(&sq)) {
//                printerror("Pas de root."); return NULL;
//            } else if(sq.pos == n) {
//#ifdef DEBUG_SEARCH
//                printf("\n");
//#endif
//                return NULL;
//            }
//        }
//#ifdef DEBUG_SEARCH
//        printf("\n");
//#endif
//    }
//}
