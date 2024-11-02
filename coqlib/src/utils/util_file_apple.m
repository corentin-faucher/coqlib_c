//
//  bundle_utils.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#include "util_file.h"

#import <Foundation/Foundation.h>
#include "util_base.h"

// Espace où est stocker le dernier path demandé.
static char  FileManager_tmp_path_[PATH_MAX];

char* FileManager_getResourcePathOpt(const char*const fileNameOpt, const char*const fileExtOpt,
                                   const char*const subDirOpt) {
    memset(FileManager_tmp_path_, 0, PATH_MAX);
    NSString* fileName = fileNameOpt ? [NSString stringWithUTF8String:fileNameOpt] : nil;
    NSString* fileExt = fileExtOpt ? [NSString stringWithUTF8String:fileExtOpt] : nil;
    NSString* subDir = subDirOpt ? [NSString stringWithUTF8String:subDirOpt] : nil;
    NSURL* url = [NSBundle.mainBundle URLForResource:fileName
                                       withExtension:fileExt
                                        subdirectory:subDir];
    if(url == nil) { return NULL; }
    // Il faut copier car url et son path seront detruit...
    const char* path = [[url path] UTF8String];
    strcpy(FileManager_tmp_path_, path);
    return FileManager_tmp_path_;
}
char* FileManager_getApplicationSupportDirectoryPathOpt(void) {
    memset(FileManager_tmp_path_, 0, PATH_MAX);
    NSError* error = nil;
    NSURL* appSupDir = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&error];
    if(error != nil) {
        printerror("Cannot get application support directory.");
        return NULL;
    }
    const char* path = [[appSupDir path] UTF8String];
    strcpy(FileManager_tmp_path_, path);
    return FileManager_tmp_path_;
}

char* FileManager_getApplicationCloudMainDirectoryPathOpt(void) {
    NSURL* icloudContainer = [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:NULL];
//    if(icloudContainer == nil) {
//        printerror("No iCloud countainer? and icloud token is %s.", 
//            [[[[NSFileManager defaultManager] ubiquityIdentityToken] description] UTF8String]);
//    }
    if(icloudContainer == nil) { return NULL; }
    strcpy(FileManager_tmp_path_, [[icloudContainer path] UTF8String]);
    return FileManager_tmp_path_;
}
char* FileManager_getApplicationCloudDocumentsDirectoryPathOpt(void) {
    NSURL* icloudContainer = [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:NULL];
    NSURL* icloudDocuments = [icloudContainer URLByAppendingPathComponent:@"Documents" isDirectory:YES];
    if(icloudDocuments == nil) { return NULL; }
    const char* icloudDocCharStr = [[icloudDocuments path] UTF8String];
    if(!FileManager_checkAndCreateDirectory(icloudDocCharStr))
        return NULL;
    strcpy(FileManager_tmp_path_, icloudDocCharStr);
    return FileManager_tmp_path_;
}

bool  FileManager_checkAndCreateDirectory(const char* dirPath_cstr) {
    if(!dirPath_cstr) { printerror("No dir path."); return false; }
    NSString* path = [NSString stringWithUTF8String:dirPath_cstr];
    BOOL isDirectory;
    BOOL fileExists = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDirectory];
    // 1. Case OK (already exists)
    if (fileExists && isDirectory) {
        return true;
    }
    // 2. Case file... delete the file.
    if (fileExists) {
        printwarning("File exists with the name of the directory %s.", dirPath_cstr);
        NSError* error = nil;
        [[NSFileManager defaultManager] removeItemAtPath:path error:&error];
        if(error != nil) {
            printerror("Cannot remove file with dir name : %s.", [[error localizedDescription] UTF8String]);
            return false;
        }
    }
    // 3. Create the directory
    NSError* error = nil;
    [[NSFileManager defaultManager] createDirectoryAtPath:path
                              withIntermediateDirectories:YES attributes:nil error:&error];
    if(error != nil) {
        printerror("Cannot create dir : %s.", [[error localizedDescription] UTF8String]);
        return false;
    }
    return true;
}
