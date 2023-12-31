//
//  node_structs.h
//  Ajout de petites structures de noeuds pratiques...
//
//  Created by Corentin Faucher on 2023-11-02.
//

#ifndef _coq_node_structs_h
#define _coq_node_structs_h

#include "_node_drawable.h"

void node_last_addIcon(uint32_t diskPngId, uint32_t diskTile,
                       uint32_t iconPngId, uint32_t iconTile);
void node_last_addIconSingle(uint32_t iconPngId, uint32_t iconTile);
void node_last_addIconLanguage(uint32_t pngId);

typedef struct {
    /// Largeur du cadre, e.g. 0.2 -> 0.2*h.
    float   frame_ratio;
    /// inside == 0 -> le cadre est à l'extérieur des lettres.
    float   frame_inside;
    /// Pas de spiling -> Le frame est dans les dimension a priori.
    bool    frame_isSpilling;
    /// On met à jours les dimension du parent.
    bool    updateParentSizes;
    /// Marge en x (0.5*h).
    float   x_margin;
    /// Text noir par défaut.
    Vector4 text_color;
} FramedStringParams;

/// Paramètres par défaut pour une string avec cadre :
/// Cadre 20% de height ; inside = 0% (cadre à l'extérieur des lettres) ;
/// Pas de spiling -> Le frame est dans les dimension a priori ;
/// On met à jours les dimension du parent ;
/// Marge en x 50% de height ; Text noir.
extern const FramedStringParams framedString_defPars;

/// Ajoute un frame et string (string encadrée) au noeud.
/// Voir `FramedStringParams` pour les options.
/// Retourne le drawable string créé.
Drawable* node_addFramedString(Node* n, uint32_t framePngId, Texture* strTex,
                          FramedStringParams params);

/// Ajoute au dernier noeud créé une frame et string.
/// On a donc last->{..., frame, string}. Voir `node_addFramedString`.
void node_last_addFramedString(uint32_t framePngId, Texture* strTex,
                               FramedStringParams params);


#endif /* node_structs_h */
