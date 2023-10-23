//
//  MetalView.m
//  Test2
//
//  Created by Corentin Faucher on 2023-10-11.
//

#import "MetalView.h"
#include "graph_texture_apple.h"
#include "MyRoot.h"

@implementation MetalView

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device {
    self = [super initWithFrame:frameRect device:device];
    [self setColorPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    [self setClearColor:MTLClearColorMake(1, 0.5, 0.5, 1)];
    renderer = [[Renderer alloc] initWithView:self];
    [self setDelegate:renderer];
    Mesh_init();
    Texture_init(device);
    Texture_loadPngs(MyProject_pngInfos, png_total_pngs);
    root = NodeRoot_createMyRoot();
    return self;
}

@end
