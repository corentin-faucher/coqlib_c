//
//  bundle_utils.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#include "utils/utils_file.h"

#import <Foundation/Foundation.h>
#include "utils/utils_base.h"

// Espace où est stocker le dernier path demandé.
static char  _FileManager_tmp_path[PATH_MAX];

char* FileManager_getResourcePathOpt(const char* fileNameOpt, const char* fileExtOpt,
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
char* FileManager_getApplicationSupportDirectoryPathOpt(void) {
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

char* FileManager_getApplicationCloudMainDirectoryPathOpt(void) {
    NSURL* icloudContainer = [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:NULL];
    if(icloudContainer == nil) { return NULL; }
    strcpy(_FileManager_tmp_path, [[icloudContainer path] UTF8String]);
    return _FileManager_tmp_path;
}
char* FileManager_getApplicationCloudDocumentsDirectoryPathOpt(void) {
    NSURL* icloudContainer = [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:NULL];
    NSURL* icloudDocuments = [icloudContainer URLByAppendingPathComponent:@"Documents" isDirectory:YES];
    if(icloudDocuments == nil) { return NULL; }
    const char* icloudDocCharStr = [[icloudDocuments path] UTF8String];
    if(!FileManager_checkAndCreateDirectory(icloudDocCharStr))
        return NULL;
    strcpy(_FileManager_tmp_path, icloudDocCharStr);
    return _FileManager_tmp_path;
}

bool  FileManager_checkAndCreateDirectory(const char* dir_path) {
    if(!dir_path) { printerror("No dir path."); return false; }
    int exist = FILE_fileExistAt(dir_path);
    // 1. Case OK (already exists)
    if (exist == file_exist_dir) {
        return true;
    }
    // 2. Case file... delete the file.
    if (exist == file_exist_file) {
        printwarning("File exists with the name of the directory %s.", dir_path);
        NSError* error = nil;
        [[NSFileManager defaultManager] removeItemAtPath:[NSString stringWithUTF8String:dir_path] error:&error];
        if(error != nil) {
            printerror("Cannot remove file with dir name : %s.", [[error localizedDescription] UTF8String]);
            return false;
        }
    }
    // 3. Create the directory
    NSError* error = nil;
    [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithUTF8String:dir_path]
                              withIntermediateDirectories:YES attributes:nil error:&error];
    if(error != nil) {
        printerror("Cannot create dir : %s.", [[error localizedDescription] UTF8String]);
        return false;
    }
    return true;
}
