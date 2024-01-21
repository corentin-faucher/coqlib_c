//
//  language.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-25.
//


#import <Foundation/Foundation.h>

#include "coq_utils.h"

/*----- Privates variables -----------------_*/
const Language default_language_ = language_english;
Language       current_language_ = language_english;
const char*    language_iso_strings_[language_total_language] = {
    "fr", "en", "ja", "de",
    "zh-Hans", "it", "es", "ar",
    "el", "ru", "sv", "zh-Hant",
    "pt", "ko", "vi",
};
const char*    language_name_strings_[language_total_language] = {
    "french", "enghish", "japanese", "german",
    "chinese simpl.", "italian", "spanish", "ararbic",
    "greek", "russian", "swedish", "chinese trad.",
    "portuguese", "korean", "vietnamese",
};

// Specifique a Apple.
NSBundle *default_bundle_ = nil;
NSBundle *current_bundle_ = nil;
NSBundle* getBundleForIsoOpt_(const char* iso) {
    NSString* path = [NSBundle.mainBundle
        pathForResource:[NSString stringWithUTF8String:iso]
                 ofType:@"lproj"];
    if(path == NULL) {
        printerror("Cannot find bundle resource for %s.", iso);
        return nil;
    }
    NSBundle* bundle = [NSBundle bundleWithPath:path];
    if(bundle == NULL) {
        printerror("Cannot load bundle at %s.", path.UTF8String);
        return nil;
    }
    return bundle;
}

// Init les bundles.
void     Language_init(void) {
    default_bundle_ = getBundleForIsoOpt_("en");
    if(!default_bundle_) default_bundle_ = [NSBundle mainBundle];
    current_bundle_ = default_bundle_;
    Language_setCurrent(Language_getSystemLanguage());
}
/// La langue detecter sur l'OS. (English si non disponible.)
Language Language_getSystemLanguage(void) {
    NSString *langISO = [NSLocale currentLocale].languageCode;
    if(langISO == NULL) {
        printerror("Locale language not defined. Taking default: %u.", default_language_);
        return default_language_;
    }
    // Cas du chinois
    if([langISO compare:@"zh"] == NSOrderedSame) {
        NSString* scriptCode = NSLocale.currentLocale.scriptCode;
        langISO = [NSString stringWithFormat:@"zh-%@", (scriptCode ? scriptCode : @"Hans")];
        Language language = Language_getLanguageWithIso(langISO.UTF8String);
        if(language == default_language_) {
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
        if(strcmp(language_iso_strings_[language], iso) == 0) {
            return language;
        }
    }
    printwarning("Language with iso %s not found.", iso);
    return default_language_;
}
Language Language_current(void) {
    return current_language_;
}
void     Language_setCurrent(Language newCurrentLanguage) {
    if(newCurrentLanguage == current_language_)
        return;
    if(newCurrentLanguage >= language_total_language) {
        printerror("Language undefined.");
        return;
    }
    const char* iso = language_iso(newCurrentLanguage);
    NSBundle* bundle_tmp = getBundleForIsoOpt_(iso);
    if(!bundle_tmp) {
        printerror("No bundle for language %s.", iso);
        return;
    }
    current_bundle_ = bundle_tmp;
    current_language_ = newCurrentLanguage;
}
bool     Language_currentIs(Language other) {
    return other == current_language_;
}
/// De droite a gauche. Vrai pour l'arabe.
bool     Language_currentIsRightToLeft(void) {
    return current_language_ == language_arabic;
}
/// Direction d'ecriture. Arabe -1, autre +1.
float    Language_currentDirectionFactor(void) {
    return (current_language_ == language_arabic) ? -1.f : 1.f;
}
const char* Language_currentIso(void) {
    return language_iso_strings_[current_language_];
}

/// Le code iso, e.g. english -> en.
const char*    language_iso(Language language) {
    return language_iso_strings_[language];
}
/// Le nom de la langue en anglais, e.g. language_english -> english.
const char*    language_name(Language language) {
    return language_name_strings_[language];
}


/*-- Pour la localisation des strings. ----------------*/
/// Localization d'une string dans la langue courante.
/// Apple : Utilise le Bundle de l'app et la resource Localizable.strings.
char* String_createLocalized(const char* stringKey) {
    NSString* key = [NSString stringWithUTF8String:stringKey];
    // ("Localizable" est la "table" utilisée par défaut...)
    NSString* localized = [current_bundle_ localizedStringForKey:key value:@"⁉️" table:nil];
    if(![localized  isEqual: @"⁉️"])
        return String_createCopy([localized UTF8String]);
    // Essayer avec le default bundle (english).
    if(current_bundle_ != default_bundle_) {
        localized = [default_bundle_ localizedStringForKey:key value:@"⁉️" table:nil];
        if(![localized  isEqual: @"⁉️"])
            return String_createCopy([localized UTF8String]);
    }
    printerror("Cannot localized %s.", stringKey);
    return String_createCopy(stringKey);
}
/// Version par defaut de la string, e.g. localization anglaise.
char* String_createLocalizedDefault(const char* stringKey) {
    NSString* key = [NSString stringWithUTF8String:stringKey];
    NSString* localized = [default_bundle_ localizedStringForKey:key value:@"⁉️" table:nil];
    if(![localized  isEqual: @"⁉️"]) {
        return String_createCopy([localized UTF8String]);
    }
    printerror("Cannot localized %s in english.", stringKey);
    return String_createCopy(stringKey);
}
