//
//  node_tree.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef _coq_node_tree_h
#define _coq_node_tree_h

#include "node_base.h"
#include "node_button.h"

void  node_tree_addFlags(Node* nd, flag_t flags);
void  node_tree_removeFlags(Node* nd, flag_t flags);
void  node_tree_apply(Node* nd, void (*block)(Node*));
void  node_tree_applyToTyped(Node* nd, void (*block)(Node*), uint32_t type_flag);
//void  node_childs_applyToTyped...
void  node_tree_addRootFlags(Node* nd, flag_t flags);
// supelflu void  node_tree_makeSelectable(Node* nd);
void  node_tree_openAndShow(Node* nd);
void  node_tree_unhideAndTryToOpen(Node* nd);
void  node_tree_close(Node* nd);
void  node_tree_hideAndTryToClose(Node* nd);
void  node_tree_reshape(Node* nd);

// Les options pour l'alignement de noeuds.
typedef enum {
    node_align_vertically =      0x0001,
    node_align_dontUpdateSizes = 0x0002,
    /** Ajoute de l'espacement supplémentaire entre les élément pour respecter le ratio w/h.
     (Compact si option absente.) */
    node_align_respectRatio =          0x0004,
    node_align_fixPos =                0x0008,
    /** En horizontal, le "primary" est "x" des children,
     * le "secondary" est "y". (En vertical prim->"y", sec->"x".)
     * Place la position "alignée" comme étant la position par défaut pour le primary des children
     * et pour le width/height du parent. Ne touche pas à defPos du secondary des children.
     * S'il y a "setSecondaryToDefPos", on place "y" à sa position par défaut,
     * sinon, on le place à zéro. */
    node_align_setSecondaryToDefPos =   0x0020,
    /// Par défaut (pour un Fluid) on set aussi la def pos du primary.
    /// Dans le cas du reshape de view, on n'y touche pas. 
    node_align_dontSetPrimaryDefPos =   0x0010,
} node_align_option;

/// Pour aligner les enfants d'un noeud les un par rapport aux autres.
/// alignOpt : Options d'alignement (voir plus bas);
/// ratio : le ratio voulu du noeud (defaut 1);
/// spacingRef : l'espacement entre les noeud (defaut 1).
int  node_tree_alignTheChildren(Node* nd, node_align_option alignOpt, float ratio, float spacingRef);


#endif /* node_tree_h */
