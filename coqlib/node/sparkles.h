//
//  sparkles.h
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-04.
//

#ifndef sparkles_h
#define sparkles_h

#include "view.h"
#include "graph_texture.h"

void Sparkle_init(View* frontView, const char* pngName, uint32_t soundId);
void Sparkle_setPng(uint32_t pngId);
void Sparkle_spawnAt(float xabs, float yabs, float delta, Texture* texOpt);
void Sparkle_spawnOver(Node* nd, float deltaRatio);


#endif /* sparkles_h */
