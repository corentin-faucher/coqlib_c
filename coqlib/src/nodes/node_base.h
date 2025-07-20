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
#include "../utils/util_timer.h"

// Pre-déclarations...
typedef struct coq_Node Node;

// MARK: Données d'un noeud non drawable
// (Pour les Drawables, voir `InstanceUniform` dans `graph_base.h`.)
typedef __attribute__((aligned(16))) struct NoDrawData {
    // Model et render_flags sont commun à tout les noeud (i.e. commun avec InstanceUniforms)
    Matrix4   _render_model;  
    uint32_t  _render_flags;  // Flags pour renderer et shaders
    float     _render_show;   // Flag analogique d'affichage du drawable.
    float     _render_extra1; // Floats extra customizables, e.g. emphase, flip, frequency...
    float     _render_extra2;
    // Data quelconques libres pour noeuds "non-drawables", e.g. les infos des boutons...
    // Prend la place de color et uvRect des InstanceUniforms.
    union {
        struct { Data128 data0, data1; };
    };
} NoDrawData;

// MARK: - Le Noeud.
typedef __attribute__((aligned(16))) struct coq_Node {
// Données d'un noeud:
// Première donnée est la matrice model, calculée par le renderer avec dimensions (plus bas).
// (les données render_... sont réservées pour la thread de rendering.)
// Pour les noeuds `non-drawable`, on peut utiliser les données pour des infos quelconques...
    union {
        InstanceUniforms renderIU;   // Pour drawables, i.e. avec flag de type `node_type_drawable`.
        NoDrawData       nodrawData; // Pour autres.
    };
// Dimensions/positions
// Note : sizes (w, h) est l'espace occupé dans *son* référentiel (interne).
// Pour l'espace occupé "de l'extérieure" on prend, e.g., delta x = w * sx / 2.
// Les positions x, y, z sont de l'extérieur. (La positon "interne" étant (0, 0, 0).)
// Les scales est le changement de scaling quand on passe au référentiel interne.
// Voir les chargement de référentiels dans `Box`.
    union {
        float dims[9];
        struct {
        union {
            Vector2 sizes;   // width et height seulement.
            Vector3 sizes3d; // Avec `depth` (rarement utile, par default depth = 0).
            struct { float w, h, d; };
        };
        union { // Scaling en tant que référentiel pour ses enfants.
            Vector3 scales;
            struct {float sx, sy, sz; };
        };
        union { // Position par rapport au parent (i.e. dans le référentiel du parent)
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

// MARK: - Méthodes overloadable d'un noeud...
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

// MARK: - Constructors
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


// MARK: - Model matrix ---
/// Retourne la matrice model du parent pour mise à jour de model (vérifie s'il y a parent).
const Matrix4* node_parentModel(Node const* n);
/// La fonction utilisée par défaut pour mettre à jour les instances uniformes avant l'affichage.
/// (Peut être remplacé par un fonction custom)
extern void (*Node_renderer_defaultUpdateInstanceUniforms)(Node*);


// MARK: - Getters --
/// "Demi-Espaces occupés"/"hitbox" du noeud de l'extérieur,
/// i.e. `Dx = w * sx / 2`, `Dy = h * sy / 2`.
static inline Vector2 node_deltas(Node const*const n);
static inline float   node_deltaX(Node const*const n);
static inline float   node_deltaY(Node const*const n);
static inline float   node_deltaZ(Node const*const n);
static inline Box     node_hitbox(Node const*const n);
/// Noeud en tant que "boîte" référentiel (non absolue, pour absolue il faut remonter à la root),
/// i.e. simplement position et scaling.
static inline Box     node_referential(Node const*const n);

/// Donne la position du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du
/// noeud.
Vector2 node_posInParentReferential(Node const* n, Node const* parentOpt);
/// Donne la position et les dimension (en demi-largeur/demi-hauteur)
/// du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du
/// noeud.
Box node_hitboxInParent(Node const* n, Node const* parentOpt);
/// Noeud en tant que "boîte" référentiel remonté jusqu'au (grand) parent.
Box node_referentialInParent(Node const* n, Node const* parentOpt);
// Superflu...
/// Position absolue (i.e. remonté à la root) d'une position dans le même référentiel que n (i.e. dans le ref de n->parent.)
//Vector2 vector2_toAbsolute(Vector2 pos, Node* n);
/// Convertie une position absolue (au niveau de la root) en une position
///  dans le réferentiel de nodeOpt (si NULL -> reste absPos).
//Vector2 vector2_absPosToPosInReferentialOfNode(Vector2 absPos, Node *nodeOpt);

// MARK: -- Move node, pour changer de place dans l'arbre de noeuds. --
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

/*-- Setters --*/
/// Permet d'aligner le noeud (à droite/gauche). Voir les `relatives_...` dans `math_base.h`.
/// ci-haut ou `flags_fluidRelative`... Dans le cas des Fluid,
/// `node_setXYrelatively` est callé par open et reshape et l'aligement se fait
/// par rapport à la positien par défaut.
void node_setXYrelatively(Node *node, uint32_t relative_flags, bool open);
void node_setXrelatively(Node* n, uint32_t relativeFlags, bool open);
void node_setYrelatively(Node* n, uint32_t relativeFlags, bool open);

// Pour déboguage
const char* node_debug_getTypeName(const Node* n);

// Inlines...
#include "node_base.inl"

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
