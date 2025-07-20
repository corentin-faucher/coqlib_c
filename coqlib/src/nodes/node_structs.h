//
//  node_structs.h
//  Ajout de petites structures de noeuds pratiques...
//
//  Created by Corentin Faucher on 2023-11-02.
//

#ifndef coq_node_structs_h
#define coq_node_structs_h

#include "node_string.h"

// Utilise InstanceUniform -> extra1 pour l'emphase...
// (Superflu finalement)
//void node_last_addIcon(uint32_t diskPngId, uint32_t diskTile,
//                       uint32_t iconPngId, uint32_t iconTile);
//void node_last_addIconSingle(uint32_t iconPngId, uint32_t iconTile);
//void node_last_addIconLanguage(uint32_t pngId);

typedef struct {
    /// Largeur du cadre, e.g. 0.2 -> 0.2*h.
    float   frame_ratio;
    /// inside == 0 -> le cadre est à l'extérieur des lettres.
    float   frame_inside;
    /// Pas de spilling -> Le frame est dans les dimension a priori.
    /// Si spilling -> le frame fait grossing les dimension a priori.
    bool    frame_isSpilling;
    /// On met à jours les dimension du parent.
    bool    updateParentSizes;
} FramedStringParams;

/// Paramètres par défaut pour une string avec cadre :
/// Cadre 20% de height ; inside = 0% (cadre à l'extérieur des lettres) ;
/// Pas de spiling -> Le frame est dans les dimension a priori ;
/// On met à jours les dimension du parent ;
/// Marge en x 50% de height ; Text noir.
extern const FramedStringParams framedString_defPars;

/// Ajoute un frame et string (string encadrée) au noeud.
/// Voir `FramedStringParams` pour les options.
/// Retourne la string créé.
NodeString* node_addFramedString(Node* n, uint32_t framePngId, StringGlyphedInit str,
                          FramedStringParams params);

/// Ajoute au dernier noeud créé une frame et string.
/// On a donc last->{..., frame, string}. Voir `node_addFramedString`.
void node_last_addFramedString(uint32_t framePngId, StringGlyphedInit str,
                               FramedStringParams params);

/// Petite structure avec un array de string dans un encadré.
/// parent -> node -> {frame, strRoot-> {str1, str2,...}}
void Node_createFramedMultiString(Node* parent, uint32_t framePngId, StringGlyphedInit* str_arr, uint32_t str_count,
                                  float x, float y, float twoDxOpt, float strHeight,
                                  FramedStringParams params);

#endif /* node_structs_h */
