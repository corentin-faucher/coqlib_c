//
//  sparkles.h
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-04.
//

#ifndef _coq_node_sparkles_h
#define _coq_node_sparkles_h

#include "_node_view.h"
#include "_graph/_graph_texture.h"

/// Init pour les "feux d'artifices". S'il n'y a pas de pngName, on prend `coqlib_sparkle_stars`.
void Sparkle_init(View* frontView, const char* pngNameOpt, uint32_t soundId);
void Sparkle_setPng(uint32_t pngId);
void Sparkle_spawnAt(float xabs, float yabs, float delta, Texture* texOpt);
void Sparkle_spawnOver(Node* nd, float deltaRatio);


#endif /* sparkles_h */
