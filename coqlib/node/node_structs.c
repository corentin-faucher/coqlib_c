//
//  node_structs.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-02.
//

#include "node_structs.h"
#include "colors.h"

void node_last_addIcon(uint32_t diskPngId, uint32_t diskTile,
                       uint32_t iconPngId, uint32_t iconTile) {
    Node* nd = _node_last_created;
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
    Node* nd = _node_last_created;
    if(nd == NULL) { printerror("No last node."); return; }
    float height = nd->h;
    Drawable* d = Drawable_createAndSetDims(nd, 0.f, 0.f, 0.f, height,
                     Texture_sharedImage(iconPngId), mesh_sprite, 0, 0);
    drawable_setTile(d, iconTile, 0);
    d->n.piu.emph = 0.1f;
}
void node_last_addIconLanguage(uint32_t pngId) {
    Node* nd = _node_last_created;
    if(nd == NULL) {
        printerror("No last node."); return;
    }
    float height = nd->h;
    Drawable* d = Drawable_createImageLanguage(nd, pngId, 0, 0, height, 0);
    d->n.piu.emph = 0.1f;
}
void node_last_addFramedString(uint32_t framePngId, UnownedString str, float maxWidth,
                               float frame_ratio, float frame_inside, Bool frame_isSpilling) {
    Node* nd = _node_last_created;
    if(nd == NULL) {
        printerror("No last node."); return;
    }
    // Ici on veut une frame avec les dimensions "target",
    // donc on ajuste les dimension de la string en consequences.
    float strH;
    float delta;
    float strW;
    if(!frame_isSpilling) {
        strH = nd->h / (1.f + 2.f*frame_ratio*(1.f-frame_inside));
        delta = frame_ratio * strH;
        strW = maxWidth - 2.f*delta*(1.f-frame_inside);
    } else {
        strH = nd->h;
        delta = frame_ratio * strH;
        strW = maxWidth;
    }
    Frame_create(nd, frame_inside, delta, 0, 0, framePngId, frame_option_giveSizesToParent);
    Drawable *d = Drawable_create(nd, Texture_createString(str), mesh_sprite, flag_giveSizeToBigbroFrame, 0);
    d->n.piu.color = color4_black;
    d->x_margin = 0.5;
    drawable_updateDimsWithDeltas(d, strW, strH);
}
