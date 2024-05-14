//
//  Drawables : les noeuds feuilles étant affichés,
//  i.e. avec mesh et texture.
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef _coq_node_drawable_h
#define _coq_node_drawable_h

#include "node_base.h"
#include "../maths/math_smtrans.h"
#include "../graphs/graph_texture.h"
#include "../graphs/graph_mesh.h"

/// Noeud affichable (image, string).
/// Pour un drawable les dimensions sont données par scaleX et scaleY.
/// w et h sont relatif à 1 pour donner l'overlapping/spacing.
/// (et espace occupé est alors `deltaX = w * scaleX`.)
/// Voir `_drawable_updateScaleXAndWidth` et `node_updateModelMatrixWithParentModel`.
typedef struct Coq_Drawable {
    /// Upcasting as Node.
    Node     n;
    /// La texture attaché. Peut être owned ou shared.
    /// C'est plus ou moins `privé`... On ne devrait pas avoir besoin d'y toucher.
    /// (Les texture doivent être `release` quand on en a plus besoin...
    ///  Drawable s'occupe de release sa texture lors du deinit.) 
    Texture* _tex;
    /// La mesh attaché. Peut être owned ou shared.
    /// Ici aussi c'est plus ou moins privé, et le deinit de Drawable libère la mesh si besoin.
    Mesh*    _mesh;
    /// Smooth transition pour affichage ON/OFF.
    SmTrans  trShow;
    /// Transition en extra ! e.g. emphasis, flip, etc.
    SmTrans  trExtra;
    /// Mémoire des dimensions `target`. Utile pour l'edition de strings.
    /// twoDxTarget == 0 => On prend le ratio de la texture...
    float    _twoDxTarget, _twoDyTarget;
    /// Marge en x. -> Ratio de deltaY.
    ///  (peut être négatif, voir `_drawable_updateScaleXAndWidth`)
    float    _xMargin;
} Drawable;

/// La fonction utilisée pour mettre à jour la matrice modèle avant l'affichage.
/// (Peut être remplacé par un fonction custom)
extern Drawable* (*Drawable_defaultUpdateModel)(Node*);

/// Constructeur Général. Utilisez a priori les constructeurs spécifiques...
/// (image, string, frame...).
__attribute__((deprecated("utiliser `coq_calloc` + `node_init_` + `drawable_init_`.")))
Drawable* Drawable_createImageGeneral(Node* const refOpt,
                          Texture* const tex, Mesh* const mesh,
                          float x, float y, float twoDxOpt, float twoDy, float x_margin,
                          flag_t flags, uint8_t node_place);
Drawable*  node_asDrawableOpt(Node* nd);
/// Voir `noderef_fastDestroyAndNull`.
void       drawableref_fastDestroyAndNull(Drawable** const drawableOptRef);
/// Vérifie si c'est un noeud drawable actif ou parant de drawable actif.
int        node_isDisplayActive(Node* const node);
/// Init (pour sous struct). Ne fait que setter les variables et méthodes.
void      drawable_init_(Drawable* d, Texture* tex, Mesh* mesh, float twoDxOpt, float twoDy, float xMargin);


/*-- Surface d'image (png) : tile d'un png... ------------------*/
/// Convenience constructor : création d'une image (sprite avec png). Le ratio w/h est le même que le png d'origine.
/// Ici, on passe l'id du png.
Drawable* Drawable_createImage(Node* refOpt, uint32_t pngId,
                               float x, float y, float twoDy, flag_t flags);
Drawable* Drawable_createImageWithFixedWidth(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDx, float twoDy, flag_t flags);
/// Convenience constructor : création d'une image (sprite avec png). Le ratio w/h est le même que le png d'origine.
/// Ici, on passe le nom du png.
Drawable* Drawable_createImageWithName(Node* refOpt, const char* pngName,
                          float x, float y, float twoDy, flag_t flags);
/// Cas particulier de `Drawable_createImage`. Ici, la tile est setter à la langue actuelle a l'ouverture.
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy, flag_t flags);
/// Convenience constructor : création d'une image (sprite) de couleur uni.
Drawable* Drawable_createColor(Node* refOpt, Vector4 color,
                               float x, float y, float twoDx, float twoDy);
/*-- Surface de String (localise ou mutable). ----------------------------------------*/
Drawable* Drawable_createString(Node* const refOpt, StringDrawable str,
                   float x, float y, float maxTwoDxOpt, float twoDy,
                   flag_t flags, uint8_t node_place);
/// Mise à jour d'une string `mutable`.
void      drawable_updateAsMutableString(Drawable* d, const char* new_c_str, bool forceRedraw);
/// Mise à jour d'une string `shared`/constante.
void      drawable_updateAsSharedString(Drawable* d, StringDrawable const str);
void      drawable_updatePngId(Drawable* d, uint32_t newPngId, bool updateDims);
void      drawable_updateTargetDims(Drawable* d, float newTwoDxOpt, float newTwoDy, float newXMargin);
// Convenience setters...
void      drawable_setTile(Drawable* d, uint32_t i, uint32_t j);
void      drawable_setTileI(Drawable* d, uint32_t i);
void      drawable_setTileJ(Drawable* d, uint32_t j);
void      drawable_setTileFull(Drawable *d, InstanceTile tile);
// Modifis supplémentaires sur le dernier drawable créé.
//extern Drawable* drawable_last_;
void      drawable_last_setTile(uint32_t i, uint32_t j);
void      drawable_last_setEmph(float emph);
void      drawable_last_setShowOptions(bool isHard, bool isPoping);
void      drawable_last_setColor(Vector4 color);


#pragma mark - Frames, drawable particulier pour "entourer" un autre drawable/node. -----------

typedef enum {
    frame_option_getSizesFromParent = 0x0001,  // S'ajuste aux w/h de son parent.
    frame_option_giveSizesToParent=   0x0002,  // Ajuste sont parent à ses dimensions.
    frame_option_horizotalBar =       0x0004,  // Frame "1D" horizontal.
    frame_option_verticalBar =        0x0008,  // Frame "1D" vertical.
} FrameOptions;
typedef struct _Frame Frame;
typedef struct _Frame {
    // Upcasting
    union {
        Node     n;
        Drawable d;
    };
    float    delta;   // Epaisseur du bord du cadre.
    float    inside;  // 0 -> border outside box, 1 -> border inside box.
    void     (*setDims)(Frame* f, Vector2 deltas);
} Frame;

/*-- Surface de "Frame". Un cadre ajustable. Typiquement autour d'un autre noeud. -----------*/
Frame* Frame_createWithName(Node* const refOpt, float inside, float delta, 
                    float twoDxOpt, float twoDyOpt, const char* pngName, uint16_t options);
Frame* Frame_create(Node* refOpt, float inside, float delta,
                    float twoDxOpt, float twoDyOpt, uint32_t pngId, uint16_t options);

void   node_tryUpdatingAsFrameOfBro(Node* nodeOpt, Node* broOpt);

/*-- Surface pour visualiser les dimension d'un noeud. (debug only) --*/
void   node_tryToAddTestFrame(Node* ref);
void   node_last_tryToAddTestFrame(void);

// Deinit de drawable (pour sub-struct)
void drawable_deinit_(Node* nd);

#endif /* node_surface_h */
