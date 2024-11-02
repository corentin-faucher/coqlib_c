//
//  Drawables : les noeuds feuilles étant affichés,
//  i.e. avec mesh et texture.
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef coq_node_drawable_h
#define coq_node_drawable_h

#include "node_base.h"
#include "../maths/math_smtrans.h"
#include "../graphs/graph_texture.h"
#include "../graphs/graph_mesh.h"

/// Noeud affichable (image, string).
/// Pour un drawable les dimensions sont données par scaleX et scaleY.
/// Un drawable est un noeud feuille ou w et h sont fixés à 1 a priori.
/// (w, h peuvent être différent de 1 pour avoir une hitbox différente de la sprite affichée,
///  voir `DrawableChar` comme exemple.)
typedef struct Coq_Drawable {
    Node     n;  // Upcasting as Node.
    
    /// La texture attaché. Peut être owned ou shared.
    /// C'est plus ou moins `privé`... On ne devrait pas avoir besoin d'y toucher.
    /// (Les texture doivent être `release` quand on en a plus besoin...
    ///  Drawable s'occupe de release sa texture lors du deinit.) 
    Texture* _tex;
    /// La mesh attaché. Peut être owned ou shared.
    /// Ici aussi c'est plus ou moins privé, et le deinit de Drawable libère la mesh si owned.
    Mesh*    _mesh;
    /// Smooth transition pour affichage ON/OFF.
    SmTrans  trShow;
    /// Transition en extra ! e.g. emphasis, flip, etc.
    SmTrans  trExtra;
} Drawable;

/// Init (ne fait que setter les variables et méthodes de Drawable)
/// A priori, utiliser les constructeur `Drawable_create...`.
/// Un drawable est un noeud feuille ou w et h sont fixés à 1 a priori. (Largeur et hauteur sont settés avec sx / sy.)
void      drawable_init(Drawable* d, Texture* tex, Mesh* mesh, float twoDxOpt, float twoDy);
/// Deinit de drawable (pour sub-struct)
void      drawable_deinit_(Node* nd);
/// Voir `noderef_fastDestroyAndNull`.
void      drawableref_destroyAndNull(Drawable** const drawableOptRef);
/// Downcasting node as drawable.
Drawable* node_asDrawableOpt(Node* nOpt);
void      drawable_changeMeshTo(Drawable* d, Mesh* newMesh);

/// La fonction utilisée pour mettre à jour la matrice modèle avant l'affichage.
/// (Peut être remplacé par un fonction custom)
extern void (*Drawable_renderer_defaultUpdateInstanceUniforms)(Node*);


#pragma mark -- Surface d'image (png) : tile d'un png... -----------------
/// Convenience constructor : création d'une image (sprite avec png). 
/// Le ratio w/h est le même que le png d'origine.
/// Ici, on passe l'id du png.
Drawable* Drawable_createImage(Node* refOpt, uint32_t pngId,
                               float x, float y, float twoDy, flag_t flags);
Drawable* Drawable_createImageWithWidth(Node* const refOpt, uint32_t pngId,
                               float x, float y, float twoDx, float twoDy, flag_t flags);
/// Convenience constructor : création d'une image (sprite avec png).
/// Le ratio w/h est le même que le png d'origine.
/// Ici, on passe le nom du png.
Drawable* Drawable_createImageWithName(Node* refOpt, const char* pngName,
                          float x, float y, float twoDy, flag_t flags);
/// Cas particulier de `Drawable_createImage`. Ici, la tile est setter à la langue actuelle a l'ouverture.
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy, flag_t flags);
/// Convenience constructor : création d'une image (sprite) de couleur uni.
Drawable* Drawable_createColor(Node* refOpt, Vector4 color,
                               float x, float y, float twoDx, float twoDy);
/// Convenience constructor : image de `coqlib_test_frame.png` pour testing.                   
Drawable* Drawable_createTestFrame(Node* parent, float x, float y, float twoDx, float twoDy);

/// Change la texture présente pour la texture d'un autre png.
/// Il faut probablement ajuster le rectangle UV ou le ratio de l'image ensuite -> `drawable_setTile`,
/// `drawable_checkRatioWithUVrectAndTexture`, ...
void      drawable_changeTexToPngId(Drawable* d, uint32_t const newPngId);


#pragma mark -- Setters ------------
//void      drawable_updateTargetDims(Drawable* d, float newTwoDxOpt, float newTwoDy, float newXMargin);
// Convenience setters...
/// Met à jour le uvRect pour être la tile (i,j) de la texture présente (avec m x n subdivisions).
void      drawable_setTile(Drawable* d, uint32_t i, uint32_t j);
/// Ne fait que mettre à jour `uvRect.o_x` (ne vérifie pas le reste du uvRect).
void      drawable_setTileI(Drawable* d, uint32_t i);
/// Ne fait que mettre à jour `uvRect.o_y` (ne vérifie pas le reste du uvRect).
void      drawable_setTileJ(Drawable* d, uint32_t j);
/// Set le rectangle uv. (on peut aussi le setter directement...)
void      drawable_setUVRect(Drawable* d, Rectangle const uvrect);
/// Met à jour le scaling en x `n.sx` pour respecter le ratio w/h de la texture et du rectangle UV.
/// On peut en profiter pour mettre à jour la dimension (sy = 2Dy).
void      drawable_checkRatioWithUVrectAndTexture(Drawable* d, float newTwoDyOpt);
// Modifs supplémentaires sur le dernier drawable créé.
//extern Drawable* drawable_last_;
void      drawable_last_setTile(uint32_t i, uint32_t j);
void      drawable_last_setExtra1(float emph); // Superflu ?
void      drawable_last_setShowOptions(bool isHard, bool isPoping);
void      drawable_last_setColor(Vector4 color);


#pragma mark - Frames, drawable particulier pour "entourer" un autre drawable/node. -----------
typedef enum {
    frame_option_getSizesFromParent = 0x0001,  // S'ajuste aux w/h de son parent.
    frame_option_giveSizesToParent=   0x0002,  // Ajuste le parent avec ses dimensions.
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
Frame* Frame_create(Node* const refOpt, float inside, float delta, 
                    float twoDxOpt, float twoDyOpt, Texture* tex, uint16_t options);

void   node_tryUpdatingAsFrameOfBro(Node* nodeOpt, Node* broOpt);

/*-- Surface pour visualiser les dimension d'un noeud. (debug only) --*/
void   node_tryToAddTestFrame(Node* ref);
void   node_last_tryToAddTestFrame(void);



#endif /* node_surface_h */

// Garbage
//#pragma mark -- Obsolete ? Surface de String -----------------------------------
//// ** Utiliser plutôt NodeString ??...
//
//Drawable* Drawable_createString(Node* const refOpt, StringDrawable str,
//                   float x, float y, float maxTwoDxOpt, float twoDy,
//                   flag_t flags, uint8_t node_place);
///// Mise à jour d'une string `mutable`.
//void      drawable_updateAsMutableString(Drawable* d, const char* new_c_str, bool forceRedraw);
///// Mise à jour d'une string `shared`/constante.
//void      drawable_updateAsSharedString(Drawable* d, StringDrawable const str);
