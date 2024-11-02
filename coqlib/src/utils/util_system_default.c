//
//  util_system_default.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-09.
//

#include "../utils/util_system.h"
#include <stddef.h>

void        CoqSystem_init(void) {
    // pass
}

static unsigned coqsystem_os_type_ = coqsystem_os_desktop;
unsigned    CoqSystem_OS_type(void) {
    return coqsystem_os_type_;
}
/// Pour tester. Mettre en mode phone/tablet, par exemple.
void        CoqSystem_OS_forceTo(unsigned coqsystem_os) {
    coqsystem_os_type_ = coqsystem_os;
}

const char* CoqSystem_OS_versionOpt(void) {
    return NULL;
}

void        CoqSystem_layoutUpdate(void) {
    // pass
}
/// Nom du layout (juste dans macOS pour l'instant), e.g. "com.apple.keylayout.US" pour le Qwerty-US.
const char* CoqSystem_layoutOpt(void) {
    return NULL;
}

unsigned    CoqSystem_keyboardType(void) {
    return keyboardtype_ansi;
}

const char* CoqSystem_appVersionOpt(void) {
    return NULL;
}
const char* CoqSystem_appBuildOpt(void) {
    return NULL;
}
const char* CoqSystem_appDisplayNameOpt(void) {
    return NULL;
}

void        CoqSystem_theme_OsThemeUpdate(void) {
    // pass
}
static bool theme_isDark_ = false;
bool        CoqSystem_theme_OsThemeIsDark(void) {
    return theme_isDark_;
}
void        CoqSystem_theme_setAppTheme(bool isDark) {
    theme_isDark_ = isDark;
}
void        CoqSystem_theme_setAppThemeToOsTheme(void) {
    // pass
}
bool        CoqSystem_theme_appThemeIsDark(void) {
    return theme_isDark_;
}

void        CoqSystem_cloudDrive_startWatching_(const char* subFolderOpt, const char* extensionOpt) {
    // pass
}
bool        CoqSystem_cloudDrive_isEnabled(void) {
    return false;
}
bool        CoqSystem_cloudDrive_isUpdating(void) {
    return false;
}
void        CoqSystem_cloudDrive_stopWatching_(void) {
    // pass
}

bool CoqSystem_dontResizeOnVirtualKeyboard = false;
