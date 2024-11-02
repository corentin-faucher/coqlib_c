//
//  apple_opengl_view.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-08.
//
#import "apple_view_opengl.h"
#import <OpenGL/gl.h> 
#import <OpenGL/glu.h> 
#import <GLUT/glut.h>

#include "utils/util_base.h"
#include "graphs/graph__opengl.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@implementation CoqOpenGLView

-(instancetype)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format {
    printhere();
    self = [super initWithFrame:frameRect pixelFormat:format];
    return self;
}

-(void)prepareOpenGL {
    printhere();
    [super prepareOpenGL];
    [NSTimer scheduledTimerWithTimeInterval:0.1 repeats:true block:^(NSTimer * _Nonnull timer) {
        [self display];
    }];
    [[self openGLContext] makeCurrentContext];
    CoqGraph_engine_initOpenGL();
    
    CoqGraph_init(NULL, 0, true);
}

-(void)drawRect:(NSRect)dirtyRect {
//    printdebug("Drawing rect %f %f.", dirtyRect.size.width, dirtyRect.size.height);
    [[self openGLContext] makeCurrentContext];
    glViewport(0, 0, dirtyRect.size.width, dirtyRect.size.height);
    glClearColor(0.5,0,0,0.5); 
    glClear(GL_COLOR_BUFFER_BIT);
//    glFlush();
    [[ self openGLContext ] flushBuffer];
}

@end
#pragma clang diagnostic pop
