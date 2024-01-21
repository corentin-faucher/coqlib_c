//
//  coq_language.h
//  Pour la localisation des strings...
//
//  Created by Corentin Faucher on 2023-10-25.
//

#include "cJSON.h"
#include "coq_map.h"
#include "coq_utils.h"

/*----- Privates variables -----------------_*/
const Language _Language_default = language_english;
Language       _Language_current = language_english;
const char*    _Language_isoList[language_total_language] = {
    "fr", "en", "ja", "de",
    "zh-Hans", "it", "es", "ar",
    "el", "ru", "sv", "zh-Hant",
    "pt", "ko", "vi",
};
const char*    _Language_nameList[language_total_language] = {
    "french", "enghish", "japanese", "german",
    "chinese simpl.", "italian", "spanish", "ararbic",
    "greek", "russian", "swedish", "chinese trad.",
    "portuguese", "korean", "vietnamese",
};
static StringMap* _localizedOfStrKey = NULL;
static StringMap* _localizedOfStrKeyDefault = NULL;

void     _Language_fillMap(StringMap* map, const char* iso) {
  if(!map) {
    printerror("Map not init."); return;
  }
  const char* path = FileManager_getResourcePathOpt(iso, "json", "localized");
  const char* content = FILE_contentOpt(path);
  cJSON* jsonFile = cJSON_ParseWithLength(content, FILE_bufferSize());
  FILE_freeBuffer();
  if(!jsonFile) { printwarning("No json for language %s.", Language_currentIso()); return; }
  for(cJSON* item = jsonFile->child; item != NULL; item = item->next) {
     map_putAsString(map, item->string, item->valuestring);
  }
  cJSON_Delete(jsonFile);
}

void     Language_init(void) {
  if(_localizedOfStrKeyDefault) {
    printwarning("Already init."); return;
  }
  
  _localizedOfStrKeyDefault = Map_create(100, 100);
  _Language_fillMap(_localizedOfStrKeyDefault, language_iso(_Language_default));

  _Language_current = Language_getSystemLanguage();
  if(_Language_current == _Language_default) {
    _localizedOfStrKey = _localizedOfStrKeyDefault;
    return;
  }
  _localizedOfStrKey = Map_create(100, 100);
  _Language_fillMap(_localizedOfStrKey, language_iso(_Language_current));
}
Language Language_current(void) {
  return _Language_current;
}
void     Language_setCurrent(Language newCurrentLanguage) {
  if(newCurrentLanguage == _Language_current)
        return;
    if(newCurrentLanguage >= language_total_language) {
        printerror("Language undefined.");
        return;
    }
    if(_localizedOfStrKey != _localizedOfStrKeyDefault) {
      map_destroyAndNull(&_localizedOfStrKey, NULL);
    }

    _Language_current = newCurrentLanguage;
    if(_Language_current == _Language_default) {
      _localizedOfStrKey = _localizedOfStrKeyDefault;
      return;
    }
    _localizedOfStrKey = Map_create(100, 100);
    _Language_fillMap(_localizedOfStrKey, language_iso(_Language_current));
//    changeLanguageAction?()
}
/// La langue detecter sur l'OS. (English si non disponible.)
Language Language_getSystemLanguage(void) {
  return _Language_default;
}
/// Obtenir l'enum Language a partir du code iso, e.g. "en" -> language_english.
Language Language_getLanguageWithIso(const char* const iso) {
  for(Language language = 0; language < language_total_language; language++) {
        if(strcmp(_Language_isoList[language], iso) == 0) {
            return language;
        }
    }
    printwarning("Language with iso %s not found.", iso);
    return _Language_default;
}
//Language Language_getLanguageWithIso(char* iso);
bool     Language_currentIs(Language other) {
  return other == _Language_current;
}
/// De droite a gauche. Vrai pour l'arabe.
bool     Language_currentIsRightToLeft(void) {
    return _Language_current == language_arabic;
}
/// Direction d'ecriture. Arabe -1, autre +1.
float    Language_currentDirectionFactor(void) {
    return (_Language_current == language_arabic) ? -1.f : 1.f;
}
const char* Language_currentIso(void) {
    return _Language_isoList[_Language_current];
}
/// Le code iso, e.g. language_english -> "en".
const char*    language_iso(Language language) {
    return _Language_isoList[language];
}
/// Le nom de la langue en anglais, e.g. language_english -> english.
const char*    language_name(Language language) {
    return _Language_nameList[language];
}

/*-- Pour la localisation des strings. ----------------*/
/// Localization d'une string dans la langue courante.
/// Apple : Utilise le Bundle de l'app et la resource Localizable.strings.
char*    String_createLocalized(const char* stringKey) {
  if(!_localizedOfStrKey) {
    printerror("Language map not init."); 
    return String_createCopy(stringKey);
  }
  const char * locRef = map_valueRefOptOfKey(_localizedOfStrKey, stringKey);
  if(!locRef) {
    return String_createLocalizedDefault(stringKey);
  }
  return String_createCopy(locRef);
}
/// Version par defaut de la string, e.g. localization anglaise.
char*    String_createLocalizedDefault(const char* stringKey) {
  if(!_localizedOfStrKeyDefault) {
    printerror("Default language map not init."); 
    return String_createCopy(stringKey);
  }
  const char * locRef = map_valueRefOptOfKey(_localizedOfStrKeyDefault, stringKey);
  if(!locRef) {
    printerror("No default localization for %s.", stringKey); 
    return String_createCopy(stringKey);
  }
  return String_createCopy(locRef);
}

