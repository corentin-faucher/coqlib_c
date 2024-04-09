//
//  utils_string.h
//  Struct et fonction utiles pour manipuler les strings.
//
//  Created by Corentin Faucher on 2023-10-25.
//

#ifndef COQ_UTILS_STRING_H
#define COQ_UTILS_STRING_H

#include <string.h> // strcat, strcmp, etc.
#include "../maths/math_base.h"

/// Structure pratique pour passer une série de string.
/// Il s'agit de references unowned/shared, i.e. ne doivent pas être deallocated.
typedef struct {
    const char* const* stringArray;
    size_t             count;
} SharedStringsArray;

// Flag pour l'init de la texture (flag commun avec Texture)
enum {
    // Cherche la version localisée dans les .strings des differentes langues.
    string_flag_localized    = 0x0001,
    // Si `shared` la texture retourné sera toujours la même pour une même string.
    string_flag_shared       = 0x0002,
    // On a le droit d'éditer la texture-string.
    string_flag_mutable      = 0x0004,
    // Style pixélisé.
    string_flag_nearest      = 0x0008,
};
/// Structure temporaire pous passer une string devant être dessinée.
/// (Ici, les string sont des pointeurs unowned/shared, i.e. ne seront pas deallocated.)
typedef struct {
    const char* c_str;
    /// Nom d'une police de caractères custom à utiliser (sinon prend la font current).
    const char* fontNameOpt;
    /// On peut laisser à (0, 0, 0), i.e. noir par défaut et ne tient pas compte d'alpha.
    Vector4     color;
    uint32_t    string_flags; // (Voir ci-dessus)
    /// Espacement supplémentaire sur les côtés (en % de la hauteur). Typiquement 0.5 est bien.
    float       x_margin;
} StringDrawable;

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
// Superflu ? `Juste while(**ref) (*ref)++;`
//void   charRef_moveToEnd(const char** ref);

/*-- Edition string utf8 ---------------------------------------*/

void   stringUTF8_deleteLastChar(char* c_str);

/*-- Getters string utf8 ---------------------------------------*/

/// Nombre de char (utf8) dans la string.
size_t stringUTF8_lenght(const char* c_str);
bool   stringUTF8_isSingleEmoji(const char* c_str);

#endif /* string_utils_h */
