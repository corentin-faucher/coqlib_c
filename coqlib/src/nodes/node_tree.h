//
//  node_tree.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#ifndef coq_node_tree_h
#define coq_node_tree_h

#include "node_base.h"
#include "node_button.h"
#include "../utils/util_event.h"

// MARK: - "Squirrel": Méthodes pour déplacer un pointeur de noeud dans l'arbre de la structure.
void nodeptr_goRightForced(Node ** sq, Node *(*createNew)(void *), void *paramsOpt);
bool nodeptr_throwToGarbageThenGoToBroOrUp(Node ** sq);
bool nodeptr_renderer_goToNextToDisplay(Node ** sq);

// MARK: - Methodes qui s'applique à toute la branche (noeud et descendants)
void  node_tree_addFlags(Node* nd, flag_t flags);
void  node_tree_removeFlags(Node* nd, flag_t flags);
void  node_tree_apply(Node* nd, void (*block)(Node*));
void  node_tree_applyToTyped(Node* nd, void (*block)(Node*), uint32_t type_flag);
// (ici, ça s'applique aux parents, mais pas au noeud présent)
void  node_tree_addRootFlags(Node* nd, flag_t flags);
void  node_tree_openAndShow(Node* nd);
void  node_tree_unhideAndTryToOpen(Node* nd);
void  node_tree_close(Node* nd);
void  node_tree_hideAndTryToClose(Node* nd);
void  node_tree_reshape(Node* nd);

// MARK: - Recherche de noeud dans une branche.
NodeTouch    node_tree_searchActiveButtonWithPosOpt(NodeTouch nt, Node const* nodeToAvoidOpt);
Node*        node_tree_searchFirstOfTypeWithDataOpt(Node * n, uint32_t type, uint32_t data0);
Node*        node_tree_searchFirstOfTypeInBranchOpt(Node * n, uint32_t type_flag, flag_t parentFlag);

// MARK: - Aligement des enfants
// Les options pour l'alignement de noeuds.
typedef enum {
    node_align_vertically =            0x0001,
    node_align_dontUpdatePrimarySize = 0x0002,
    node_align_dontUpdateSecondarySize = 0x0004,
    /** Ajoute de l'espacement supplémentaire entre les élément pour respecter le ratio w/h.
     (Compact si option absente.) */
    node_align_respectRatio =          0x0008,
    node_align_fixPos =                0x0010,
    /** En horizontal, le "primary" est "x" des children,
     * le "secondary" est "y". (En vertical prim->"y", sec->"x".)
     * Place la position "alignée" comme étant la position par défaut pour le primary des children
     * et pour le width/height du parent. Ne touche pas à defPos du secondary des children.
     * S'il y a "setSecondaryToDefPos", on place "y" à sa position par défaut,
     * sinon, on le place à zéro. */
    node_align_setSecondaryToDefPos =   0x0020,
    /// Par défaut (pour un Fluid) on set aussi la def pos du primary.
    /// Dans le cas du reshape de view, on n'y touche pas. 
    node_align_dontSetPrimaryDefPos =   0x0040,
    /// Si vertical -> les éléments seront du côté droit.
    /// Si horizontal -> les éléments seront en haut.
    /// (`node_align_setSecondaryToDefPos` prend le dessus sur l'alignement)
    node_align_rightBottom =            0x0080,
    node_align_leftTop =                0x0100,
} node_align_option;
/// Pour aligner les enfants d'un noeud les un par rapport aux autres.
/// alignOpt : Options d'alignement (voir plus bas);
/// ratio : le ratio voulu du noeud (defaut 1);
/// spacingRef : l'espacement entre les noeud (defaut 1).
int  node_tree_alignTheChildren(Node* nd, node_align_option alignOpt, float ratio, float spacingRef);


#endif /* node_tree_h */
