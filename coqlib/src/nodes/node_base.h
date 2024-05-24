//
//  node.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_NODE_BASE_H
#define COQ_NODE_BASE_H

#include "../graphs/graph_base.h"
#include "node__flags.h"
#include "node__types.h"
#include <stdint.h>
#include <stdio.h>

typedef struct coq_Node Node;
// typedef struct coq_View View;
// typedef struct coq_Root Root;
// typedef struct coq_Button Button;
typedef struct Coq_Drawable Drawable;

typedef __attribute__((aligned(16))) struct coq_Node {
  PerInstanceUniforms _piu; // Mieux vaut être bien aligne pour le gpu/simd...
  union {
    float dims[8];
    struct {
      Vector2 sizes;
      Vector2 scales;
      Vector3 pos; // Placé à 96 + 16 un multiple de 16B. (compatible avec simd)
                   // (float unused; dans le padding du vector3)
    };
    struct {
      float w, h, sx, sy, x, y, z;
      // (float unused;)
    };
  };
  flag_t flags;
  //    size_t    _size;
  uint32_t _type;
#ifdef DEBUG
  uint32_t _nodeId;
#endif
  /// Action à l'ouverture du noeud, e.g. placer par rapport au parent.
  void (*openOpt)(Node *);
  /// Action à la fermeture, e.g. fade out.
  void (*closeOpt)(Node *);
  /// Action après un reshape, e.g. replacer dans le cadre du parent.
  void (*reshapeOpt)(Node *);
  /// Action avant free/delete. (Liberer des resoureces attachées)
  void (*deinitOpt)(Node *);
  /// Méthode pour updater les PerInstanceUniforms (Caller par Renderer).
  /// Retourne le drawable à dessiner si c'est un drawable (NULL sinon). 
  /// Par défault : `node_updateModelMatrixDefault`.
  Drawable* (*updateModel)(Node*);

  Node *_parent;
  Node *_firstChild;
  Node *_lastChild;
  Node *_bigBro;
  Node *_littleBro;
} Node;

// Flag de placement d'un nouveau noeud : enfant/frere, aine/cadet.
// Defaut 0 -> enfant, cadet.
// Enum ?
#define node_place_asBro 0x0001
#define node_place_asElderBig 0x0002

#pragma mark - Constructors

Node *Node_createEmpty(void);
/// Constructeur de noeud ordinaire avec dimensions.
Node *Node_create(Node *refOpt, float x, float y, float w, float h,
                  flag_t flags, uint8_t node_place);
/// Constructeur de copie par defaut.
/// Faire des versions particuliere si besoin de plus que memcpy.
// Node* Node_default_createCopy(const Node* const other);
/// Enlever un noeud de la structure. Ne doit plus être référencé.
/// (Les noeuds sont placé dans la "poubelle")
void node_tree_throwToGarbage(Node *node);
/// Mettre le noeud référé à la poubelle et met le pointeur à NULL.
/// Pas de délai, l'objet disparaît d'un coup.
void noderef_fastDestroyAndNull(Node **nodeOptRef);
/// Version avec délai de `noderef_fastDestroyAndNull`.
/// Close, et schedule destroy. deltaTimeMS = 400 par défaut (si mit à 0).
void noderef_slowDestroyAndNull(Node **nodeOptRef, int64_t deltaTimeMSOpt);
/// Free les noeuds dans la poubelle. A faire régulièrement
/// (à chaque 2~3 frame? ou après avoir updaté la structure).
void NodeGarbage_burn(void);

#pragma mark - /*-- Getters --*/

/// "Demi-Espaces occupés"/"hitbox" du noeud de l'extérieur,
/// i.e. `Dx = w * sx / 2`, `Dy = h * sy / 2`.
Vector2 node_deltas(Node *node);
float node_deltaX(Node *node);
float node_deltaY(Node *node);
Vector3 node_pos(Node *node);    // (x,y,z)
Vector2 node_scales(Node *node); // scaleX / scaleY

/*-- Movers, pour changer de place dans l'arbre de noeuds. --*/
void node_simpleMoveTo(Node *const node, Node *const destOpt,
                       const uint8_t node_place);
void node_simpleMoveToBro(Node *const node, Node *const broOpt,
                          const bool asBig);
void node_simpleMoveToParent(Node *const node, Node *const parentOpt,
                             const bool asElder);
void node_moveWithinBro(Node *const node, bool asElder);
/** Change de noeud de place (et ajuste sa position/scaling relatif). */
void node_moveToParent(Node *n, Node *new_parent, bool asElder);
/** Change de noeud de place (et ajuste sa position/scaling relatif). */
void node_moveToBro(Node *n, Node *new_bro, bool asBigBro);

/*-- Maths (vecteur position et model matrix) --*/
/// Mise à jour par défaut de la matrice modèle pour se placer dans le référentiel du parent.
Drawable* node_updateModelMatrixDefault(Node *n);
/// Donne la position du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du
/// noeud.
Vector2 node_posInParentReferential(Node *n, const Node *parentOpt);
/// Donne la position et les dimension (en demi-largeur/demi-hauteur)
/// du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du
/// noeud.
Box node_hitBoxInParentReferential(Node *n, const Node *parentOpt);
/// Hitbox absolue (i.e. remonté à la root) d'une box dans le même référentiel que nRef (i.e. dans le ref de nRef->parent.)
Box box_toAbsolute(Box box, Node* nRef);
/// Convertie une position absolue (au niveau de la root) en une position
///  dans le réferentiel de nodeOpt (si NULL -> reste absPos).
Vector2 vector2_absPosToPosInReferentialOfNode(Vector2 absPos, Node *nodeOpt);

/*-- Setters --*/
// Flags pour `node_setXYrelatively`.
// Même chose que les `flags_fluidRelative` dans `node__flags.h` gardés en
// mémoire pour les Fluid.
enum {
  relative_parentRight = flag_fluidRelativeToRight,
  relative_parentLeft = flag_fluidRelativeToLeft,
  relative_parentTop = flag_fluidRelativeToTop,
  relative_parentBottom = flag_fluidRelativeToBottom,
  relative_justifiedRight = flag_fluidJustifiedRight,
  relative_justifiedLeft = flag_fluidJustifiedLeft,
  relative_justifiedTop = flag_fluidJustifiedTop,
  relative_justifiedBottom = flag_fluidJustifiedBottom,
  relatives_top = relative_parentTop | relative_justifiedTop,
  relatives_bottom = relative_parentBottom | relative_justifiedBottom,
  relatives_topRight = relative_parentRight | relative_parentTop |
                       relative_justifiedRight | relative_justifiedTop,
  relatives_topLeft = relative_parentLeft | relative_parentTop |
                      relative_justifiedLeft | relative_justifiedTop,
  relatives_bottomRight = relative_parentRight | relative_parentBottom |
                          relative_justifiedRight | relative_justifiedBottom,
  relatives_bottomLeft = relative_parentLeft | relative_parentBottom |
                         relative_justifiedLeft | relative_justifiedBottom,
};
/// Permet d'aligner le noeud (à droite/gauche). Voir les `relatives_...`
/// ci-haut ou `flags_fluidRelative`... Dans le cas des Fluid,
/// `node_setXYrelatively` est callé par open et reshape et l'aligement se fait
/// par rapport à la positien par défaut.
void node_setXYrelatively(Node *node, uint32_t relative_flags, bool open);

///// Set x en verifiant si c'est Fluid ou Node.
// void    node_setX(Node* n, float x);
// void    node_fixX(Node* n, float x);
/////// Set y en tant que Fluid ou Node.
// void    node_setY(Node* n, float y);
// void    node_fixY(Node* n, float y);
// void    node_setScaleX(Node* n, float sx);
// void    node_fixScaleX(Node* n, float sx);
// void    node_setScaleY(Node* n, float sy);
// void    node_fixScaleY(Node* n, float sy);

/// Init de base d'un noeud.
void node_init_(Node *n, Node *const refOpt, float x, float y, float width,
                float height, const uint32_t type, const flag_t flags,
                const uint8_t node_place);
/// Allocation d'un noeud de type quelconque.
/// Obsolete : utiliser `coq_calloc` + `node_init_`...
__attribute__((deprecated("utiliser `coq_calloc` + `node_init_`."))) void *
Node_createOfType_Obsolete_(const uint32_t type, const size_t size,
                            const flag_t flags, Node *const refOpt,
                            const uint8_t node_place);
/// Le dernier noeud init qui n'est pas une feuille (i.e. pas drawable)
extern Node *node_last_nonDrawable;

// Pour déboguage
const char* node_debug_getTypeName(const Node* const n);

#endif /* node_h */
