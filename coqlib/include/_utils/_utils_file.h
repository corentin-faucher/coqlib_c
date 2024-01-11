//
//  _utils_file.h
//  Fonctions de base pratiques pour 
//  l'ecriture/lecture de fichiers.
//
//  Created by Corentin Faucher on 2023-12-08.
//
#ifndef _coq_utils_file_h
#define _coq_utils_file_h

#include "_utils_.h"
#if __APPLE__
#include <sys/syslimits.h>
#endif
#ifdef __linux__
#include <limits.h>
#endif

/// Optenir le contenu d'un fichier texte.
/// Il ne faut pas `free` le buffer retourné. Pour ce faire utiliser `FILE_freeBuffer`.
const char* FILE_contentOpt(const char* path);
/// Taille du tempon retourné par `FILE_contentOpt`.
size_t      FILE_bufferSize(void);
/// Libérer le tempon (facultatif).
void        FILE_freeBuffer(void);
/// Ecrire une string dans un fichier texte.
void        FILE_write(const char* path, const char* content);

/// Dossier où sont situé les resources de l'application (res, Resources,...)
/// Retourne le buffer editable `_FileManager_tmp_path` de taille `PATH_MAX`.
char* const FileManager_getResourcePathOpt(const char* fileNameOpt, 
                        const char* fileExtOpt, const char* subDirOpt);
/// Dossier où l'application peut stocker ses fichiers.
/// Retourne le buffer editable `_FileManager_tmp_path` de taille `PATH_MAX`.
char*       FileManager_getApplicactionSupportDirectoryPathOpt(void);

#endif /* file_utils_h */
