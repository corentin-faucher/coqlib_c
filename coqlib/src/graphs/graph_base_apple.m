//
//  graph_base_apple.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-31.
//
#include "graph_base.h"

#include "../utils/util_base.h"
#import <AppKit/AppKit.h>

// MARK: - Dessin de png
PixelBGRAArray* PixelBGRAArray_engine_createFromPngFileOpt(char const*const pngPath, bool const showError) 
{
    if(!pngPath) { printerror("No png to load."); return NULL; }
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:pngPath]];
    if(image == nil) {
        if(showError) printerror("Cannot load image at %s.", pngPath);
        return NULL;
    }
    NSGraphicsContext *context = [NSGraphicsContext currentContext];
    NSRect rect = { .origin = {0, 0}, .size = image.size };
    CGRect cgRect = { .origin = {0, 0}, .size = image.size };
    // Création de l'image et de l'array de pixels.
    CGImageRef cgImage = [image CGImageForProposedRect:&rect context:context hints:nil];
    size_t width =  CGImageGetWidth(cgImage);
    size_t height = CGImageGetHeight(cgImage);
    PixelBGRAArray* pa = PixelBGRAArray_createEmpty(width, height);
    // Copier l'image dans l'array de pixels.
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;
    CGContextRef cgcontext = CGBitmapContextCreate(pa->pixels, width, height,
                                                 bitsPerComponent, bytesPerRow, colorSpace,
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    CGContextDrawImage(cgcontext, cgRect, cgImage);
    CGContextRelease(cgcontext);
    return pa;
}


