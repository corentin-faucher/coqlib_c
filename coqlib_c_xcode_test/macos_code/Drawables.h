//
//  Drawables.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#ifndef Drawables_h
#define Drawables_h

#include "graph.h"

typedef enum {
    png_the_cat,
    png_total_pngs
} Drawables;

static const PngInfo MyProject_pngInfos[] = {
    {"the_cat", 1, 1},
};

#endif /* Drawables_h */
