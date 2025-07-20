//
//  util_string.h
//  Struct et fonction utiles pour manipuler les strings.
//
//  Created by Corentin Faucher on 2023-10-25.
//
#ifndef COQ_UTIL_STRING_H
#define COQ_UTIL_STRING_H

#include <string.h> // strcat, strcmp, etc.
#include "../maths/math_base.h"

/// Structure pratique pour passer une série de string.
/// Il s'agit de references unowned/shared, i.e. ne doivent pas être deallocated.
typedef struct {
    char const*const* stringArray;
    size_t            count;
} SharedStringsArray;

/*-- Création de strings utf8 ---------------------------------------*/

char* String_createCopy(const char* src);
char* String_createCat(const char* src1, const char* src2);
char* String_createCat3(const char* src1, const char* src2, const char* src3);

bool  string_startWithPrefix(const char* c_str, const char* prefix);

/*-- Navigation dans string utf8 ---------------------------------------*/
// Ici on met des `char*` et non des `const char*` 
// car la navigation est pour éditer les strings, a priori...
size_t charRef_sizeAsUTF8(const char* ref);    // (ici ça change pas grand chose const ou non)
void   charRef_moveToNextUTF8Char(char** ref);
void   charRef_moveToPreviousUTF8Char(char** ref);
void   charconstRef_moveToNextUTF8Char(char const** ref);
// Superflu ? `Juste while(**ref) (*ref)++;`
//void   charRef_moveToEnd(const char** ref);

/*-- Edition string utf8 ---------------------------------------*/
void   stringUTF8_deleteLastChar(char* c_str);

/*-- Getters string utf8 ---------------------------------------*/
/// Nombre de char (utf8) dans la string.
size_t stringUTF8_lenght(const char* c_str);
bool   stringUTF8_isSingleEmoji(const char* c_str);

#endif /* string_util_h */
