//
//  utils_locales.m
//  AnimalCounting
//
//  Created by Corentin Faucher on 2024-02-28.
//

#include "utils/utils_locales.h"
#import <Foundation/Foundation.h>

#define     coqlocale_default_countryCode_ "US"
static char CoqLocale_countryCode_[5] = {};
#define     coqlocale_default_languageCode_ "en"
static char CoqLocale_languageCode_[5] = {};
static char CoqLocale_scriptCode_[5] = {};

void CoqLocale_update(void) {
    NSLocale* locale = [NSLocale autoupdatingCurrentLocale];
    NSString *countryCode = [locale countryCode];
    if(countryCode == nil) {
        printf("🐔 System region not defined. Takeng default: %s.", coqlocale_default_countryCode_);
        strncpy(CoqLocale_countryCode_, coqlocale_default_countryCode_, 5);
    } else {
        strncpy(CoqLocale_countryCode_, [countryCode UTF8String], 5);
    }
    NSString *langISO = [locale languageCode];
    if(langISO == nil) {
        printf("🐔System language not defined. Taking default: %s.", coqlocale_default_languageCode_);
        strncpy(CoqLocale_languageCode_, coqlocale_default_languageCode_, 5);
    } else {
        strncpy(CoqLocale_languageCode_, [langISO UTF8String], 5);
    }
    NSString *scriptCode = [locale scriptCode];
    if(scriptCode == nil) {
        memset(CoqLocale_scriptCode_, 0, 5);
    } else {
        strncpy(CoqLocale_scriptCode_, [scriptCode UTF8String], 5);
    }
}

const char* CoqLocale_getCountryCode(void) {
    if(CoqLocale_languageCode_[0] == 0) CoqLocale_update();
    return CoqLocale_countryCode_;
}
const char* CoqLocale_getLanguageCode(void) {
    if(CoqLocale_languageCode_[0] == 0) CoqLocale_update();
    return CoqLocale_languageCode_;
}
const char* CoqLocale_getScriptCode(void) {
    if(CoqLocale_languageCode_[0] == 0) CoqLocale_update();
    return CoqLocale_scriptCode_;
}

