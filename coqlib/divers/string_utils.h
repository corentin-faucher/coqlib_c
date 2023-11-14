//
//  string_utils.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-25.
//

#ifndef string_utils_h
#define string_utils_h

#include "utils.h"

/// Structure pratique pour passer une série de string.
/// Il s'agit de references unowned/shared, i.e. ne doivent pas être deallocated.
typedef struct {
    const char* const* stringArray;
    const size_t       count;
} SharedStringsArray;
/// Structure pratique pour passer une string localized (ou non), avec(sans) font spécifique.
/// Il s'agit de references unowned/shared, i.e. ne doivent pas être deallocated.
typedef struct {
    const char* c_str;
    const char* fontNameOpt;
    Bool  isLocalized;
} UnownedString;

char* string_createCopy(const char* src);
Bool  string_startWithPrefix(const char* c_str, const char* prefix);

#endif /* string_utils_h */
