//
//  node_tree.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef _coq_node_tree_h
#define _coq_node_tree_h

#include "_node.h"
#include "_node_button.h"
#include "_node_sliding_menu.h"

void  node_tree_addFlags(Node* nd, flag_t flags);
void  node_tree_removeFlags(Node* nd, flag_t flags);
void  node_tree_apply(Node* nd, void (*block)(Node*));
void  node_tree_applyToTyped(Node* nd, void (*block)(Node*), uint8_t type);
//void  node_childs_applyToTyped...
void  node_tree_addRootFlag(Node* nd, flag_t flag);
// supelflu void  node_tree_makeSelectable(Node* nd);
void  node_tree_openAndShow(Node* nd);
void  node_tree_unhideAndTryToOpen(Node* nd);
void  node_tree_close(Node* nd);
void  node_tree_hideAndTryToClose(Node* nd);
void  node_tree_reshape(Node* nd);

//Node* node_

Button*      root_searchButtonOpt(Root* const root, Vector2 const absPos,
                                 Node* const nodeToAvoidOpt);
SlidingMenu* root_searchFirstSlidingMenuOpt(Root* root);

// Les options pour l'alignement de noeuds.
typedef enum {
    node_align_vertically =      0x0001,
    node_align_dontUpdateSizes = 0x0002,
    /** Ajoute de l'espacement supplémentaire entre les élément pour respecter le ratio w/h.
     (Compact si option absente.) */
    node_align_respectRatio =    0x0004,
    node_align_fixPos =          0x0008,
    // Utile ?
    //node_align_setAsDefPos =     0x0010,
    /** En horizontal, le "primary" est "x" des children,
     * le "secondary" est "y". (En vertical prim->"y", sec->"x".)
     * Place la position "alignée" comme étant la position par défaut pour le primary des children
     * et pour le width/height du parent. Ne touche pas à defPos du secondary des children.
     * S'il y a "setSecondaryToDefPos", on place "y" à sa position par défaut,
     * sinon, on le place à zéro. */
    node_align_setSecondaryToDefPos = 0x0020,
} node_align_option;

/// Pour aligner les enfants d'un noeud les un par rapport aux autres.
/// alignOpt : Options d'alignement (voir plus bas);
/// ratio : le ratio voulu du noeud (defaut 1);
/// spacingRef : l'espacement entre les noeud (defaut 1).
int  node_tree_alignTheChildren(Node* nd, node_align_option alignOpt, float ratio, float spacingRef);


#endif /* node_tree_h */
