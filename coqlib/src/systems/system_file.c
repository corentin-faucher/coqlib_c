//
//  file_utils.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//
#include "system_file.h"

#include <math.h>
#include "../graphs/graph_base.h"
#include "../utils/util_base.h"

static char*  FILE_buffer_ = NULL;
static size_t FILE_buffer_size_ = 0;

const char* FILE_stringContentOptAt(const char* path, bool const hideError) {
    FILE_freeBuffer();
    if(!path) {  
        if(!hideError) printerror("No path to open."); 
        return NULL;
    }
    guard_let(FILE*, f, fopen(path, "r"), if(!hideError) { printerror("Cannot open %s.", path); }, NULL)
    fseek(f, 0, SEEK_END);
    // Ajouter + 1 pour le `\0` de fin de string.
    FILE_buffer_size_ = ftell(f) + 1;
    rewind(f);
    FILE_buffer_ = calloc(1, FILE_buffer_size_);
    if(!FILE_buffer_) {
        FILE_buffer_size_ = 0;
        printerror("Cannot alloc %s.", path);
        goto close_file;
    }
    size_t n = fread(FILE_buffer_, FILE_buffer_size_ - 1, 1, f);
    if(n != 1) {
        FILE_freeBuffer();
        printerror("Cannot read %s.", path);
    }
close_file:
    fclose(f);
    return FILE_buffer_;
}
const void* FILE_bufferContentOptAt(const char* path) {    
    FILE_freeBuffer();
    if(!path) {  printerror("No path to open."); return NULL; }
    guard_let(FILE*, f, fopen(path, "rb"), printerror("Cannot open %s.", path), NULL)
    fseek(f, 0, SEEK_END);
    FILE_buffer_size_ = ftell(f);
    rewind(f);
    FILE_buffer_ = calloc(1, FILE_buffer_size_);
    if(!FILE_buffer_) {
        FILE_buffer_size_ = 0;
        printerror("Cannot alloc %s.", path);
        goto close_file;
    }
    size_t n = fread(FILE_buffer_, FILE_buffer_size_, 1, f);
    fclose(f);
    if(n != 1) {
        FILE_freeBuffer();
        printerror("Cannot read %s.", path);
    }
close_file:
    fclose(f);
    return FILE_buffer_;
}
void     FILE_writeString(const char* path, const char* content) {
    if(!path) { printwarning("No path to open."); return; }
    if(!content) { printwarning("No content to write."); return; }
    guard_let(FILE*, f, fopen(path, "w"), printerror("Cannot write at %s.", path), )
    fputs(content, f);
    fclose(f);
}
void   FILE_writeData(const char* path, const void* buffer, size_t buffer_size) {
    if(!path) { printwarning("No path to open."); return; }
    if(!buffer) { printwarning("No content to write."); return; }
    guard_let(FILE*, f, fopen(path, "wb"), printerror("Cannot write at %s.", path), )
    fwrite(buffer, buffer_size, 1, f);
    fclose(f);
}
size_t FILE_bufferSize(void) {
    return FILE_buffer_size_;
}
void   FILE_freeBuffer(void) {
    if(!FILE_buffer_) return;
    free(FILE_buffer_);
    FILE_buffer_ = NULL;
    FILE_buffer_size_ = 0;
}

// Garbage
//int         FILE_existAt(const char* path) {
//    struct stat st;
//    stat(path, &st);
//    if(S_ISDIR(st.st_mode))
//        return file_exist_dir;
//    if(S_ISREG(st.st_mode))
//        return file_exist_file;
//    return file_exist_none;
//}
// Bitmap... voir http://en.wikipedia.org/wiki/Windows_bitmap
//const void* FILE_bitmapBGRA8ContentOptAt(const char* const path,  uint32_t* widthRef, uint32_t* heightRef, bool flipY) 
//{
//    FILE_freeBuffer();
//    if(!path) { printerror("No path to open."); return NULL; }
//    guard_let(FILE*, f, fopen(path, "rb"), printerror("Cannot open %s.", path), NULL)
//    // Variables du header
//    uint16_t BM_header = 0;
//    uint32_t file_size = 0;
//    uint32_t pixel_offset = 0;
//    uint32_t windowsHeaderSize;
//    uint16_t colorPlane_count, bitsPerPixels;
//    // width et height dans un bmp `signed`. Si négatif -> axe x/y flip.
//    int32_t width, height;  // entier avec signe... ?
//    // Lectur des infos dans le header.
//    fread(&BM_header, 2, 1, f);
//    fread(&file_size, 4, 1, f);
//    fseek(f, 0x0A, SEEK_SET);
//    fread(&pixel_offset, 4, 1, f);
//    fread(&windowsHeaderSize, 4, 1, f);
//    fread(&width, 4, 1, f);
//    fread(&height, 4, 1, f);
//    fread(&colorPlane_count, 2, 1, f);
//    fread(&bitsPerPixels, 2, 1, f);
//    *widthRef =  width >= 0 ?  width :  -width;
//    *heightRef = height >= 0 ? height : -height;
//    if(height < 0) flipY = !flipY;
//    // Besoin de flip l'axe des x ?? (non a priori)
////    printdebug("bitmap w %d, h %d, planes %d, bitsPerPixels %d.", 
////               *widthRef, *heightRef, colorPlane_count, bitsPerPixels);
//    // Header check
//    if(BM_header != 0x4D42) { 
//        printerror("No BM in bitmap header, found %#06x.", BM_header); 
//        goto close_file;
//    }
//    if(*widthRef == 0 || *heightRef == 0 || *widthRef > 4096 || *heightRef > 4096) {
//        printerror("Bad bitmap dimensions, width %d, height %d.", *widthRef, *heightRef); 
//        goto close_file;
//    }
//    // Ok, créer le buffer et se se placer sur les pixels.
//    FILE_buffer_size_ = (*widthRef) * (*heightRef) * sizeof(PixelBGRA);
//    FILE_buffer_ = calloc(1, FILE_buffer_size_);
//    fseek(f, pixel_offset, SEEK_SET);
//    // Lecture des lignes
//    {
//    // Itérateur sur les lignes (Pixels en sortie, format BGRA, dans le buffer)
//    PixelBGRA (*pixLineOut)[*widthRef] = (PixelBGRA (*)[*widthRef])FILE_buffer_;
//    if(flipY) pixLineOut += *heightRef - 1; // (se placer à la fin si flip upside-down)
//    if(bitsPerPixels == 24) {
//        // Taille en bytes (multiple de 32 bits)
//        size_t line_size = 4*(size_t)ceilf(((float)*widthRef * bitsPerPixels) / 32.f);
//        uint8_t line[line_size]; // buffer d'une ligne
//        for(uint32_t j = 0; j < *heightRef; j ++) {
//            // Charger une ligne
//            fread(line, line_size, 1, f);
//            // Pixels du bitmap en BGR
//            const PixelBGR *p =          (PixelBGR const*) line;
//            const PixelBGR * const end = (PixelBGR const*)&line[*widthRef * 3];
//            // Pixel de sortie (en bgra)
//            PixelBGRA *pixelOut = (PixelBGRA*)pixLineOut;
//            while(p < end) {
//                pixelOut->bgr = *p;
//                pixelOut->a =   255;
//                p++; pixelOut++;
//            }
//            if(flipY) pixLineOut --;
//            else      pixLineOut ++;
//        }
//    } else if(bitsPerPixels == 32) {
//        PixelBGRA line[*widthRef]; // buffer d'une ligne
//        size_t line_size = sizeof(PixelBGRA) * (*widthRef); // (4 bytes per pixel)
//        for(uint32_t j = 0; j < *heightRef; j ++) {
//            // Charger une ligne
//            fread(line, line_size, 1, f);
//            const PixelBGRA *p =           line;
//            const PixelBGRA * const end = &line[*widthRef];
//            PixelBGRA *pixelOut = (PixelBGRA*)pixLineOut;
//            while(p < end) {
//                *pixelOut = *p;
//                p++; pixelOut++;
//            }
//            if(flipY) pixLineOut --;
//            else      pixLineOut ++;
//        }
//    } else {
//        printerror("Unsupported bitsPerPixels %d.", bitsPerPixels);
//    }
//    }
//close_file:
//    fclose(f);
//    return FILE_buffer_;
//}
