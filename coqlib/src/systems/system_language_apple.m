//
//  util_language.m
//
//  Created by Corentin Faucher on 2023-10-25.
//
#import <Foundation/Foundation.h>

#include "system_language.h"

#include "system_locale.h"
#include "../utils/util_base.h"
#include "../utils/util_string.h"

// MARK: - Langue et région

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

void     Language_init_(void) {
    // Init app bundle.
    if(default_bundle_ != nil) { printerror("Language already init."); return; }
    default_bundle_ = getBundleForIsoOpt_("en");
    if(!default_bundle_) default_bundle_ = [NSBundle mainBundle];
    current_bundle_ = default_bundle_;
    // Init system language
    Language_checkSystemLanguage();
    Language_setCurrent(Language_systemLanguage());
}
/// Mettre le bundle dans la bonne langue...
bool     Language_system_tryToSetTo_(Language const language) {
    const char* iso = language_iso(language);
    NSBundle* bundle_tmp = getBundleForIsoOpt_(iso);
    if(!bundle_tmp) {
        printerror("No bundle for language %s.", iso);
        return false;
    }
    current_bundle_ = bundle_tmp;
    return true;
}

/*-----------------------------------------------------*/
// MARK: - Localisation des strings

/// Localization d'une string dans la langue courante.
/// Apple : Utilise le Bundle de l'app et la resource Localizable.strings.
const char* String_createLocalized(const char* stringKey) {
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
void  String_copyLocalizedTo(const char*const stringKey, char*const buffer, size_t const size_max_opt) {
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
const char* String_createLocalizedDefault(const char* stringKey) {
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
