//
//  utils_locales.h
//  Optenir les `locales` de l'OS.
//  N'utilise que `Foundation.h` (pour Apple)
//
//  Created by Corentin Faucher on 2024-02-28.
//

#ifndef utils_locales_h
#define utils_locales_h

void        CoqLocale_update(void);
const char* CoqLocale_getCountryCode(void);
const char* CoqLocale_getLanguageCode(void);
const char* CoqLocale_getScriptCode(void);

#endif /* utils_locales_h */
