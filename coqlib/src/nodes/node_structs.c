//
//  node_structs.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-02.
//

#include "nodes/node_structs.h"
#include "graphs/graph_colors.h"

void node_last_addIcon(uint32_t diskPngId, uint32_t diskTile,
                       uint32_t iconPngId, uint32_t iconTile) {
    Node* nd = node_last_nonLeaf;
    if(nd == NULL) {
        printerror("No last node."); return;
    }
    float height = nd->h;
    
    Drawable* d = Drawable_createAndSetDims(nd, 0.f, 0.f, 0.f, height,
                         Texture_sharedImage(diskPngId), mesh_sprite, 0, 0);
    drawable_setTile(d, diskTile, 0);
    d->n.piu.emph = 0.1f;
    d = Drawable_createAndSetDims(nd, 0.f, 0.f, 0.f, height,
                                  Texture_sharedImage(iconPngId), mesh_sprite, 0, 0);
    drawable_setTile(d, iconTile, 0);
}
void node_last_addIconSingle(uint32_t iconPngId, uint32_t iconTile) {
    Node* nd = node_last_nonLeaf;
    if(nd == NULL) { printerror("No last node."); return; }
    float height = nd->h;
    Drawable* d = Drawable_createAndSetDims(nd, 0.f, 0.f, 0.f, height,
                     Texture_sharedImage(iconPngId), mesh_sprite, 0, 0);
    drawable_setTile(d, iconTile, 0);
    d->n.piu.emph = 0.1f;
}
void node_last_addIconLanguage(uint32_t pngId) {
    Node* nd = node_last_nonLeaf;
    if(nd == NULL) {
        printerror("No last node."); return;
    }
    float height = nd->h;
    Drawable* d = Drawable_createImageLanguage(nd, pngId, 0, 0, height, 0);
    d->n.piu.emph = 0.1f;
}

const FramedStringParams framedString_defPars = {
    0.2f,  // Largeur du cadre (0.2*h).
    0.f,   // inside == 0 -> le cadre est à l'extérieur des lettres.
    false, // Pas de spiling -> Le frame est dans les dimension a priori.
    true,  // On met à jours les dimension du parent.
    0.5,   // Marge en x (0.5*h).
    {{0, 0, 0, 1 }}, // Text noir par défaut.
};

/// Ajoute un frame et string (string encadrée) au noeud.
/// Voir `FramedStringParams` pour les options.
/// Retourne le drawable string créé.
Drawable* node_addFramedString(Node* n, uint32_t framePngId, Texture* strTex,
                          FramedStringParams params) {
    float strH;
    float delta;
    float strW;
    // Ici on veut une frame avec les dimensions "target",
    // donc on ajuste les dimension de la string en consequences.
    if(params.frame_isSpilling) {
        strH = n->h;
        delta = params.frame_ratio * strH;
        strW = n->w;
    } else {
        strH = n->h / (1.f + 2.f*params.frame_ratio*(1.f-params.frame_inside));
        delta = params.frame_ratio * strH;
        strW = n->w - 2.f*delta*(1.f - params.frame_inside);
    }
    Frame_create(n, params.frame_inside, delta, 0, 0, framePngId,
                 params.updateParentSizes ? frame_option_giveSizesToParent : 0);
    Drawable *d = Drawable_create(n, strTex, mesh_sprite,
                                  flag_giveSizeToBigbroFrame, 0);
    d->n.piu.color = params.text_color;
    d->x_margin = params.x_margin;
    drawable_updateDimsWithDeltas(d, strW, strH);
    return d;
}

/// Ajoute au dernier noeud créé une frame et string.
/// On donc last->{..., frame, string}. Voir `node_addFramedString`.
void node_last_addFramedString(uint32_t framePngId, Texture* strTex,
                               FramedStringParams params) {
    Node* nd = node_last_nonLeaf;
    if(nd == NULL) {
        printerror("No last node."); return;
    }
    node_addFramedString(nd, framePngId, strTex, params);
}
