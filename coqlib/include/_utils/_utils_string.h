//
//  _utils_string.h
//  Struct et fonction utiles pour manipuler les strings.
//
//  Created by Corentin Faucher on 2023-10-25.
//

#ifndef _coq_utils_string_h
#define _coq_utils_string_h

#include "_utils/_utils_.h"

/// Structure pratique pour passer une série de string.
/// Il s'agit de references unowned/shared, i.e. ne doivent pas être deallocated.
typedef struct {
    const char* const* stringArray;
    const size_t       count;
} SharedStringsArray;
/// Structure pratique pour passer une string localized (ou non), avec (ou sans) font spécifique.
/// Il s'agit de references unowned/shared, i.e. ne doivent pas être deallocated.
typedef struct {
    const char* c_str;
    const char* fontNameOpt;
    bool  isLocalized;
} UnownedString;

char* String_createCopy(const char* src);
char* String_createCat(const char* src1, const char* src2);
bool  string_startWithPrefix(const char* c_str, const char* prefix);

/*-- Navigation dans string utf8 ---------------------------------------*/
size_t stringUTF8_charSizeAt(const char* c_str, size_t pos);
void   stringUTF8_moveToNextChar(const char* c_str, const char** ref);
void   stringUTF8_setToLastChar(const char* c_str, const char** ref);
void   stringUTF8_moveToPreviousChar(const char* c_str, const char** ref);
void   stringUTF8_deleteLastChar(char* const c_str);
/// Nombre de char (utf8) dans la string.
size_t stringUTF8_lenght(const char* c_str);

bool   stringUTF8_isSingleEmoji(const char* c_str);

#endif /* string_utils_h */
