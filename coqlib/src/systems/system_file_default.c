//
//  _util_file_default.c
//  Obtenir les dossiers lecture / écriture de l'app.
//  Utilise de préférence les versions d'un OS spécifique
//  -> voir _util_file_apple.m...
//
//  Created by Corentin Faucher on 2023-12-08.
//
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include "../systems/system_file.h"
#include "../utils/util_base.h"

// Espace où est stocker le dernier path demandé.
static char  FileManager_tmpPath_[PATH_MAX];
char*        FileManager_getResourcePathOpt(const char* fileNameOpt,
                        const char* fileExtOpt, const char* subDirOpt) 
{
    memset(FileManager_tmpPath_, 0, PATH_MAX);
    if(fileNameOpt) {
        if(subDirOpt) {
            if(fileExtOpt)
                sprintf(FileManager_tmpPath_, "./res/%s/%s.%s",
                    subDirOpt, fileNameOpt, fileExtOpt);
            else
                sprintf(FileManager_tmpPath_, "./res/%s/%s",
                    subDirOpt, fileNameOpt);
        } else {
            if(fileExtOpt)
                sprintf(FileManager_tmpPath_, "./res/%s.%s",
                    fileNameOpt, fileExtOpt);
            else
                sprintf(FileManager_tmpPath_, "./res/%s",
                    fileNameOpt);
        }
    } else {
        if(subDirOpt) {
            sprintf(FileManager_tmpPath_, "./res/%s/", subDirOpt);
        } else {
            sprintf(FileManager_tmpPath_, "./res/");
        }
    }

    return FileManager_tmpPath_;
}
char*        FileManager_getApplicationSupportDirectoryPathOpt(void) {
    memset(FileManager_tmpPath_, 0, PATH_MAX);
    struct stat st = {0};
    if(stat("./userdata/", &st) == -1) {
        mkdir(".userdata", 0755);
    }
    sprintf(FileManager_tmpPath_, "./userdata/");

    return FileManager_tmpPath_;
}

char*        FileManager_getApplicationCloudMainDirectoryPathOpt(void) {
  return NULL;
}

char*        FileManager_getApplicationCloudDocumentsDirectoryPathOpt(void) {
  return NULL;
}

bool  FileManager_checkAndCreateDirectory(const char* dirPath_cstr) {
  if(!dirPath_cstr) { printerror("No dir path."); return false; }

  struct stat st = {0};
  // Rien, créer.
  if(stat(dirPath_cstr, &st) == -1) {
    mkdir(dirPath_cstr, 0755);
    return true;
  }
  // Cas OK.
  if(S_ISDIR(st.st_mode))
    return true;
  // Sinon il y a déjà un fichie du même nom...
  printwarning("File exists with the name of the directory %s.", dirPath_cstr);
  if(remove(dirPath_cstr) != 0) {
    printerror("Cannot delete existing file.");
    return false;
  }
  mkdir(dirPath_cstr, 0755);
  return true;
}
