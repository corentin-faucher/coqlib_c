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
    Drawable* surf = Drawable_createImage(nd, diskPngId, 0, 0, height, 0, 0);
    drawable_setTile(surf, diskTile, 0);
    surf->n.piu.emph = 0.1f;
    surf = Drawable_createImage(nd, iconPngId, 0, 0, height, 0, 0);
    drawable_setTile(surf, iconTile, 0);
}
void node_last_addIconSingle(uint32_t iconPngId, uint32_t iconTile) {
    Node* nd = _node_last_created;
    if(nd == NULL) { printerror("No last node."); return; }
    float height = nd->h;
    Drawable* d = Drawable_createImage(nd, iconPngId, 0, 0, height, 0, 0);
    drawable_setTile(d, iconTile, 0);
    d->n.piu.emph = 0.1f;
}
void node_last_addImageLanguage(uint32_t pngId) {
    Node* nd = _node_last_created;
    if(nd == NULL) {
        printerror("No last node."); return;
    }
    float height = nd->h;
    Drawable* d = Drawable_createImageLanguage(nd, pngId, 0, 0, height, 0, 0);
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
    Frame_create(nd, frame_inside, delta, 0, 0, framePngId, frametype_giveSizesToParent);
    Texture* stringTex = Texture_createString(str);
    Drawable* string = _Drawable_create(nd, 0, 0, flag_giveSizeToBigbroFrame, 0,
        stringTex, mesh_sprite);
    string->n.piu.color = color4_black;
    string->x_margin = 0.5;
    drawable_updateDimsWithDeltas(string, strW, strH);
}
