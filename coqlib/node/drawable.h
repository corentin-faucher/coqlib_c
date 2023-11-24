//
//  node_surface.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#ifndef node_surface_h
#define node_surface_h

#include <stdio.h>
#include "_node.h"
#include "smtrans.h"
#include "graph_texture.h"
#include "graph_mesh.h"


/// Noeud affichable (image, string)
typedef struct _Drawable {
    Node     n;        // Peut être casté comme un noeud.
    Texture* tex;      // La texture attaché. Peut être owned ou shared.
    Mesh*    mesh;     // La mesh attaché. Peut être owned ou shared.
    SmTrans  trShow;   // Smooth transition pour affichage ON/OFF.
    SmTrans  trExtra;  // Transition en extra ! e.g. emphasis, flip, etc.
    float    x_margin; // Marge en x pour les textures de string. (sub-struct ?)
} Drawable;

// Init de base, set le deinit.
void      drawable_init(Drawable* d, Texture* tex, Mesh* mesh);
// Constructeur de base.
Drawable* Drawable_create(Node* const refOpt,
                          Texture* const tex, Mesh* const mesh,
                          flag_t flags, uint8_t node_place);
Drawable* Drawable_createAndSetDims(Node* const refOpt,
                                    float x, float y, float twoDx, float twoDy,
                                    Texture* const tex, Mesh* const mesh,
                                    flag_t flags, uint8_t node_place);
/// Downcasting
Drawable* node_asDrawableOpt(Node* nd);
Drawable* node_defaultUpdateModelAndGetAsDrawableOpt(Node* node);


/*-- Convenience constructors... ------------------------------------------*/

/*-- Surface d'image (png), peut etre une tile du png... ------------------*/
/// Création d'une image (sprite avec png). Le ratio w/h est le même que le png d'origine.
Drawable* Drawable_createImage(Node* const refOpt, uint pngId,
                               float x, float y, float twoDy, flag_t flags);
/// Création d'une image (sprite avec png). Le ratio w/h est le même que le png d'origine.
Drawable* Drawable_createImageWithName(Node* const refOpt, const char* pngName,
                          float x, float y, float twoDy, flag_t flags);
/// Cas particulier de Drawable_createImage.
/// Création d'une image où la tile est setter à la langue actuelle a l'ouverture.
Drawable* Drawable_createImageLanguage(Node* refOpt, uint32_t pngId,
                                       float x, float y, float twoDy, flag_t flags);
/*-- Surface de String constante. ---------------------------------------------------*/
Drawable* Drawable_createConstantString(Node* const refOpt, const char* c_str,
                              float x, float y, float maxTwoDxOpt, float twoDy,
                              flag_t flags);
/*-- Surface de String (localise ou mutable). ----------------------------------------*/
Drawable* Drawable_createString(Node* const refOpt, UnownedString str,
                   float x, float y, float maxTwoDxOpt, float twoDy,
                   flag_t flags);
/// Mise à jours des dimensions du drawable.
/// Si twoDx (width) == 0 -> la larger se base sur le ratio w/h du png/string d'origine.
void      drawable_updateDimsWithDeltas(Drawable *surf, float twoDxOpt, float twoDy);
void      drawable_setTile(Drawable *surf, uint i, uint j);
void      drawable_setTileI(Drawable *surf, uint i);
void      drawable_setTileJ(Drawable *surf, uint j);

/*-- Frames, drawable particulier pour "entourer" un autre drawable/node. ------------------*/
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
Frame* Frame_createWithTex(Node* const refOpt, float inside,
                           float delta, float twoDxOpt, float twoDyOpt,
                           Texture* tex, uint16_t options);
Frame* Frame_create(Node* const refOpt, float inside, float delta,
                    float twoDxOpt, float twoDyOpt, uint pngId, uint16_t options);

void   node_tryUpdatingAsFrameOfBro(Node* nodeOpt, Node* broOpt);

/*-- Surface pour visualiser les dimension d'un noeud. (debug only) --*/
void   node_tryToAddTestFrame(Node* ref);
void   node_last_tryToAddTestFrame(void);

// Deinit de drawable (free mesh ou texture si owner).
void _drawable_deinit_freeMesh(Node* nd);
void _drawable_deinit_freeTexture(Node* nd);
void _drawable_deinit_freeTextureAndMesh(Node* nd);


#endif /* node_surface_h */
