//
//  main.m
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//

#import <Cocoa/Cocoa.h>
#include "AppDelegate.h"

typedef int v4si __attribute__((vector_size(16)));

int main(int argc, const char * argv[]) {
    v4si a, b, c;
    a = (v4si){ 1, 2, 3, 4};
    b = (v4si){ 1, 2, 3, 4};
    c = a + b;
    printdebug("a + b = %d, %d, %d, %d.", c[0], c[1], c[2], c[3]);
    
    AppDelegate *delegate = [AppDelegate new];
    NSApplication *app = [NSApplication sharedApplication];
    [app setDelegate:delegate];
    [app run];
    
    return 0;
}
