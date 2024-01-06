//
//  bundle_utils.h
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#ifndef bundle_utils_h
#define bundle_utils_h

const char* MacOS_currentLayoutOpt(void);
/// Vérifie le layout courant dans macOS.
/// Doit être callé dans la thread principale.
void        _MacOS_updateCurrentLayout(void);

#endif /* bundle_utils_h */
