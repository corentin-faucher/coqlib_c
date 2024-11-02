//
//  _util_file.h
//  Fonctions de base pratiques pour
//  l'ecriture/lecture de fichiers.
//
//  Created by Corentin Faucher on 2023-12-08.
//
#ifndef COQ_UTIL_FILE_H
#define COQ_UTIL_FILE_H

#include <stdio.h>  // printf, sprintf, file, etc.
#include <stdbool.h>
#include <stdint.h>
#if __APPLE__
#include <sys/syslimits.h>
#endif
#ifdef __linux__
#include <limits.h>
#endif

//enum {
//    file_exist_none,
//    file_exist_file,
//    file_exist_dir,
//};
/// Retourne 1 si un ficher, 2 si un dossier et 0 si rien.
/// Superflu ? On peu juste essayer d'ouvrir...
//int         FILE_existAt(const char* path);

/// Optenir le contenu d'un fichier texte (+1 à la taille pour le `\0` terminal ajouté à la string).
/// Retourne une référence au buffer (privé de `util_file.c`).
/// Il ne faut pas `free` le buffer retourné. Pour ce faire utiliser `FILE_freeBuffer`.
const char* FILE_stringContentOptAt(const char* path);
/// Obtenir le contenu d'un fichier binaire.
const void* FILE_bufferContentOptAt(const char* path);
/// Obtenir le contenu d'un bmp. On passe les emplacements où stocker width et height.
/// Le bitmap est stocker dans le buffer dans le format BGRA, i.e. en hexadec 0xAARRGGBB
///  (compatible avec les textures).
const void* FILE_bitmapBGRA8ContentOptAt(const char* pathOpt,  uint32_t* widthRef, uint32_t* heightRef, bool flipY);
/// Taille du tempon retourné par `FILE_XXXcontentOpt`.
size_t      FILE_bufferSize(void);
/// Libérer le tempon (facultatif).
void        FILE_freeBuffer(void);
/// Ecrire une string dans un fichier texte (terminée par `\0`).
void        FILE_writeString(const char* path, const char* content);
void        FILE_writeData(const char* path, const void* buffer, size_t buffer_size);

/// Dossier où sont situé les resources de l'application (res, Resources,...)
/// Retourne le buffer *editable* `_FileManager_tmp_path` de taille `PATH_MAX`.
char*        FileManager_getResourcePathOpt(const char* fileNameOpt,
                        const char* fileExtOpt, const char* subDirOpt);
/// Dossier `local` où l'application peut stocker ses fichiers.
/// Retourne le buffer *editable* `_FileManager_tmp_path` de taille `PATH_MAX`.
char*        FileManager_getApplicationSupportDirectoryPathOpt(void);
/// Dossier `Cloud` où l'application peut stocker ses fichiers.
/// Retourne le buffer editable `_FileManager_tmp_path` de taille `PATH_MAX`.
char*        FileManager_getApplicationCloudMainDirectoryPathOpt(void);
/// Dossier `Cloud` document où l'application peut stocker ses fichiers.
/// Il s'agit du sous-dossier "Documents" visible dans Finder -> iCloud Drive.
/// Retourne le buffer editable `_FileManager_tmp_path` de taille `PATH_MAX`.
char*        FileManager_getApplicationCloudDocumentsDirectoryPathOpt(void);
/// Vérifie si le directory existe et est bien un directory.
/// S'il s'agit d'un fichier le fichier est effacé.
/// Ensuite on crée le directory s'il est manquant.
/// Retourne true si OK, false si échec.
bool         FileManager_checkAndCreateDirectory(const char* dirPath_cstr);

#endif /* file_util_h */
