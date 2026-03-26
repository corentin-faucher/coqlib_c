//
//  _graph_.c
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "graph_base.h"

#include "../external/lodepng.h"
#define NANOSVG_IMPLEMENTATION
//#define NANOSVG_ALL_COLOR_KEYWORDS
#define NANOSVGRAST_IMPLEMENTATION
#include "../external/nanosvgrast.h"
#include "../utils/util_base.h"
#include "../systems/system_file.h"

// MARK: - Image de pixels

PixelArray* PixelArray_createEmpty(size_t const width, size_t const height) {
    if(!width || !height) { printerror("Missing dims."); return NULL; }
    PixelArray* pa = coq_callocArray(PixelArray, PixelRGBA, width * height);
    size_initConst(&pa->width, width);
    size_initConst(&pa->height, height);
    // Dimensions "solides" a priori...
    pa->solidWidth = width;
    pa->solidHeight = height;
    return pa;
}
PixelArray* PixelArray_createSubRegion(PixelArray const*const src, RectangleUint const region) {
    if(!src) { printerror("No src."); return NULL; }
    if((region.o_x + region.w > (uint32_t)src->width) ||
       (region.o_y + region.h > (uint32_t)src->height)) {
        printerror("Overflow"); return NULL;
    }
    PixelArray* dst = PixelArray_createEmpty(region.w, region.h);
    // Pixels sous forme array 2D
    PixelRGBA (*const srcPix)[src->width] = (PixelRGBA (*)[src->width])src->pixels;
    PixelRGBA (*const dstPix)[dst->width] = (PixelRGBA (*)[dst->width])dst->pixels;
    for(uint32_t j = 0; j < region.h; j++) {
        PixelRGBA*       p_dst =    &dstPix[j][0];
        PixelRGBA*const  p_dst_end =&dstPix[j][dst->width]; 
        PixelRGBA const* p_src =    &srcPix[j+region.o_y][region.o_x];
        for(; p_dst < p_dst_end; p_dst++, p_src++) {
            *p_dst = *p_src;
        }
    }
    return dst;
}
void pixelarray_copyAt(PixelArray const*const src, PixelArray*const dst, UintPair const dstOrigin) {
    if(!src || !dst) { printerror("No src or dst."); return; }
    if((dstOrigin.uint0 + src->width >  (uint32_t)dst->width) ||
       (dstOrigin.uint1 + src->height > (uint32_t)dst->height)) {
        printerror("Overflow"); return;
    }
    // Pixels sous forme array 2D. Ici on copie toute la source et on se place à dstOrigin dans la destination.
    PixelRGBA (*const srcPix)[src->width] = (PixelRGBA (*)[src->width])src->pixels;
    PixelRGBA (*const dstPix)[dst->width] = (PixelRGBA (*)[dst->width])dst->pixels;
    for(uint32_t j = 0; j < src->height; j++) {
        PixelRGBA*       p_dst =     &dstPix[j+dstOrigin.uint1][dstOrigin.uint0];
        PixelRGBA const* p_src =     &srcPix[j][0];
        PixelRGBA const* p_src_end = &srcPix[j][src->width];
        for(; p_src < p_src_end; p_src++, p_dst++) {
            *p_dst = *p_src;
        }
    }
}
void pixelarray_copyRegionAt(PixelArray const*const src, RectangleUint const srcRegion,
                                 PixelArray*const dst, UintPair const dstOrigin) {
    if(!src || !dst) { printerror("No src or dst."); return; }
    if((dstOrigin.uint0 + srcRegion.w > (uint32_t)dst->width) ||
       (dstOrigin.uint1 + srcRegion.h > (uint32_t)dst->height) ||
       (srcRegion.o_x + srcRegion.w > (uint32_t)src->width) ||
       (srcRegion.o_y + srcRegion.h > (uint32_t)src->height)) {
        printerror("Overflow"); return;
    }
    // Pixels sous forme array 2D. Ici on copie toute la source et on se place à dstOrigin dans la destination.
    PixelRGBA (*const srcPix)[src->width] = (PixelRGBA (*)[src->width])src->pixels;
    PixelRGBA (*const dstPix)[dst->width] = (PixelRGBA (*)[dst->width])dst->pixels;
    for(uint32_t j = 0; j < srcRegion.h; j++) {
        PixelRGBA*       p_dst =     &dstPix[j+dstOrigin.uint1][dstOrigin.uint0];
        PixelRGBA const* p_src =     &srcPix[j+srcRegion.o_y][srcRegion.o_x];
        PixelRGBA const* p_src_end = &srcPix[j+srcRegion.o_y][srcRegion.o_x+srcRegion.w];
        for(; p_src < p_src_end; p_src++, p_dst++) {
            *p_dst = *p_src;
        }
    }                            
}


// MARK: - Lecture de fichiers d'images.

// MARK: Bitmap... voir http://en.wikipedia.org/wiki/Windows_bitmap
struct __attribute__((packed)) BMHeader_ {
    uint16_t BM;                // 0, La signature "BM" -> 0x4D42 en hexadec.
    uint32_t file_size;         // 2 ** Ici, pour forcer l'aligment compact des données,
    uint32_t _reserved0;        // 6        il faut utiliser `__attribute__((packed))`.
    uint32_t pixel_offset;      // 10
    uint32_t windowsHeaderSize; // 14
    int32_t  width;             // 18
    int32_t  height;            // 22
    uint16_t colorPlane_count;  // 26
    uint16_t bitsPerPixels;     // 28
};
PixelArray* PixelArray_createFromBitmapFileOpt(const char*const path, bool flipY) {
    if(!path) { printerror("No path to open."); return NULL; }
    guard_let(FILE*, bmpFile, fopen(path, "rb"), printerror("Cannot open %s.", path), NULL)
    PixelArray* pa = NULL;
    struct BMHeader_ header;
    fread(&header, sizeof(struct BMHeader_), 1, bmpFile);
    // S'assurer d'avoir des dimension positives...
    if(header.width < 0) {
        header.width = -header.width; // Besoin de flip l'axe des x ?? (non a priori)
    }
    if(header.height < 0) {
        header.height = -header.height;
        flipY = -flipY;
    }
    // Header check
    if(header.BM != 0x4D42) {
        printerror("No 'BM' in bitmap header, found %#06x.", header.BM); 
        goto close_file;
    }
    if(header.width == 0 || header.height == 0 || header.width > 4096 || header.height > 4096) {
        printerror("Bad bitmap dimensions, width %d, height %d.", header.width, header.height); 
        goto close_file;
    }
    {
    // Ok, création d'un array de pixels.
    pa = PixelArray_createEmpty(header.width, header.height);
    // Itérateur des lignes en sortie.
    PixelRGBA (*pixLineOut)[header.width] = (PixelRGBA (*)[header.width])pa->pixels;
    if(flipY) pixLineOut += header.height - 1; // (se placer à la fin si flip upside-down)
    // Se placer pour les pixels du bmp.
    fseek(bmpFile, header.pixel_offset, SEEK_SET);
    // Lecture des lignes du bmp.
    if(header.bitsPerPixels == 24) { // Cas Bmp en BGR.
        // Taille d'une ligne du bmp en bytes (multiple de 32 bits)
        size_t const line_size = 4*(size_t)ceilf(((float)header.width * header.bitsPerPixels) / 32.f);
        for(int32_t j = 0; j < header.height; j ++) {
            // Charger une ligne
            uint8_t line[line_size];
            size_t const readCount = fread(line, line_size, 1, bmpFile);
            if(readCount < 1) {
                printerror("Cannot read line of pixels.");
                goto close_file;
            }
            // Pixels du bitmap en BGR
            PixelRGB const*      pBmp =     (PixelRGB const*) line;
            PixelRGB const*const pBmp_end = (PixelRGB const*)&line[header.width * 3];
            // Pixel de sortie (en bgra)
            PixelRGBA *p = (PixelRGBA*)pixLineOut;
            for(; pBmp < pBmp_end; pBmp++, p++) {
                *p = (PixelRGBA) {
                    .b = pBmp->r, .g = pBmp->g, .r = pBmp->b,
                    .a =    255,
                };
            }
            if(flipY) pixLineOut --;
            else      pixLineOut ++;
        }
        goto close_file;
    }
    if(header.bitsPerPixels == 32) { // Cas bmp en BGRA.
        size_t const line_size = sizeof(PixelRGBA) * header.width; // (4 bytes per pixel)
        for(int32_t j = 0; j < header.height; j ++) {
            // Charger une ligne
            PixelRGBA line[header.width]; // buffer d'une ligne
            size_t const readCount = fread(line, line_size, 1, bmpFile);
            if(readCount < 1) {
                printerror("Cannot read line of pixels.");
                goto close_file;
            }
            const PixelRGBA *pBmp =            line;
            const PixelRGBA *const pBmp_end = &line[header.width];
            PixelRGBA *p = (PixelRGBA*)pixLineOut;
            for(; pBmp < pBmp_end; pBmp++, p++) {
                *p = (PixelRGBA) {
                    .b = pBmp->r, .g = pBmp->g, .r = pBmp->b, .a = pBmp->a
                };
            }
            if(flipY) pixLineOut --;
            else      pixLineOut ++;
        }
        goto close_file;
    }
    }
    printerror("Unsupported bitsPerPixels %d.", header.bitsPerPixels);
close_file:
    fclose(bmpFile);
    return pa;
}

// MARK: PNG (Portable Network Graphics), image compressée avec transparence.
PixelArray* PixelArray_createFromPngFileOpt(const char*const pngPath, bool const showError) 
{
    if(!pngPath) { if(showError) printerror("No png to load."); return NULL; }
    unsigned char* pixels;
    unsigned int width, height;
    unsigned error = lodepng_decode32_file(&pixels, &width, &height, pngPath);
    if(error || !pixels) {
        if(showError) printerror(" Png load error %u: %s, %s.", 
            error, lodepng_error_text(error), pngPath);
        return NULL;
    } 
    size_t const pixelCount = width * height;
    PixelArray *pa = PixelArray_createEmpty(width, height);
    memcpy(pa->pixels, pixels, pixelCount * sizeof(PixelRGBA));
    free(pixels);
    return pa;
}

// MARK: SVG (Scalable Vector Graphics), image vectorielle.
PixelArray* PixelArray_createFromSvgFileOpt(const char*const svgPath, 
                    size_t const height, bool const showError) 
{
    if(!svgPath) { if(showError) printerror("No png to load."); return NULL; }
    
    guard_let(NSVGimage*, svgImage, nsvgParseFromFile(svgPath, "px", 32),
              printerror("Cannot load SVG at %s.", svgPath), NULL)
    printdebug("Svg %d x %d.", (int)svgImage->width, (int)svgImage->height);
    size_t const width = ((float)height * svgImage->width) / svgImage->height;
    float const scale = height / svgImage->height;
    guard_let(PixelArray*, pa, PixelArray_createEmpty(width, height), , NULL)
    
	// Rasterization
    struct NSVGrasterizer* svgRasterizer = nsvgCreateRasterizer();   
    nsvgRasterize(svgRasterizer, svgImage, 0, 0, scale, 
                  (uint8_t*)pa->pixels, (int)width, (int)height, (int)width * 4);
    nsvgDeleteRasterizer(svgRasterizer);
    
    return pa;
}

