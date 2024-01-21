//
//  _utils_file_default.c
//  Obtenir les dossiers lecture / écriture de l'app.
//  Utilise de préférence les versions d'un OS spécifique
//  -> voir _utils_file_apple.m...
//
//  Created by Corentin Faucher on 2023-12-08.
//
#include <sys/stat.h>
#include <sys/types.h>
#include "utils/utils_file.h"

// Espace où est stocker le dernier path demandé.
static char  _FileManager_tmp_path[PATH_MAX];

char* const FileManager_getResourcePathOpt(const char* fileNameOpt, const char* fileExtOpt,
                                   const char* subDirOpt) {
    memset(_FileManager_tmp_path, 0, PATH_MAX);

    if(fileNameOpt) {
        if(subDirOpt) {
            if(fileExtOpt)
                sprintf(_FileManager_tmp_path, "./res/%s/%s.%s", 
                    subDirOpt, fileNameOpt, fileExtOpt);
            else
                sprintf(_FileManager_tmp_path, "./res/%s/%s",
                    subDirOpt, fileNameOpt);
        } else {
            if(fileExtOpt)
                sprintf(_FileManager_tmp_path, "./res/%s.%s", 
                    fileNameOpt, fileExtOpt);
            else
                sprintf(_FileManager_tmp_path, "./res/%s",
                    fileNameOpt);
        }
    } else {
        if(subDirOpt) {
            sprintf(_FileManager_tmp_path, "./res/%s/", subDirOpt);
        } else {
            sprintf(_FileManager_tmp_path, "./res/");
        }
    }
    
    return _FileManager_tmp_path;
}
char* FileManager_getApplicactionSupportDirectoryPathOpt(void) {
    memset(_FileManager_tmp_path, 0, PATH_MAX);
    struct stat st = {0};
    if(stat("./userdata/", &st) == -1) {
        mkdir(".userdata", 0755);
    }
    sprintf(_FileManager_tmp_path, "./userdata/");
    
    return _FileManager_tmp_path;
}

