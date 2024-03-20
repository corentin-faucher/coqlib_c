//
//  main.m
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//

#import <Cocoa/Cocoa.h>
#include "AppDelegate.h"

#include "coq_utils.h"

int main(int argc, const char * argv[]) {
//    test_print_mkcOfKeycode_();
    char char_arr[16];
    printdebug("Sizeof bool %d, char %d, char[16] %d.", sizeof(bool), sizeof(char), sizeof(char_arr));
    
    AppDelegate *delegate = [AppDelegate new];
    NSApplication *app = [NSApplication sharedApplication];
    [app setDelegate:delegate];
    [app run];
    
    return 0;
}
