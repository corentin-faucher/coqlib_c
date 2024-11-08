//
//  node.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_NODE_BASE_H
#define COQ_NODE_BASE_H

#include "node__flags.h"
#include "node__types.h"
#include "../graphs/graph_base.h"
#include "../coq_timer.h"

// Pre-déclarations...
typedef struct coq_Node Node;
// typedef struct coq_View View;
// typedef struct coq_Root Root;
// typedef struct coq_Button Button;
typedef struct Coq_Drawable Drawable;
// Une variable "Timer" est en fait un pointeur vers la structure privé coq_TimerStruct. (i.e. une "variable" Java style...)
typedef struct coq_TimerStruct* Timer;


#pragma mark - Le Noeud.
typedef __attribute__((aligned(16))) struct coq_Node {
// Données d'un noeud:
// Première donnée est la matrice model. Calculer lors de l'affichage 
// en fonction du parent et des Dimensions (voir plus bas).
  InstanceUniforms _iu;
    
// Dimensions/positions
// Note : sizes (w, h) est l'espace occupé dans *son* référentiel (interne).
// Pour l'espace occupé "de l'extérieure" on prend, e.g., delta x = w * sx / 2.
// Les positions x, y, z sont de l'extérieur. (La positon "interne" étant (0, 0, 0).)
// Les scales est le changement de scaling quand on passe au référentiel interne.
  __attribute__((aligned(16))) union {
    float dims[8];
    struct {
      union {
        Vector2 sizes;
        struct { float w, h; };
      };
      union {
        Vector2 scales;
        struct {float sx, sy; };
      };
      union {
        Vector4 pos4;
        struct { Vector3 xyz; };
        struct { Vector2 xy;  };
        struct { float x, y, z; };
      };
    };
  };
  
// Flags, type de noeud et ID de noeud.
  flag_t   flags;
  uint32_t _type;
#ifdef DEBUG
  uint32_t _nodeId;
#endif

#pragma mark - Méthodes overloadable d'un noeud...
  /// Action à l'ouverture du noeud, e.g. placer par rapport au parent.
  void (*openOpt)(Node *);
  /// Action à la fermeture, e.g. fade out.
  void (*closeOpt)(Node *);
  /// Action après un reshape, e.g. replacer dans le cadre du parent.
  void (*reshapeOpt)(Node *);
  /// Action avant free/delete. (Liberer des resoureces attachées)
  void (*deinitOpt)(Node *);
  /// Méthode pour updater les InstanceUniforms (Caller par Renderer).
  /// Par défault : `Node_renderer_defaultUpdateInstanceUniforms`.
  void (*renderer_updateInstanceUniforms)(Node*);

// Liens avec les voisins (volatile superflu ?)
  Node* volatile _parent;
  Node* volatile _firstChild;
  Node* volatile _lastChild;
  Node* volatile _bigBro;
  Node* volatile _littleBro;
} Node;

// Flag de placement d'un nouveau noeud : enfant/frere, aine/cadet.
// Defaut 0 -> enfant, cadet.
enum {
    node_place_asBro =      0x0001,
    node_place_asElderBig = 0x0002,
//    node_place_asBigBro =   0x0003,
};

#pragma mark - Constructors

/// Init de base d'un noeud. Set les dimensions, connecte avec les voisins...
/// (Ne set pas le type. Chaque sous-structure ajoute son flag de sous-type.)
void node_init(Node* n, Node* const refOpt,
                 float x, float y, float width, float height,
                 const flag_t flags, const uint8_t node_place);
                 
/// Le dernier noeud taggé avec le flag `flag_lastNode` ou tout simplement asigné : `Node_last = Node_create(...);`.
extern Node *Node_last;

/// Convenience init de noeud simple. Juste `calloc` + `node_init`.
Node* Node_create(Node* refOpt, 
                float x, float y, float width, float height, 
                flag_t flags, uint8_t node_place);
/// Mettre le noeud référé à la poubelle et met le pointeur à NULL.
/// Pas de délai, l'objet disparaît d'un coup.
void noderef_destroyAndNull(Node **nodeOptRef);
/// Version avec délai de `noderef_fastDestroyAndNull`.
/// Close, et schedule destroy. `deltaTimeMS` = 400 par défaut (si mit à 0).
void  noderef_slowDestroyAndNull(Node** const nodeOptRef, Timer *parentDestroyTimer, int64_t deltaTimeMSOpt);
/// Free les noeuds dans la poubelle. A faire régulièrement
/// (à chaque 2~3 frame? ou après avoir updaté la structure).
void NodeGarbage_burn(void);
/// `Privé` : Enleve un noeud de la structure, ** Ne doit plus être référencé. **,
/// et le place dans la poubelle.
/// Utiliser de préférence les versions plus safe `noderef_destroyAndNull`...
void node_throwToGarbage_(Node* n);
/// Version `void *` pour timer callback. 
static void (* const node_throwToGarbage__)(void*) = (void (*)(void*))node_throwToGarbage_;


#pragma mark - Model matrix ---
/// Retourne la matrice model du parent pour mise à jour de model (vérifie s'il y a parent).
const Matrix4* node_parentModel(Node* n);
/// La fonction utilisée par défaut pour mettre à jour les instances uniformes avant l'affichage.
/// (Peut être remplacé par un fonction custom)
extern void (*Node_renderer_defaultUpdateInstanceUniforms)(Node*);


#pragma mark - Getters --
/// "Demi-Espaces occupés"/"hitbox" du noeud de l'extérieur,
/// i.e. `Dx = w * sx / 2`, `Dy = h * sy / 2`.
Vector2 node_deltas(Node *node);
float   node_deltaX(Node *node);
float   node_deltaY(Node *node);
Vector3 node_pos(Node *node);    // (x,y,z)
Vector2 node_scales(Node *node); // scaleX / scaleY


#pragma mark -- Move node, pour changer de place dans l'arbre de noeuds. --
/// Place le noeud à côté de `destOpt` (sans ajuster les positions spatiales)
void node_simpleMoveTo(Node* node, Node* destOpt,
                       uint8_t node_place);
/// Place le noeud comme frère de `broOpt` (sans ajuster les positions spatiales)
void node_simpleMoveToBro(Node* node, Node* broOpt,
                          bool asBig);
/// Place le noeud comme descendant de `parentOpt` (sans ajuster les positions spatiales)
void node_simpleMoveToParent(Node* node, Node* parentOpt,
                             bool asElder);
void node_moveWithinBro(Node* node, bool asElder);
void node_moveRight(Node* node);
/** Change de noeud de place (et ajuste sa position/scaling relatif). */
void node_moveToParent(Node *n, Node *new_parent, bool asElder);
/** Change de noeud de place (et ajuste sa position/scaling relatif). */
void node_moveToBro(Node *n, Node *new_bro, bool asBigBro);


#pragma mark -- Positions spatiales (x,y) --
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
/// Position absolue (i.e. remonté à la root) d'une position dans le même référentiel que n (i.e. dans le ref de n->parent.)
Vector2 vector2_toAbsolute(Vector2 pos, Node* n);
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
//  relative_parentCenter = flag_fluidRelativeToCenter,
  relative_justifiedRight = flag_fluidJustifiedRight,
  relative_justifiedLeft = flag_fluidJustifiedLeft,
  relative_justifiedTop = flag_fluidJustifiedTop,
  relative_justifiedBottom = flag_fluidJustifiedBottom,
  relatives_top = relative_parentTop | relative_justifiedTop,
  relatives_bottom = relative_parentBottom | relative_justifiedBottom,
  relatives_left = relative_parentLeft | relative_justifiedLeft,
  relatives_right = relative_parentRight | relative_justifiedRight,
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
void node_setXrelatively(Node* n, uint32_t relativeFlags, bool open);
void node_setYrelatively(Node* n, uint32_t relativeFlags, bool open);

// Pour déboguage
const char* node_debug_getTypeName(const Node* n);

#endif /* node_h */

// Garbage
/// Allocation d'un noeud de type quelconque.
/// Obsolete : utiliser `coq_calloc` + `node_init_`...
//__attribute__((deprecated("utiliser `coq_calloc` + `node_init_`."))) void *
//Node_createOfType_Obsolete_(uint32_t type, size_t size,
//                            flag_t flags, Node* refOpt,
//                            uint8_t node_place);
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
