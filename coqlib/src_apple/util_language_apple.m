//
//  util_language.m
//
//  Created by Corentin Faucher on 2023-10-25.
//
#import <Foundation/Foundation.h>

#include "util_language.h"
#include "util_base.h"
#include "util_locale.h"
#include "util_string.h"

#pragma mark - Langue et région

static const Language  language_default_ =    language_english;
static Language        language_system_ =     language_english;
static Language        language_current_ =    language_english;
const char*    language_iso_strings_[language_total_language] = {
    "fr", "en", "ja", "de",
    "zh-Hans", "it", "es", "ar",
    "el", "ru", "sv", "zh-Hant",
    "pt", "ko", "vi",
};
const char*    language_code_strings_[language_total_language] = {
    "fr", "en", "ja", "de",
    "zh", "it", "es", "ar",
    "el", "ru", "sv", "zh",
    "pt", "ko", "vi",
};
const char*    language_name_strings_[language_total_language] = {
    "french", "english", "japanese", "german",
    "chinese simpl.", "italian", "spanish", "arabic",
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

void     Language_checkSystemLanguage(void) {
    const char* langCode = CoqLocale_getLanguageCode();
    if(strcmp(langCode, "zh") == 0) {  // Cas du chinois
        const char* scriptCode = CoqLocale_getScriptCode();
        if(strcmp(scriptCode, "Hant") == 0) {
            language_system_ = language_chineseTraditional;
        } else {
            language_system_ = language_chineseSimplified;
        }
    } else {
        language_system_ = Language_languageWithIso(langCode);
    }
}
Language Language_systemLanguage(void) {
    return language_system_;
}
void     Language_init(void) {
    // Init app bundle.
    if(default_bundle_ != nil) { printerror("Language already init."); return; }
    default_bundle_ = getBundleForIsoOpt_("en");
    if(!default_bundle_) default_bundle_ = [NSBundle mainBundle];
    current_bundle_ = default_bundle_;
    // Init system language
    Language_checkSystemLanguage();
    Language_setCurrent(language_system_);
}

void     Language_setCurrent(Language newCurrentLanguage) {
    if(newCurrentLanguage >= language_total_language) {
        // language_undefined_language -> Get default.
        newCurrentLanguage = language_system_;
    }
    if(newCurrentLanguage == language_current_)
        return;
    
    const char* iso = language_iso(newCurrentLanguage);
    NSBundle* bundle_tmp = getBundleForIsoOpt_(iso);
    if(!bundle_tmp) {
        printerror("No bundle for language %s.", iso);
        return;
    }
    current_bundle_ = bundle_tmp;
    language_current_ = newCurrentLanguage;
}
Language Language_languageWithIso(const char* const iso) {
    for(Language language = 0; language < language_total_language; language++) {
        if(strcmp(language_iso_strings_[language], iso) == 0) {
            return language;
        }
    }
    printwarning("Language with iso %s not found.", iso);
    return language_default_;
}
Language Language_languageWithCode(const char* const code) {
    for(Language language = 0; language < language_total_language; language++) {
        if(strcmp(language_code_strings_[language], code) == 0) {
            return language;
        }
    }
    printwarning("Language with code %s not found.", code);
    return language_default_;
}
Language Language_current(void) {
    return language_current_;
}
bool     Language_currentIs(Language const other) {
    return other == language_current_;
}
/// De droite a gauche. Vrai pour l'arabe.
bool     Language_currentIsRightToLeft(void) {
    return language_current_ == language_arabic;
}
/// Utilise un maru `◯` au lieu du check `✓` pour une bonne réponse (japonaise et corréen).
bool     Language_currentUseMaruCheck(void) {
    return (language_current_ == language_japanese) || (language_current_ == language_korean);
}
/// Direction d'ecriture. Arabe -1, autre +1.
float    Language_currentDirectionFactor(void) {
    return (language_current_ == language_arabic) ? -1.f : 1.f;
}
const char* Language_currentIso(void) {
    return language_iso_strings_[language_current_];
}
const char* Language_currentCode(void) {
    return language_code_strings_[language_current_];
}
/// Le code iso, e.g. english -> en.
const char*    language_iso(Language language) {
    return language_iso_strings_[language];
}
/// Le nom de la langue en anglais, e.g. language_english -> english.
const char*    language_name(Language language) {
    return language_name_strings_[language];
}


/*-----------------------------------------------------*/
#pragma mark - Localisation des strings

/// Localization d'une string dans la langue courante.
/// Apple : Utilise le Bundle de l'app et la resource Localizable.strings.
char* String_createLocalized(const char* stringKey) {
    if(current_bundle_ == nil) {
        printerror("Language not init.");
        return String_createCopy(stringKey);
    }
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
void  String_copyLocalizedTo(const char* stringKey, char* buffer, size_t size_max_opt) {
    if(current_bundle_ == nil) { printerror("Language not init."); return;}
    if(!buffer) { printerror("No dest. to copy to."); return;}
    NSString* key = [NSString stringWithUTF8String:stringKey];
    // ("Localizable" est la "table" utilisée par défaut...)
    NSString* localized = [current_bundle_ localizedStringForKey:key value:@"⁉️" table:nil];
    if(![localized  isEqual: @"⁉️"]) {
        if(size_max_opt) strncpy(buffer, [localized UTF8String], size_max_opt);
        else strcpy(buffer, [localized UTF8String]);
        return;
    }
    // Essayer avec le default bundle (english).
    if(current_bundle_ != default_bundle_) {
        localized = [default_bundle_ localizedStringForKey:key value:@"⁉️" table:nil];
        if(![localized  isEqual: @"⁉️"]) {
            if(size_max_opt) strncpy(buffer, [localized UTF8String], size_max_opt);
            else strcpy(buffer, [localized UTF8String]);
            return;
        }
    }
    printerror("Cannot localized %s.", stringKey);
    if(size_max_opt) strncpy(buffer, stringKey, size_max_opt);
    else strcpy(buffer, stringKey);
}
/// Version par defaut de la string, e.g. localization anglaise.
char* String_createLocalizedDefault(const char* stringKey) {
    if(default_bundle_ == nil) {
        printerror("Language not init.");
        return String_createCopy(stringKey);
    }
    NSString* key = [NSString stringWithUTF8String:stringKey];
    NSString* localized = [default_bundle_ localizedStringForKey:key value:@"⁉️" table:nil];
    if(![localized  isEqual: @"⁉️"]) {
        return String_createCopy([localized UTF8String]);
    }
    printerror("Cannot localized %s in english.", stringKey);
    return String_createCopy(stringKey);
}
