//
//  main.m
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-23.
//

#import <Cocoa/Cocoa.h>
#include "AppDelegate.h"

#include "maths.h"
#include "string_utils.h"

int main(int argc, const char * argv[]) {
    
    printf("Testing 🐷🐷\n");
    const char test1[] = "Ω";
    const char test2[] = "Ω🐷";
    const char* p = test1;
    while(*p) {
        printf(" %d ", (uint8_t)*p);
        p++;
    }
    size_t len1 = strlen(test1), len2 = strlen(test2);
    printf("\nstrcmp %d, strlen %d, %d.\n",
           strncmp(test1, test2,len1), len1, len2);
    
    printf("%s start with prefix %s : %d.",
           test1, test2, string_startWithPrefix(test1, test2));
    
    printf("\n\nTesting 🐷🐷\n\n");
    
    AppDelegate *delegate = [AppDelegate new];
    NSApplication *app = [NSApplication sharedApplication];
    [app setDelegate:delegate];
    [app run];
    
    return 0;
}
