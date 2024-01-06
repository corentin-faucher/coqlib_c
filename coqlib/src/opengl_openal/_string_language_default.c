//
//  coq_language.h
//  Pour la localisation des strings...
//
//  Created by Corentin Faucher on 2023-10-25.
//

#include "_string_language.h"

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

void     Language_init(void) {
  printwarning("TODO");
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
    _Language_current = newCurrentLanguage;
    
//    changeLanguageAction?()
}
/// La langue detecter sur l'OS. (English si non disponible.)
Language Language_getSystemLanguage(void) {
  printerror("TODO");
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
  printerror("TODO");
  return "error";
}
/// Version par defaut de la string, e.g. localization anglaise.
char*    String_createLocalizedEnglish(const char* stringKey) {
  printerror("TODO");
  return "error";
}

