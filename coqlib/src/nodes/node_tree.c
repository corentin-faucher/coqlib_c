//
//  node_tree.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//
#include "node_tree.h"

#include "node_squirrel.h"
#include "../utils/util_base.h"


//#define DEBUG_SEARCH

void  node_tree_addFlags(Node* const nd, flag_t flags) {
    nd->flags |= flags;
    // Version avec goto et juste un pointeur...
    Node* pos = nd -> _firstChild;
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
    // Version avec while et Squirrel...
//    Node* firstChild = nd->firstChild;
//    if(firstChild == NULL) return;
//    Squirrel sq;
//    sq_init(&sq, firstChild, sq_scale_ones);
//    while(true) {
//        sq.pos->flags |= flags;
//        if(sq_goDown(&sq))
//            continue;
//        while(!sq_goRight(&sq)) {
//            if(!sq_goUp(&sq)) {
//                printerror("Pas de root."); return;
//            } else if(sq.pos == nd) return;
//        }
//    }
}
void  node_tree_removeFlags(Node* nd, flag_t flags) {
    if(nd == NULL) return;
    nd->flags &= ~flags;
    Node* firstChild = nd->_firstChild;
    if(firstChild == NULL) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while(true) {
        sq.pos->flags &= ~flags;
        if(sq_goDown(&sq))
            continue;
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return;
            } else if(sq.pos == nd) return;
        }
    }
}
void  node_tree_apply(Node* nd, void (*block)(Node*)) {
    if(nd == NULL) return;
    block(nd);
    // Version goto
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
    if(pos == NULL) {
        printerror("Root not found.");
        return;
    }
    goto check_littleBro;
//    Node* firstChild = nd->firstChild;
//    if(firstChild == NULL) return;
//    Squirrel sq;
//    sq_init(&sq, firstChild, sq_scale_ones);
//    while(true) {
//        block(sq.pos);
//        if(sq_goDown(&sq))
//            continue;
//        while(!sq_goRight(&sq)) {
//            if(!sq_goUp(&sq)) {
//                printerror("Pas de root."); return;
//            } else if(sq.pos == nd) return;
//        }
//    }
}
void  node_tree_applyToTyped(Node* nd, void (*block)(Node*), uint32_t type_flag) {
    if(nd == NULL) return;
    if(nd->_type & type_flag)
        block(nd);
    Node* firstChild = nd->_firstChild;
    if(firstChild == NULL) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while(true) {
        
        if(sq.pos->_type & type_flag)
            block(sq.pos);
        if(sq_goDown(&sq))
            continue;
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return;
            } else if(sq.pos == nd) return;
        }
    }
}
//void  node_childs_applyToTyped...
void  node_tree_addRootFlags(Node* nd, flag_t flags) {
    Node* pos = nd->_parent;
    while(pos) {
        // (Il faut tout les flags, d'où le ==)
        if((pos->flags & flags) == flags) break;
        pos->flags |= flags;
        pos = pos->_parent;
    }
}
// Superflu...
//void  node_tree_makeSelectable(Node* nd) {
//    node_tree_addRootFlag(nd, flag_selectableRoot);
//
//}
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
    if(pos == NULL) {
        printerror("Root not found.");
        return;
    }
    goto check_littleBro;
//    Node* firstChild = nd->firstChild;
//    if(firstChild == NULL) return;
//    Squirrel sq;
//    sq_init(&sq, firstChild, sq_scale_ones);
//    while (true) {
//        if(sq.pos->open) sq.pos->open(sq.pos);
//        if(!(sq.pos->flags & flag_hidden))
//            sq.pos->flags |= flag_show;
//        if(sq.pos->flags & flag_show)
//            if(sq_goDown(&sq))
//                continue;
//        while (!sq_goRight(&sq)) {
//            if (!sq_goUp(&sq)) {
//                printerror("Pas de branch."); return;
//            } else if (sq.pos == nd) return;
//        }
//    }
}
void  node_tree_unhideAndTryToOpen(Node* nd) {
    nd->flags &= ~flag_hidden;
    if(!nd->_parent) return;
    if(!(nd->_parent->flags & flag_show)) return;
    node_tree_openAndShow(nd);
}
void  node_tree_close(Node* nd) {
    if(!(nd->flags & flag_exposed))
        nd->flags &= ~flag_show;
    if(nd->closeOpt) nd->closeOpt(nd);
    Node* firstChild = nd->_firstChild;
    if(firstChild == NULL) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while (true) {
        if(!(sq.pos->flags & flag_exposed))
            sq.pos->flags &= ~flag_show;
        if(sq.pos->closeOpt) sq.pos->closeOpt(sq.pos);
        
        if(sq_goDown(&sq))
            continue;
        while (!sq_goRight(&sq)) {
            if (!sq_goUp(&sq)) {
                printerror("Pas de branch."); return;
            } else if (sq.pos == nd) return;
        }
    }
}
void  node_tree_hideAndTryToClose(Node* nd) {
    nd->flags |= flag_hidden;
    if(nd->flags & flag_show)
        node_tree_close(nd);
}
void  node_tree_reshape(Node* nd) {
    if(!(nd->flags & flag_show)) return;
    if(nd->reshapeOpt) nd->reshapeOpt(nd);
    if(!(nd->flags & flag_parentOfReshapable)) return;
    Node *firstChild = nd->_firstChild;
    if(!firstChild) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while (true) {
        if(sq.pos->flags & flag_show) {
            if(sq.pos->reshapeOpt) sq.pos->reshapeOpt(sq.pos);
            if(sq.pos->flags & flag_parentOfReshapable)
                if(sq_goDown(&sq))
                    continue;
        }
        while (!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pes de branch."); return;
            } else if (sq.pos == nd) return;
        }
    }
}

Button* node_tree_searchActiveButtonWithPosOpt(Node* const n, Vector2 const pos, Node* const nodeToAvoidOpt) {
    if(n == nodeToAvoidOpt) return NULL;
//    vector2_absPosToPosInReferentialOfNode(absPos, root);
    Squirrel sq;
    sq_initWithRelPos(&sq, n, pos, sq_scale_ones);
    if(!sq_isIn(&sq)) return NULL;
    if(!(sq.pos->flags & flag_show)) return NULL;
    Button* b = NULL;
    // 2. Se placer au premier child
    if(!sq_goDownP(&sq))
        return NULL;
    while (true) {
#ifdef DEBUG_SEARCH
        printf("🐰 sq at %f, %f in ", sq.v.x, sq.v.y);
        node_print_type(sq.pos);
        printf(" pos %f, %f, in %d, show %lu, ", sq.pos->pos.x, sq.pos->pos.y, sq_isIn(&sq), (sq.pos->flags & flag_show));
#endif
        if(sq_isIn(&sq) && (sq.pos->flags & flag_show)
           && (sq.pos != nodeToAvoidOpt))
        {
#ifdef DEBUG_SEARCH
            printf("In...");
#endif
            // 1. Possibilité trouvé
            b = node_asActiveButtonOpt(sq.pos);
            if(b) {
#ifdef DEBUG_SEARCH
                printf("button !\n");
#endif
                return b;
            }
            // 2. Aller en profondeur
            if(sq.pos->flags & flag_parentOfButton) {
                if(sq_goDownP(&sq)) {
#ifdef DEBUG_SEARCH
                    printf(" Go Down!\n");
#endif
                    continue;
                }
            }
        }
        // 3. Remonter, si plus de petit-frère
#ifdef DEBUG_SEARCH
        printf(" out, next...");
#endif
        while(!sq_goRight(&sq)) {
#ifdef DEBUG_SEARCH
            printf(" up ");
#endif
            if(!sq_goUpP(&sq)) {
                printerror("Pas de root."); return NULL;
            } else if(sq.pos == n) {
#ifdef DEBUG_SEARCH
                printf("\n");
#endif
                return NULL;
            }
        }
#ifdef DEBUG_SEARCH
        printf("\n");
#endif
    }
}
Button* node_tree_searchFirstButtonWithDataOpt(Node* const n, uint32_t const typeOpt, uint32_t const data0) {
    Squirrel sq;
    sq_init(&sq, n, sq_scale_ones);
    // Se placer au premier child
    if(!sq_goDown(&sq))
        return NULL;
    while(true) {
        if(sq.pos->flags & flag_show) {
            Button* b = node_asButtonOpt(sq.pos);
            if(b) if(b->data.uint0 == data0) {
                if(!typeOpt) return b;
                if(b->n._type & typeOpt)
                    return b;
            }
            if(sq.pos->flags & flag_parentOfButton)
                if(sq_goDown(&sq)) continue;
        }
        // 3. Remonter, si plus de petit-frère
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return NULL;
            } else if(sq.pos == n) return NULL;
        }
    }
}
Node*   node_tree_searchFirstOfTypeInBranchOpt(Node* const n, uint32_t const type_flag, flag_t parentFlag) {
    if(!type_flag || !parentFlag) { printerror("No parent branch flag or no type flag."); return NULL; }
    // Cas trivial...
    if(n->_type & type_flag) return n;
    if(!(n->flags & parentFlag)) return NULL;
    Squirrel sq;
    sq_init(&sq, n, sq_scale_ones);
    // Se placer au premier child
    if(!sq_goDown(&sq))
        return NULL;
    while(true) {
        if(sq.pos->flags & flag_show) {
            if(sq.pos->_type & type_flag)
                return sq.pos;
            if(sq.pos->flags & parentFlag)
                if(sq_goDown(&sq)) continue;
        }
        // 3. Remonter, si plus de petit-frère
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return NULL;
            } else if(sq.pos == n) return NULL;
        }
    }
}








int  node_tree_alignTheChildren(Node* nd, node_align_option alignOpt, float ratio, float spacingRef) {
    Squirrel sq;
    sq_init(&sq, nd, sq_scale_ones);
    if(!sq_goDownWithout(&sq, flag_hidden|flag_notToAlign))
        return 0;
    bool fix = ((alignOpt & node_align_fixPos) != 0);
    bool horizontal = ((alignOpt & node_align_vertically) == 0);
    bool setSecondaryToDefPos = ((alignOpt & node_align_setSecondaryToDefPos) != 0);
    bool dontSetPrimaryDefPos = ((alignOpt & node_align_dontSetPrimaryDefPos) != 0);
    // 1. Mesurer largeur et hauteur requises
    float w = 0;
    float h = 0;
    int n = 0;
    if(horizontal) {
        do {
            w += 2.f*spacingRef*node_deltaX(sq.pos);
            n ++;
            float htmp = 2.f*node_deltaY(sq.pos);
            Fluid* fluid = node_asFluidOpt(sq.pos);
            if(fluid && setSecondaryToDefPos)
                htmp += 2*fabsf(fluid->y.def);
            if(htmp > h) h = htmp;
        } while(sq_goRightWithout(&sq, flag_hidden|flag_notToAlign));
    } else {
        do {
            h += 2.f*spacingRef*node_deltaY(sq.pos);
            n ++;
            float wtmp = 2.f*node_deltaX(sq.pos);
            Fluid* fluid = node_asFluidOpt(sq.pos);
            if(fluid && setSecondaryToDefPos)
                wtmp += 2*fabsf(fluid->x.def);
            if(wtmp > w) w = wtmp;
        } while(sq_goRightWithout(&sq, flag_hidden|flag_notToAlign));
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
    if((alignOpt & node_align_dontUpdateSizes) == 0) {
        nd->w = w;
        nd->h = h;
        if(nd->flags & flag_giveSizeToBigbroFrame)
            node_tryUpdatingAsFrameOfBro(nd->_bigBro, nd);
    }
    // 4. Aligner les éléments
    sq_init(&sq, nd, sq_scale_ones);
    if(!sq_goDownWithout(&sq, flag_hidden|flag_notToAlign))
        return 0;
    if(horizontal) {
        float x = -w / 2.f;
        do {
            x += spacingRef * node_deltaX(sq.pos) + extraHalfSpace;
            Fluid* fluid = node_asFluidOpt(sq.pos);
            if(fluid) {
                if(!dontSetPrimaryDefPos) fluid->x.def = x;
                fluid_setX(fluid, x, fix);
                if(setSecondaryToDefPos) {
                    fluid_setYrelToDef(fluid, 0.f, fix);
                } else 
                    fluid_setY(fluid, 0.f, fix);
            } else {
                sq.pos->x = x;
                sq.pos->y = 0.f;
            }
            x += spacingRef * node_deltaX(sq.pos) + extraHalfSpace;
        } while(sq_goRightWithout(&sq, flag_hidden|flag_notToAlign));
        return n;
    }
    // Aligner verticalement
    float y = +h / 2.f; // (de haut en bas)
    do {
        y -= spacingRef * node_deltaY(sq.pos) + extraHalfSpace;
        Fluid* fluid = node_asFluidOpt(sq.pos);
        if(fluid) {
            if(!dontSetPrimaryDefPos) fluid->y.def = y;
            fluid_setY(fluid, y, fix);
            if(setSecondaryToDefPos) {
                fluid_setXrelToDef(fluid, 0.f, fix);
            } else fluid_setX(fluid, 0.f, fix);
        } else {
            sq.pos->y = y;
            sq.pos->x = 0.f;
        }
        y -= spacingRef * node_deltaY(sq.pos) + extraHalfSpace;
    } while(sq_goRightWithout(&sq, flag_hidden|flag_notToAlign));
    return n;
}
