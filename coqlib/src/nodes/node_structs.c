//
//  node_structs.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-02.
//

#include "node_structs.h"

#include "node_tree.h"
#include "../utils/util_base.h"

//#warning Superflu... A faire pour un projet particulier...
//void node_last_addIcon(uint32_t diskPngId, uint32_t diskTile,
//                       uint32_t iconPngId, uint32_t iconTile) {
//    Node* const nd = Node_last;
//    if(nd == NULL) { printerror("No last node."); return; }
//    float height = nd->h;
//    Drawable* d = Drawable_createImage(nd, diskPngId, 0, 0, height, 0);
//    drawable_setTile(d, diskTile, 0);
//    d->n._iu.extra1 = 0.1f;
//    d = Drawable_createImage(nd, iconPngId, 0.f, 0.f, height, 0);
//    drawable_setTile(d, iconTile, 0);
//}
//void node_last_addIconSingle(uint32_t iconPngId, uint32_t iconTile) {
//    Node* const nd = Node_last;
//    if(nd == NULL) { printerror("No last node."); return; }
//    float height = nd->h;
//    Drawable* d = Drawable_createImage(nd, iconPngId, 0.f, 0.f, height, 0);
//    drawable_setTile(d, iconTile, 0);
//    d->n._iu.extra1 = 0.1f;
//}
//void node_last_addIconLanguage(uint32_t pngId) {
//    Node* const nd = Node_last;
//    if(nd == NULL) { printerror("No last node."); return; }
//    float height = nd->h;
//    Drawable* d = Drawable_createImageLanguage(nd, pngId, 0, 0, height, 0);
//    d->n._iu.extra1 = 0.1f;
//}


const FramedStringParams framedString_defPars = {
    .frame_ratio = 0.2f,  // Largeur du cadre (0.2*h).
    .frame_inside = 0.0f,   // inside == 0 -> le cadre est à l'extérieur des lettres.
    .frame_isSpilling = false, // Pas de spiling -> Le frame est dans les dimension a priori.
    .updateParentSizes = true,  // On met à jours les dimension du parent.
};

/// Ajoute un frame et string (string encadrée) au noeud.
/// Voir `FramedStringParams` pour les options.
/// Retourne le drawable string créé.
NodeString* node_addFramedString(Node* n, uint32_t framePngId, StringGlyphedInit str,
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
        strW = fmaxf(0.f, n->w - 2.f*delta*(1.f - params.frame_inside));
    }
    Frame_create(n, params.frame_inside, delta, 0, 0,
                 Texture_sharedImage(framePngId),
                 params.updateParentSizes ? frame_option_giveSizesToParent : 0);
    return NodeString_create(n, str, 0, 0, strW, strH, flag_giveSizeToBigbroFrame, 0);
}
void Node_createFramedMultiString(Node* parent, uint32_t framePngId, StringGlyphedInit* str_arr, uint32_t str_count,
                                  float x, float y, float twoDxOpt, float strHeight,
                                  FramedStringParams params) {
    float delta = params.frame_ratio * strHeight;
    float strW = twoDxOpt;
    if(!params.frame_isSpilling && strW) {
        strW = fmaxf(0.f, strW - 2.f*delta*(1.f - params.frame_inside));
    }
    Node* n = Node_create(parent, x, y, 1, 1, 0, 0);
    Frame_create(n, params.frame_inside, delta, 0, 0, Texture_sharedImage(framePngId), frame_option_giveSizesToParent);
    Node* strsRoot = Node_create(n, 0, 0, 1, 1, flag_giveSizeToBigbroFrame, 0);
    for(uint32_t i = 0; i < str_count; i ++) {
        NodeString_create(strsRoot, str_arr[i], 0, 0, strW, strHeight, 0, 0);
    }
    node_tree_alignTheChildren(strsRoot, node_align_vertically, 1, 1);
}

/// Ajoute au dernier noeud créé une frame et string.
/// On donc last->{..., frame, string}. Voir `node_addFramedString`.
void node_last_addFramedString(uint32_t framePngId, StringGlyphedInit str,
                               FramedStringParams params) {
    Node* const nd = Node_last;
    if(nd == NULL) {
        printerror("No last node."); return;
    }
    node_addFramedString(nd, framePngId, str, params);
}
