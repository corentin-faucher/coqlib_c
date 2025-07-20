//
//  coq_language.h
//  Pour la localisation des strings...
//
//  Created by Corentin Faucher on 2023-10-25.
//

#include "system_language.h"

#include "system_file.h"
#include "../utils/cJSON.h"
#include "../utils/util_base.h"
#include "../utils/util_map.h"


static StringMap *currentStringMap_ = NULL;
static Language   currentStringMapLanguage_;
static StringMap *defaultStringMap_ = NULL;

StringMap* language_createStringMapOpt_(Language const language) {
    const char *iso = language_iso(language);
    const char *path = FileManager_getResourcePathOpt(iso, "json", "localized");
    const char *content = FILE_stringContentOptAt(path, false);
    if(!content) {
        printerror("No %s.json localization file.", iso);
        return NULL;
    }
    cJSON *jsonFile = cJSON_ParseWithLength(content, FILE_bufferSize());
    FILE_freeBuffer();
    if (!jsonFile) {
        printwarning("No json for language %s.", Language_currentIso());
        return NULL;
    }
    StringMap* map = Map_create(100, 100);
    for (cJSON *item = jsonFile->child; item != NULL; item = item->next) {
        map_putAsString(map, item->string, item->valuestring);
    }
    cJSON_Delete(jsonFile);
    return map;
}

void     Language_init_(void) {
    if (defaultStringMap_) {
        printwarning("Already init.");
        return;
    }
    defaultStringMap_ = language_createStringMapOpt_(Language_defaultLanguage);
    if(!defaultStringMap_) {
        printerror("Missing english localization."); return;
    }
    currentStringMap_ = defaultStringMap_;
    currentStringMapLanguage_ = Language_defaultLanguage;
  
    // Init system language
    Language_checkSystemLanguage();
    Language_setCurrent(Language_systemLanguage()); // -> call Language_system_tryToSetTo_...  
}
bool     Language_system_tryToSetTo_(Language const newLanguage) {
    // Pas de changement ?
    if(newLanguage == currentStringMapLanguage_) return true;
    // Changement ?
    StringMap* newMap;
    if(newLanguage == Language_defaultLanguage) {
        newMap = defaultStringMap_;
    } else {
        newMap = language_createStringMapOpt_(newLanguage);
    }
    if(!newMap) {
        printerror("Cannot find localization json for %s.", language_name(newLanguage));
        return false;
    }
    // Changer la map...
    // Effacer l'ancienne map (si pas la map par défaut)
    if(currentStringMap_ && (currentStringMap_ != defaultStringMap_))
        map_destroyAndNull(&currentStringMap_, NULL);
    currentStringMap_ = newMap;
    currentStringMapLanguage_ = newLanguage;
    return true;
}


/*-- Pour la localisation des strings. ----------------*/
/// Localization d'une string dans la langue courante.
/// Apple : Utilise le Bundle de l'app et la resource Localizable.strings.
const char *String_createLocalized(const char *stringKey) {
    if (!currentStringMap_) {
    printerror("Language map not init.");
    return String_createCopy(stringKey);
  }
  const char *locRef = map_valueRefOptOfKey(currentStringMap_, stringKey);
  if (!locRef) {
    return String_createLocalizedDefault(stringKey);
  }
  return String_createCopy(locRef);
}
void        String_copyLocalizedTo(const char* stringKey, char* buffer, size_t size_max_opt) {
    if(!buffer) { printerror("No dest. to copy to."); return;}
    if (!currentStringMap_) {
        printerror("Language map not init.");
        return;
    }
    char const* locRef = map_valueRefOptOfKey(currentStringMap_, stringKey);
    if(locRef) {
        if(size_max_opt) strncpy(buffer, locRef, size_max_opt);
        else strcpy(buffer, locRef);
        return; 
    }
    // Essayer avec le default bundle (english).
    locRef = map_valueRefOptOfKey(defaultStringMap_, stringKey);
    if(locRef) {
        if(size_max_opt) strncpy(buffer, locRef, size_max_opt);
        else strcpy(buffer, locRef);
        return; 
    }
    printerror("Cannot find %s.", stringKey);
    if(size_max_opt) strncpy(buffer, stringKey, size_max_opt);
    else strcpy(buffer, stringKey);
}
/// Version par defaut de la string, e.g. localization anglaise.
const char *String_createLocalizedDefault(const char *stringKey) {
    if (!defaultStringMap_) {
    printerror("Default language map not init.");
    return String_createCopy(stringKey);
  }
  const char *locRef = map_valueRefOptOfKey(defaultStringMap_, stringKey);
  if (!locRef) {
    printerror("No default localization for %s.", stringKey);
    return String_createCopy(stringKey);
  }
  return String_createCopy(locRef);
}

