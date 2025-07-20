//
//  util_system_default.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-09.
//
#include "system_base.h"

#include <stddef.h>
#include "system_file.h"
#include "system_language.h"
#include "../utils/util_base.h"
#include "../utils/cJSON.h"

//static unsigned    coqsystem_keyboardtype_ = keyboardtype_ansi;
static unsigned    coqsystem_os_type_ = coqsystem_os_desktop;
static char const* coqsystem_os_version_ = NULL;
static char const* coqsystem_app_version_ = NULL;
static char const* coqsystem_app_build_ = NULL;
static char const* coqsystem_app_display_name_ = NULL;


void        CoqSystem_init(void) {
    if(coqsystem_app_build_ != NULL) { printerror("Already init."); return; }
    #ifdef __APPLE__
    coqsystem_os_version_ = "macOS ?";
    #elif __linux__
    coqsystem_os_version_ = "some linux...";
    #else
    coqsystem_os_version_ = "some OS...";
    #endif
    // Essayer de Charger les infos depuis app_info.json.
    const char *path = FileManager_getResourcePathOpt("app_info", "json", NULL);
    const char *content = FILE_stringContentOptAt(path, true);
    if(!content) {
        printwarning("In res folder, add app_info.json with the fields app_version and app_build.");
        goto set_default;
    }
    cJSON *app_info = cJSON_ParseWithLength(content, FILE_bufferSize());
    FILE_freeBuffer();
    if (!app_info) {
        printerror("Cannot load app_info.json.");
        goto set_default;
    }
    if_let(cJSON*, app_version, cJSON_GetObjectItem(app_info, "app_version"))
        coqsystem_app_version_ = String_createCopy(cJSON_GetStringValue(app_version));
    if_let_end
    if_let(cJSON*, app_build, cJSON_GetObjectItem(app_info, "app_build"))
        coqsystem_app_build_ = String_createCopy(cJSON_GetStringValue(app_build));
    if_let_end
    cJSON_Delete(app_info);
    
set_default:
    if(!coqsystem_app_build_)   coqsystem_app_build_ = "0";
    if(!coqsystem_app_version_) coqsystem_app_version_ = "0.0";
    
    // Theme
    CoqSystem_theme_OsThemeUpdate();
    // iCloud
//    CoqSystem_cloudDrive_init_();
    // Langue
    Language_init_();
    coqsystem_app_display_name_ = String_createLocalized("app_name");
}

static ViewSizeInfo coqsystem_viewsize_ = {};
void       CoqSystem_setViewSize(ViewSizeInfo const viewSize) {
    coqsystem_viewsize_ = viewSize;
}
ViewSizeInfo CoqSystem_getViewSize(void) {
    if(coqsystem_viewsize_.framePt.h == 0) {
        printerror("View size info still not init...");
    }
    return coqsystem_viewsize_;
}


unsigned    CoqSystem_OS_type(void) {
    return coqsystem_os_type_;
}
/// Pour tester. Mettre en mode phone/tablet, par exemple.
void        CoqSystem_OS_forceTo(unsigned coqsystem_os) {
    coqsystem_os_type_ = coqsystem_os;
}

const char* CoqSystem_OS_versionOpt(void) {
    return coqsystem_os_version_;
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
    return coqsystem_app_version_;
}
const char* CoqSystem_appBuildOpt(void) {
    return coqsystem_app_build_;
}
const char* CoqSystem_appDisplayNameOpt(void) {
    return coqsystem_app_display_name_;
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
