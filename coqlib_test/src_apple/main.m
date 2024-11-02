//
//  main.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-02.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

//#include "maths/math_flpos.h"

int main(int argc, const char * argv[]) {
//    FluidPos fl0, fl1;
//    fl0._c = (FluidPosCore_){ .pos = 1, .A = 2, .B = 3, .time = 2000000000 };
//    fl1._c.v = fl0._c.v;
//    printf("pos %f, A %f, B %f, time %d, v %v4f.", fl1._c.pos, fl1._c.A, fl1._c.B, fl1._c.time, fl1._c.v);
    
//    AppDelegateOpenGL *delegate = [AppDelegateOpenGL new];
    AppDelegate *delegate = [AppDelegate new];
    NSApplication *app = [NSApplication sharedApplication];
    [app setDelegate:delegate];   
    [app run];
    return 0;
}
