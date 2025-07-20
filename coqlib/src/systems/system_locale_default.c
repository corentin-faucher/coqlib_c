//
//  util_locales_default.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-04-09.
//

#include "system_locale.h"

#define     coqlocale_default_countryCode_ "US"
#define     coqlocale_default_languageCode_ "en"

void        CoqLocale_update(void) {
    // pass
}
const char* CoqLocale_getCountryCode(void) {
    return coqlocale_default_countryCode_;
}
const char* CoqLocale_getLanguageCode(void) {
    return coqlocale_default_languageCode_;
}
const char* CoqLocale_getScriptCode(void) {
    return "";
}
