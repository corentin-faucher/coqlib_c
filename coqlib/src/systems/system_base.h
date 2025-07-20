//
//  coq_system.h
//  Acces à divers info du système, 
//  i.e. Keyboard layout, theme, cloud drive, 
//
//  Created by Corentin Faucher on 2023-10-25.
//

#ifndef COQ_UTIL_SYSTEM_H
#define COQ_UTIL_SYSTEM_H

#include "../utils/util_event.h"

/// Init des variables systèmes : Nom de l'OS, nom et version de l'app, 
/// keyboard layout, cloud drive, langue.
void CoqSystem_init(void);

// Dimension présente de la view.
void         CoqSystem_setViewSize(ViewSizeInfo viewSize);
ViewSizeInfo CoqSystem_getViewSize(void);

enum {
    coqsystem_os_desktop,
    coqsystem_os_tablet_big,  // (genre +12", i.e. assez gros pour afficher un clavier standard.)
    coqsystem_os_tablet,
    coqsystem_os_phone,
    
    coqsystem_os__default_,
};
unsigned    CoqSystem_OS_type(void);
/// Pour tester. Mettre en mode phone/tablet, par exemple.
void        CoqSystem_OS_forceTo(unsigned coqsystem_os);

const char* CoqSystem_OS_versionOpt(void);


void        CoqSystem_layoutUpdate(void);
/// Nom du layout (juste dans macOS pour l'instant), e.g. "com.apple.keylayout.US" pour le Qwerty-US.
const char* CoqSystem_layoutOpt(void);
enum {
    keyboardtype_ansi,
    keyboardtype_iso,
    keyboardtype_jis,
};
unsigned    CoqSystem_keyboardType(void);

const char* CoqSystem_appVersionOpt(void);
const char* CoqSystem_appBuildOpt(void);
const char* CoqSystem_appDisplayNameOpt(void);

void        CoqSystem_theme_OsThemeUpdate(void);
bool        CoqSystem_theme_OsThemeIsDark(void);
void        CoqSystem_theme_setAppTheme(bool isDark);
void        CoqSystem_theme_setAppThemeToOsTheme(void);
bool        CoqSystem_theme_appThemeIsDark(void);

void        CoqSystem_cloudDrive_startWatching_(const char* subFolderOpt, const char* extensionOpt);
bool        CoqSystem_cloudDrive_isEnabled(void);
bool        CoqSystem_cloudDrive_isUpdating(void);
void        CoqSystem_cloudDrive_stopWatching_(void);

//const char* CoqSystem_cloudUserNameOpt(void);
//void        CoqSystem_initCloudUserName(const char* container_name);
//void        CoqSystem_requestPermissionAndSetCloudUserName(void);

/// Pour iOS, par défaut l'apparition du clavier virtuel modifie la margin.bottom 
/// pour ne pas cacher les élement de l'app.
extern bool CoqSystem_dontResizeOnVirtualKeyboard; // false

#endif
