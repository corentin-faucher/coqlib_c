//
//  main.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-02.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"


int main(int argc, const char * argv[]) {

    AppDelegate *delegate = [AppDelegate new];
    NSApplication *app = [NSApplication sharedApplication];
    [app setDelegate:delegate];   
    [app run];
    return 0;
}
