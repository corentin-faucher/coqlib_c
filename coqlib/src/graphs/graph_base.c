//
//  _graph_.c
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "graph_base.h"

#include "../utils/util_base.h"
#include "../systems/system_file.h"

char*  FileManager_getPngPathOpt(const char* pngName, bool isCoqlib, bool isMini) 
{
    if(!pngName) { printerror("Texture without name."); return NULL; }
    const char* png_dir;
    if(isCoqlib) {
        png_dir = isMini ? "pngs_coqlib/minis" : "pngs_coqlib";
    } else {
        png_dir = isMini ? "pngs/minis" : "pngs";
    }
    return FileManager_getResourcePathOpt(pngName, "png", png_dir);
}

PixelBGRAArray* PixelBGRAArray_createEmpty(size_t const width, size_t const height) {
    if(!width || !height) { printerror("Missing dims."); return NULL; }
    PixelBGRAArray* pa = coq_callocArray(PixelBGRAArray, PixelBGRA, width * height);
    size_initConst(&pa->width, width);
    size_initConst(&pa->height, height);
    // Dimensions "solides" a priori...
    pa->solidWidth = width;
    pa->solidHeight = height;
    return pa;
}
PixelBGRAArray* PixelBGRAArray_createSubRegion(PixelBGRAArray const*const src, RectangleUint const region) {
    if(!src) { printerror("No src."); return NULL; }
    if((region.o_x + region.w > (uint32_t)src->width) ||
       (region.o_y + region.h > (uint32_t)src->height)) {
        printerror("Overflow"); return NULL;
    }
    PixelBGRAArray* dst = PixelBGRAArray_createEmpty(region.w, region.h);
    // Pixels sous forme array 2D
    PixelBGRA (*const srcPix)[src->width] = (PixelBGRA (*)[src->width])src->pixels;
    PixelBGRA (*const dstPix)[dst->width] = (PixelBGRA (*)[dst->width])dst->pixels;
    for(uint32_t j = 0; j < region.h; j++) {
        PixelBGRA*       p_dst =    &dstPix[j][0];
        PixelBGRA*const  p_dst_end =&dstPix[j][dst->width]; 
        PixelBGRA const* p_src =    &srcPix[j+region.o_y][region.o_x];
        for(; p_dst < p_dst_end; p_dst++, p_src++) {
            *p_dst = *p_src;
        }
    }
    return dst;
}
void pixelbgraarray_copyAt(PixelBGRAArray const*const src, PixelBGRAArray*const dst, UintPair const dstOrigin) {
    if(!src || !dst) { printerror("No src or dst."); return; }
    if((dstOrigin.uint0 + src->width >  (uint32_t)dst->width) ||
       (dstOrigin.uint1 + src->height > (uint32_t)dst->height)) {
        printerror("Overflow"); return;
    }
    // Pixels sous forme array 2D. Ici on copie toute la source et on se place à dstOrigin dans la destination.
    PixelBGRA (*const srcPix)[src->width] = (PixelBGRA (*)[src->width])src->pixels;
    PixelBGRA (*const dstPix)[dst->width] = (PixelBGRA (*)[dst->width])dst->pixels;
    for(uint32_t j = 0; j < src->height; j++) {
        PixelBGRA*       p_dst =     &dstPix[j+dstOrigin.uint1][dstOrigin.uint0];
        PixelBGRA const* p_src =     &srcPix[j][0];
        PixelBGRA const* p_src_end = &srcPix[j][src->width];
        for(; p_src < p_src_end; p_src++, p_dst++) {
            *p_dst = *p_src;
        }
    }
}
void pixelbgraarray_copyRegionAt(PixelBGRAArray const*const src, RectangleUint const srcRegion,
                                 PixelBGRAArray*const dst, UintPair const dstOrigin) {
    if(!src || !dst) { printerror("No src or dst."); return; }
    if((dstOrigin.uint0 + srcRegion.w > (uint32_t)dst->width) ||
       (dstOrigin.uint1 + srcRegion.h > (uint32_t)dst->height) ||
       (srcRegion.o_x + srcRegion.w > (uint32_t)src->width) ||
       (srcRegion.o_y + srcRegion.h > (uint32_t)src->height)) {
        printerror("Overflow"); return;
    }
    // Pixels sous forme array 2D. Ici on copie toute la source et on se place à dstOrigin dans la destination.
    PixelBGRA (*const srcPix)[src->width] = (PixelBGRA (*)[src->width])src->pixels;
    PixelBGRA (*const dstPix)[dst->width] = (PixelBGRA (*)[dst->width])dst->pixels;
    for(uint32_t j = 0; j < srcRegion.h; j++) {
        PixelBGRA*       p_dst =     &dstPix[j+dstOrigin.uint1][dstOrigin.uint0];
        PixelBGRA const* p_src =     &srcPix[j+srcRegion.o_y][srcRegion.o_x];
        PixelBGRA const* p_src_end = &srcPix[j+srcRegion.o_y][srcRegion.o_x+srcRegion.w];
        for(; p_src < p_src_end; p_src++, p_dst++) {
            *p_dst = *p_src;
        }
    }                            
}

// Bitmap... voir http://en.wikipedia.org/wiki/Windows_bitmap
struct BMHeader_ {
    uint16_t BM;                // 0, La signature "BM" -> 0x4D42 en hexadec.
    uint32_t file_size;         // 2 ** Ici, pour forcer l'aligment compact des données,
    uint32_t _reserved0;        // 6        il faut utiliser `__attribute__((packed))`.
    uint32_t pixel_offset;      // 10
    uint32_t windowsHeaderSize; // 14
    int32_t  width;             // 18
    int32_t  height;            // 22
    uint16_t colorPlane_count;  // 26
    uint16_t bitsPerPixels;     // 28
} __attribute__((packed));
PixelBGRAArray* PixelBGRAArray_createFromBitmapFile(const char*const path, bool flipY) {
    if(!path) { printerror("No path to open."); return NULL; }
    guard_let(FILE*, f, fopen(path, "rb"), printerror("Cannot open %s.", path), NULL)
    PixelBGRAArray* pa = NULL;
    struct BMHeader_ header;
    fread(&header, sizeof(struct BMHeader_), 1, f);
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
    // Ok, créer l'array de pixel et se se placer `f` sur les pixels du bmp.
    pa = PixelBGRAArray_createEmpty(header.width, header.height);
    PixelBGRA (*pixLineOut)[header.width] = (PixelBGRA (*)[header.width])pa->pixels;
    if(flipY) pixLineOut += header.height - 1; // (se placer à la fin si flip upside-down)
    fseek(f, header.pixel_offset, SEEK_SET);
    // Lecture des lignes du bmp.
    if(header.bitsPerPixels == 24) { // Cas Bmp en BGR.
        // Taille d'une ligne du bmp en bytes (multiple de 32 bits)
        size_t const line_size = 4*(size_t)ceilf(((float)header.width * header.bitsPerPixels) / 32.f);
        for(uint32_t j = 0; j < header.height; j ++) {
            // Charger une ligne
            uint8_t line[line_size];
            fread(line, line_size, 1, f);
            // Pixels du bitmap en BGR
            PixelBGR const* pBmp =          (PixelBGR const*) line;
            PixelBGR const*const pBmp_end = (PixelBGR const*)&line[header.width * 3];
            // Pixel de sortie (en bgra)
            PixelBGRA *p = (PixelBGRA*)pixLineOut;
            for(; pBmp < pBmp_end; pBmp++, p++) {
                *p = (PixelBGRA) {
                    .bgr = *pBmp,
                    .a =    255,
                };
            }
            if(flipY) pixLineOut --;
            else      pixLineOut ++;
        }
        goto close_file;
    }
    if(header.bitsPerPixels == 32) { // Cas bmp en BGRA.
        size_t const line_size = sizeof(PixelBGRA) * header.width; // (4 bytes per pixel)
        for(uint32_t j = 0; j < header.height; j ++) {
            // Charger une ligne
            PixelBGRA line[header.width]; // buffer d'une ligne
            fread(line, line_size, 1, f);
            const PixelBGRA *pBmp =            line;
            const PixelBGRA *const pBmp_end = &line[header.width];
            PixelBGRA *p = (PixelBGRA*)pixLineOut;
            for(; pBmp < pBmp_end; pBmp++, p++) {
                *p = *pBmp;
            }
            if(flipY) pixLineOut --;
            else      pixLineOut ++;
        }
        goto close_file;
    }
    }
    printerror("Unsupported bitsPerPixels %d.", header.bitsPerPixels);
close_file:
    fclose(f);
    return pa;
}

//const Vector4 color4_ofEnum[] = {
//    color4_black,
//    color4_black_back,
//    color4_white,
//    color4_white_beige,
//    color4_gray_25,
//    color4_gray_40,
//    color4_gray_50,
//    color4_gray_60,
//    color4_gray_80,
//    color4_red,
//    color4_red_vermilion,
//    color4_red_coquelicot,
//    color4_red_orange2,
//    color4_red_coral,
//    color4_red_dark,
//    color4_orange,
//    color4_orange_amber,
//    color4_orange_bronze,
//    color4_orange_saffron,
//    color4_orange_saffron2,
//    color4_yellow_cadmium,
//    color4_yellow_amber,
//    color4_yellow_citrine,
//    color4_yellow_lemon,
//    color4_green_electric,
//    color4_green_electric2,
//    color4_green_fluo,
//    color4_green_ao,
//    color4_green_spring,
//    color4_green_avocado,
//    color4_green_dark_cyan,
//    color4_aqua,
//    color4_blue,
//    color4_blue_sky,
//    color4_blue_sky2,
//    color4_blue_pale,
//    color4_blue_azure,
//    color4_blue_strong,
//    color4_purple,
//    color4_purble_china_pink,
//    color4_purble_electric_idigo,
//    color4_purble_blue_violet,
//};
