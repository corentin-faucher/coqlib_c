//
//  language.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-25.
//


#import <Foundation/Foundation.h>

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

// Specifique a Apple.
NSBundle *_englishBundle;
NSBundle *_currentBundle;
NSBundle* _getBundleForIso(const char* iso) {
    NSString* path = [NSBundle.mainBundle
        pathForResource:[NSString stringWithUTF8String:iso]
                 ofType:@"lproj"];
    if(path == NULL) {
        printerror("Cannot find bundle resource for %s.", iso);
        return [NSBundle mainBundle];
    }
    NSBundle* bundle = [NSBundle bundleWithPath:path];
    if(bundle == NULL) {
        printerror("Cannot load bundle at %s.", path.UTF8String);
        return [NSBundle mainBundle];
    }
    return bundle;
}

// Init les bundles.
void     Language_init(void) {
    _englishBundle = _getBundleForIso("en");
    _currentBundle = _englishBundle;
}
/// La langue detecter sur l'OS. (English si non disponible.)
Language Language_getSystemLanguage(void) {
    NSString *langISO = [NSLocale currentLocale].languageCode;
    if(langISO == NULL) {
        printerror("Locale language not defined. Taking default: %u.", _Language_default);
        return _Language_default;
    }
    // Cas du chinois
    if([langISO compare:@"zh"] == NSOrderedSame) {
        NSString* scriptCode = NSLocale.currentLocale.scriptCode;
        langISO = [NSString stringWithFormat:@"zh-%@", (scriptCode ? scriptCode : @"Hans")];
        Language language = Language_getLanguageWithIso(langISO.UTF8String);
        if(language == _Language_default) {
//            printwarning("Not finding locale chinese %s.", langISO.UTF8String);
            return language_chineseSimplified;
        }
        return language;
    }
    // Sinon vérifier si définie parmis les autres langues...
    return Language_getLanguageWithIso(langISO.UTF8String);
}
Language Language_getLanguageWithIso(const char* const iso) {
    for(Language language = 0; language < language_total_language; language++) {
        if(strcmp(_Language_isoList[language], iso) == 0) {
            return language;
        }
    }
    printwarning("Language with iso %s not found.", iso);
    return _Language_default;
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
    
    _currentBundle = _getBundleForIso(Language_currentIso());
    
//    changeLanguageAction?()
}
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

/// Le code iso, e.g. english -> en.
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
char* String_createLocalized(const char* stringKey) {
    NSString* key = [NSString stringWithUTF8String:stringKey];
    // ("Localizable" est la "table" utilisée par défaut...)
    NSString* localized = [_currentBundle localizedStringForKey:key value:@"⁉️" table:nil];
    if(![localized  isEqual: @"⁉️"])
        return String_createCopy([localized UTF8String]);
    
    // Essayer avec le default bundle (english).
    if(_currentBundle != _englishBundle) {
        localized = [_englishBundle localizedStringForKey:key value:@"⁉️" table:nil];
        if(![localized  isEqual: @"⁉️"])
            return String_createCopy([localized UTF8String]);
    }
    printerror("Cannot localized %s.", stringKey);
    return String_createCopy(stringKey);
}
/// Version par defaut de la string, e.g. localization anglaise.
char* String_createLocalizedEnglish(const char* stringKey) {
    NSString* key = [NSString stringWithUTF8String:stringKey];
    NSString* localized = [_englishBundle localizedStringForKey:key value:@"⁉️" table:nil];
    if(![localized  isEqual: @"⁉️"]) {
        return String_createCopy([localized UTF8String]);
    }
    printerror("Cannot localized %s in english.", stringKey);
    return String_createCopy(stringKey);
}
