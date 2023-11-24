//
//  node_structs.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-02.
//

#ifndef node_structs_h
#define node_structs_h

#include "drawable.h"

void node_last_addIcon(uint32_t diskPngId, uint32_t diskTile,
                       uint32_t iconPngId, uint32_t iconTile);
void node_last_addIconSingle(uint32_t iconPngId, uint32_t iconTile);
void node_last_addIconLanguage(uint32_t pngId);
void node_last_addFramedString(uint32_t framePngId, UnownedString str, float maxWidth,
                               float frame_ratio, float frame_inside, Bool frame_isSpilling);


#endif /* node_structs_h */
