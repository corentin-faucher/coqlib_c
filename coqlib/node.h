//
//  node.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef node_h
#define node_h

#include "graph.h"
#include "node_flags.h"
#include "node_types.h"

typedef struct _Node Node;

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
    size_t    _size; // Utile ?
    node_t   _type;
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

Node* Node_createEmpty(void);
/// Constructeur avec setter des propriétés usuelles.
Node* Node_create(Node* ref,
                  float x, float y, float w, float h,
                  flag_t flags, uint8_t node_place);
/// Constructeur de copie.
Node* Node_createCopy(Node* other);
/// Enlever un noeud de la structure. Ne doit plus être référencé.
/// (Les noeuds sont placé dans la "poubelle")
void  node_destroy(Node *nodeToDelete);
/// Free les noeuds dans la poubelle. A faire régulièrement
/// (à chaque 2~3 frame? ou après avoir updaté la structure).
void  node_garbage_burn(void);

// Utiles ?
//Vector3 node_pos(Node *node);    // x / y
//Vector2 node_wh(Node *node);   // width / height
//Vector3 node_scales(Node *node); // scaleX / scaleY

/*-- Getters --*/
/// "Demi-Espaces occupés" ou "w" et "h" du noeud de l'extérieur.
/// Dx = w * sx / 2, Dy = h * sy / 2.
Vector2 node_deltas(Node *node);
float   node_deltaX(Node *node);
float   node_deltaY(Node *node);
int     node_isDisplayActive(Node *node);

/*-- Setters --*/
/// Set x en verifiant si c'est NodeSmooth ou Node.
void    node_setX(Node* const nd, float x, Bool fix);
/// Set y en tant que NodeSmooth ou Node.
void    node_setY(Node* const nd, float y, Bool fix);
void    node_setScaleX(Node* const nd, float sx, Bool fix);
void    node_setScaleY(Node* const nd, float sy, Bool fix);

void    node_simpleMoveTo(Node* const node, Node* const destOpt, const uint8_t node_place);
void    node_simpleMoveToBro(Node* const node, Node* const broOpt, const Bool asBig);
void    node_simpleMoveToParent(Node* const node, Node* const parentOpt, const Bool asElder);

void    node_updateModelMatrixWithParentReferential(Node* const nd, Node* const parent);

Vector2 vector2_toVec2InReferentialOfNode(const Vector2 v, Node* const nodeOpt);

// Internal, pour les sous-classes.
Node* _Node_createEmptyOfType(const uint8_t type, const size_t size, const flag_t flags,
                              Node* const refOpt, const uint8_t node_place) ;

#endif /* node_h */
