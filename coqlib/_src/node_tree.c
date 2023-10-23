//
//  node_tree.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#include "node_tree.h"
#include "node_squirrel.h"
#include "node_surface.h"
#include "utils.h"

void  node_tree_addFlags(Node* const nd, flag_t flags) {
    nd->flags |= flags;
    Node* firstChild = nd->firstChild;
    if(firstChild == NULL) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while(true) {
        sq.pos->flags |= flags;
        if(sq_goDown(&sq))
            continue;
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return;
            } else if(sq.pos == nd) return;
        }
    }
}
void  node_tree_removeFlags(Node* nd, flag_t flags) {
    if(nd == NULL) return;
    nd->flags &= ~flags;
    Node* firstChild = nd->firstChild;
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
    Node* firstChild = nd->firstChild;
    if(firstChild == NULL) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while(true) {
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
void  node_tree_applyToTyped(Node* nd, void (*block)(Node*), uint8_t type) {
    if(nd == NULL) return;
    if(nd->_type == type)
        block(nd);
    Node* firstChild = nd->firstChild;
    if(firstChild == NULL) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while(true) {
        if(sq.pos->_type == type)
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
void  node_tree_addRootFlag(Node* nd, flag_t flag) {
    Squirrel sq;
    sq_init(&sq, nd, sq_scale_ones);
    while(sq_goUp(&sq)) {
        if(sq.pos->flags & flag) break;
        sq.pos->flags |= flag;
    }
}
// Superflu...
//void  node_tree_makeSelectable(Node* nd) {
//    node_tree_addRootFlag(nd, flag_selectableRoot);
//
//}
void  node_tree_openAndShow(Node* nd) {
    if(nd->open) nd->open(nd);
    if(!(nd->flags & flag_hidden))
        nd->flags |= flag_show;
    if(!(nd->flags & flag_show)) return;
    Node* firstChild = nd->firstChild;
    if(firstChild == NULL) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while (true) {
        if(sq.pos->open) sq.pos->open(sq.pos);
        if(!(sq.pos->flags & flag_hidden))
            sq.pos->flags |= flag_show;
        if(sq.pos->flags & flag_show)
            if(sq_goDown(&sq))
                continue;
        while (!sq_goRight(&sq)) {
            if (!sq_goUp(&sq)) {
                printerror("Pas de branch."); return;
            } else if (sq.pos == nd) return;
        }
    }
}
void  node_tree_unhideAndTryToOpen(Node* nd) {
    nd->flags &= ~flag_hidden;
    if(!nd->parent) return;
    if(!(nd->parent->flags & flag_show)) return;
    node_tree_openAndShow(nd);
}
void  node_tree_close(Node* nd) {
    if(!(nd->flags & flag_exposed))
        nd->flags &= ~flag_show;
    if(nd->close) nd->close(nd);
    Node* firstChild = nd->firstChild;
    if(firstChild == NULL) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while (true) {
        if(!(sq.pos->flags & flag_exposed))
            sq.pos->flags &= ~flag_show;
        if(sq.pos->close) sq.pos->close(sq.pos);
        
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
    if(nd->reshape) nd->reshape(nd);
    if(!(nd->flags & flag_rootOfReshapable)) return;
    Node *firstChild = nd->firstChild;
    if(!firstChild) return;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while (true) {
        if(sq.pos->flags & flag_show) {
            if(sq.pos->reshape)
                sq.pos->reshape(sq.pos);
            if(sq.pos->flags & flag_rootOfReshapable)
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
NodeSel* node_tree_searchSelectableOpt(Node* const nd, Vector2 const absPos, Node* const nodeToAvoidOpt) {
    if(nd == nodeToAvoidOpt) return NULL;
    Vector2 relPos = vector2_toVec2InReferentialOfNode(absPos, nd);
    Squirrel sq;
    sq_initWithRelPos(&sq, nd, relPos, sq_scale_ones);
    if(!sq_isIn(&sq)) return NULL;
    if(!(sq.pos->flags & flag_show)) return NULL;
    NodeSel* candidate = NULL;
    // 1. Vérif la root
    if(sq.pos->_type & node_type_selectable)
        candidate = (NodeSel*)sq.pos;
    if(!(sq.pos->flags & flag_rootOfSelectable))
        return candidate;
    // 2. Se placer au premier child
    if(!sq_goDownP(&sq))
        return candidate;
    
    while (true) {
        if(sq_isIn(&sq) && (sq.pos->flags & flag_show) && sq.pos != nodeToAvoidOpt) {
            // 1. Possibilité trouvé
            if(sq.pos->_type & node_type_selectable) {
                candidate = (NodeSel*)sq.pos;
                if(!(sq.pos->flags & flag_rootOfSelectable))
                    return candidate;
            }
            // 2. Aller en profondeur
            if(sq.pos->flags & flag_rootOfSelectable) {
                if(sq_goDownP(&sq))
                    continue;
                else {
//                        printdebug("selectableRoot sans descendents ?")
                }
            }
        }
        // 3. Remonter, si plus de petit-frère
        while(!sq_goRight(&sq)) {
            if(!sq_goUpP(&sq)) {
                printerror("Pas de root."); return candidate;
            } else if(sq.pos == nd) return candidate;
        }
    }
}
Node* node_tree_searchFirstSelectableOfType(Node* nd, uint8_t type) {
    if(!nd) return NULL;
    if(!(nd->flags & flag_show)) return NULL;
    if(nd->_type == type) return nd;
    if(!(nd->flags & flag_rootOfSelectable)) return NULL;
    Node *firstChild = nd->firstChild;
    if(firstChild == NULL) return NULL;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while(true) {
        if(sq.pos->flags & flag_show) {
            if(sq.pos->_type == type)
                return sq.pos;
            if(sq.pos->flags & flag_rootOfSelectable)
                if(sq_goDown(&sq))
                    continue;
        }
        // 3. Remonter, si plus de petit-frère
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return NULL;
            } else if(sq.pos == nd) return NULL;
        }
    }
}
NodeSel* node_tree_searchFirstSelectableOptUsing(Node* const nd, Bool (*testIsValid)(NodeSel*)) {
    if(!(nd->flags & flag_show)) return NULL;
    if(nd->_type & node_type_selectable) {
        NodeSel* sel=(NodeSel*)nd;
        if(testIsValid(sel))
            return sel;
    }
    if(!(nd->flags & flag_rootOfSelectable)) return NULL;
    Node *firstChild = nd->firstChild;
    if(firstChild == NULL) return NULL;
    Squirrel sq;
    sq_init(&sq, firstChild, sq_scale_ones);
    while(true) {
        if(sq.pos->flags & flag_show) {
            if(sq.pos->_type & node_type_selectable) {
                NodeSel* sel=(NodeSel*)sq.pos;
                if(testIsValid(sel))
                    return sel;
            }
            if(sq.pos->flags & flag_rootOfSelectable)
                if(sq_goDown(&sq))
                    continue;
        }
        // 3. Remonter, si plus de petit-frère
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return NULL;
            } else if(sq.pos == nd) return NULL;
        }
    }
}

int  node_tree_alignTheChildren(Node* nd, node_align_option alignOpt, float ratio, float spacingRef) {
    Squirrel sq;
    sq_init(&sq, nd, sq_scale_ones);
    if(!sq_goDownWithout(&sq, flag_hidden|flag_notToAlign))
        return 0;
    Bool fix = ((alignOpt & node_align_fixPos) != 0);
    Bool horizontal = ((alignOpt & node_align_vertically) == 0);
    Bool setSecondaryToDefPos = ((alignOpt & node_align_setSecondaryToDefPos) != 0);
    // 1. Mesurer largeur et hauteur requises
    float w = 0;
    float h = 0;
    int n = 0;
    if(horizontal) {
        float htmp;
        do {
            w += 2.f*spacingRef*node_deltaX(sq.pos);
            n ++;
            htmp = 2.f*node_deltaY(sq.pos);
            if(htmp > h) h = htmp;
        } while(sq_goRightWithout(&sq, flag_hidden|flag_notToAlign));
    } else {
        float wtmp;
        do {
            h += 2.f*spacingRef*node_deltaY(sq.pos);
            n ++;
            wtmp = 2.f*node_deltaX(sq.pos);
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
        if(nd->flags & flag_giveSizeToBigbroFrame) {
            Node* bigBro = nd->bigBro;
            if(bigBro) if(bigBro->_type & node_type_surface)
                nodesurf_frame_updateWithLittleBro((NodeSurf*)bigBro);
        }
    }
    // 4. Aligner les éléments
    sq_init(&sq, nd, sq_scale_ones);
    if(!sq_goDownWithout(&sq, flag_hidden|flag_notToAlign))
        return 0;
    if(horizontal) {
        float x = -w / 2.f;
        do {
            x += spacingRef * node_deltaX(sq.pos) + extraHalfSpace;
            node_setX(sq.pos, x, fix);
            if(setSecondaryToDefPos && (sq.pos->_type & node_type_smooth))
                nodesmooth_setYRelToDef((NodeSmooth*)sq.pos, 0, fix);
            else
                node_setY(sq.pos, 0, fix);
            x += spacingRef * node_deltaX(sq.pos) + extraHalfSpace;
        } while(sq_goRightWithout(&sq, flag_hidden|flag_notToAlign));
        return n;
    }
    // Aligner verticalement
    float y = +h / 2.f; // (de haut en bas)
    do {
        y -= spacingRef * node_deltaY(sq.pos) + extraHalfSpace;
        node_setY(sq.pos, y, fix);
        if(setSecondaryToDefPos && (sq.pos->_type & node_type_smooth)) {
            nodesmooth_setXRelToDef((NodeSmooth*)sq.pos, 0, fix);
        } else {
            node_setX(sq.pos, 0, fix);
        }
        y -= spacingRef * node_deltaY(sq.pos) + extraHalfSpace;
    } while(sq_goRightWithout(&sq, flag_hidden|flag_notToAlign));
    return n;
}
