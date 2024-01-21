//
//  bundle_utils.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#include "utils/utils_file.h"
#import <Foundation/Foundation.h>

// Espace où est stocker le dernier path demandé.
static char  _FileManager_tmp_path[PATH_MAX];

char* const FileManager_getResourcePathOpt(const char* fileNameOpt, const char* fileExtOpt,
                                   const char* subDirOpt) {
    memset(_FileManager_tmp_path, 0, PATH_MAX);
    NSString* fileName = fileNameOpt ? [NSString stringWithUTF8String:fileNameOpt] : nil;
    NSString* fileExt = fileExtOpt ? [NSString stringWithUTF8String:fileExtOpt] : nil;
    NSString* subDir = subDirOpt ? [NSString stringWithUTF8String:subDirOpt] : nil;
    NSURL* url = [NSBundle.mainBundle URLForResource:fileName
                                       withExtension:fileExt
                                        subdirectory:subDir];
    if(url == nil) { printerror("No url for %s.", fileNameOpt); return NULL; }
    
    // Il faut copier car url et son path seront detruit...
    const char* path = [[url path] UTF8String];
    strcpy(_FileManager_tmp_path, path);
    return _FileManager_tmp_path;
}
char* FileManager_getApplicactionSupportDirectoryPathOpt(void) {
    memset(_FileManager_tmp_path, 0, PATH_MAX);
    NSError* error = nil;
    NSURL* appSupDir = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&error];
    if(error != nil) {
        printerror("Cannot get application support directory.");
        return NULL;
    }
    const char* path = [[appSupDir path] UTF8String];
    strcpy(_FileManager_tmp_path, path);
    return _FileManager_tmp_path;
}

