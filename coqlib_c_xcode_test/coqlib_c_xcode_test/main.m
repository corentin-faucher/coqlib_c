//
//  main.m
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-23.
//

#import <Cocoa/Cocoa.h>
#include "AppDelegate.h"

int main(int argc, const char * argv[]) {
    AppDelegate *delegate = [AppDelegate new];
    NSApplication *app = [NSApplication sharedApplication];
    [app setDelegate:delegate];
    [app run];
    
    return 0;
}
