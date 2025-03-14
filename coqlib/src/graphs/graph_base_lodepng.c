//
//  graph_base_sdl.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-31.
//
#include "graph_base.h"
#include "../utils/util_base.h"
#include "../lodepng.h"

// MARK: - Dessin de png
PixelBGRAArray* Pixels_engine_createFromPngOpt(const char*const pngPathOpt, bool const showError) 
{
    if(!pngPathOpt) return NULL;
    unsigned char* pixels;
    unsigned int width, height;
    unsigned error = lodepng_decode32_file(&pixels, &width, &height, pngPathOpt);
    if(error || !pixels) {
        if(showError) printerror(" Png load error %u: %s, %s.", 
            error, lodepng_error_text(error), pngPathOpt);
        return NULL;
    } 
    size_t const pixelCount = width * height;
    PixelBGRAArray *pa = coq_callocArray(PixelBGRAArray, PixelBGRA, pixelCount);
    pa->width = width; pa->height = height;
    pa->solidWidth = width;
    memcpy(pa->pixels, pixels, pixelCount * sizeof(PixelBGRA));
    free(pixels);
    return pa;
}

