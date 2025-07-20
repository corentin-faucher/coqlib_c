//
//  graph_base_sdl.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-31.
//
#include "graph_base.h"

#include "../utils/util_base.h"
#include "lodepng.h"

// MARK: - Dessin de png
PixelBGRAArray* PixelBGRAArray_engine_createFromPngFileOpt(const char*const pngPath, bool const showError) 
{
    if(!pngPath) { printerror("No png to load."); return NULL; }
    unsigned char* pixels;
    unsigned int width, height;
    unsigned error = lodepng_decode32_file(&pixels, &width, &height, pngPath);
    if(error || !pixels) {
        if(showError) printerror(" Png load error %u: %s, %s.", 
            error, lodepng_error_text(error), pngPath);
        return NULL;
    } 
    size_t const pixelCount = width * height;
    PixelBGRAArray *pa = PixelBGRAArray_createEmpty(width, height);
    memcpy(pa->pixels, pixels, pixelCount * sizeof(PixelBGRA));
    free(pixels);
    return pa;
}

