//
//  node.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef node_h
#define node_h

#include "graph.h"
#include "_node_flags.h"
#include "_node_types.h"
#include "utils.h"

typedef struct _Node Node;
typedef struct _Root Root;
typedef struct _Button Button;

typedef __attribute__((aligned(16)))struct _Node {
    PerInstanceUniforms piu; // Mieux vaut être bien aligne pour le gpu/simd...
    union {
        float dims[8];
        struct {
            Vector2   wh;
            Vector2   scales;
            Vector3   pos;  // Placé à 96 + 16 un multiple de 16B. (compatible avec simd)
            // (float unused; dans le padding du vector3)
        };
        struct {
            float w, h, sx, sy, x, y, z;
            // (float unused;)
        };
    };
    flag_t    flags;
    size_t    _size; // (Pour faire une copie.)
    uint32_t  _type;
    /// Action à l'ouverture du noeud, e.g. placer par rapport au parent.
    void     (*open)(Node*);
    /// Action à la fermeture, e.g. fade out.
    void     (*close)(Node*);
    /// Action après un reshape, e.g. replacer dans le cadre du parent.
    void     (*reshape)(Node*);
    /// Action avant free/delete. (Liberer des resoureces liées)
    void     (*deinit)(Node*);
    
    Node     *parent;
    Node     *firstChild;
    Node     *lastChild;
    Node     *bigBro;
    Node     *littleBro;
} Node;

// Flag de placement d'un nouveau noeud : enfant/frere, aine/cadet.
// Defaut 0 -> enfant, cadet.
// Enum ?
#define node_place_asBro      0x0001
#define node_place_asElderBig 0x0002

/*-- Constructeurs (et destroy/garbage) --*/
Node* Node_createEmpty(void);
/// Constructeur avec setter des propriétés usuelles.
Node* Node_create(Node* ref,
                  float x, float y, float w, float h,
                  flag_t flags, uint8_t node_place);
/// Constructeur de copie par defaut.
/// Faire des versions particuliere si besoin de plus que memcpy.
Node* Node_default_createCopy(const Node* const other);
/// Enlever un noeud de la structure. Ne doit plus être référencé.
/// (Les noeuds sont placé dans la "poubelle")
void  node_tree_throwToGarbage(Node* nd);
/// Free les noeuds dans la poubelle. A faire régulièrement
/// (à chaque 2~3 frame? ou après avoir updaté la structure).
void  NodeGarbage_burn(void);

/*-- Getters --*/
/// "Demi-Espaces occupés" ou "w" et "h" du noeud de l'extérieur.
/// Dx = w * sx / 2, Dy = h * sy / 2.
Vector2 node_deltas(Node *node);
float   node_deltaX(Node *node);
float   node_deltaY(Node *node);
int     node_isDisplayActive(Node *node);
Vector3 node_pos(Node *node);      // (x,y,z)
//Vector3 node_scales(Node *node); // scaleX / scaleY

/*-- Setters --*/
/// Set x en verifiant si c'est Fluid ou Node.
void    node_setX(Node* const nd, float x, Bool fix);
/// Set y en tant que Fluid ou Node.
void    node_setY(Node* const nd, float y, Bool fix);
void    node_setScaleX(Node* const nd, float sx, Bool fix);
void    node_setScaleY(Node* const nd, float sy, Bool fix);

/*-- Movers --*/
void    node_simpleMoveTo(Node* const node, Node* const destOpt, const uint8_t node_place);
void    node_simpleMoveToBro(Node* const node, Node* const broOpt, const Bool asBig);
void    node_simpleMoveToParent(Node* const node, Node* const parentOpt, const Bool asElder);

/*-- Maths (vecteur position et model matrix) --*/
/// Mise à jour ordinaire de la matrice modèle pour se placer dans le référentiel du parent.
/// Alpha est un facteur de grossisement (pour popping)
void    node_updateModelMatrixWithParentModel(Node* n, const Matrix4* parentModel, float alpha);
/// Donne la position du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du noeud.
Vector2 node_posInParentReferential(Node* n, const Node* parentOpt);
/// Donne la position et les dimension (en demi-largeur/demi-hauteur)
/// du noeud dans le référentiel d'un (grand) parent.
/// e.g. si parentOpt est la root (ou NULL) -> on obtient la position absolue du noeud.
Box     node_hitBoxInParentReferential(Node* n, const Node* parentOpt);
/// Convertie une position absolue (au niveau de la root) en une position
///  dans le réferentiel de nodeOpt (si NULL -> reste absPos).
Vector2 vector2_absPosToPosInReferentialOfNode(Vector2 absPos, Node* nodeOpt);

// Internal constructor, pour les sous-classes.
void* _Node_createEmptyOfType(const uint32_t type, const size_t size, const flag_t flags,
                              Node* const refOpt, const uint8_t node_place) ;
// Le dernier noeud créé (avec _Node_createEmptyOfType a priori)
extern Node* _node_last_created;

#endif /* node_h */
