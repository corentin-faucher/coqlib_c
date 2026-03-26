//
//  util_string.h
//  Struct et fonction utiles pour manipuler les strings.
//
//  Created by Corentin Faucher on 2023-10-25.
//
#ifndef COQ_UTIL_CHARS_H
#define COQ_UTIL_CHARS_H

#include <stdbool.h> // pour bool
#include <stddef.h>  // pour size_t
#include <stdint.h>  // pour uint32_t, uint64_t...
// Ajouter #include <string.h> // si besoin... (strcat, strcmp, etc.)

// MARK: - Character, structure d'un caractère quelconque (pas juste ASCII)
#define CHARACTER_MAX_SIZE 7
/// Structure (union) pour mini-string, i.e. typiquement un char en *utf8*.
/// Utiliser `character_toUnicode32` pour conversion de utf8 à unicode 32.
typedef union {
    // A priori un 'Character' est une mini string UTF8 de 8 bytes
    // (entre 4 et 6 dans le cas des emojis et +1 pour le null terminal.)
    char     c_str[CHARACTER_MAX_SIZE + 1];
    uint64_t c_data8;  // Sous la forme d'un data de 64 bits (data en utf8).
    uint32_t c_data4;  // Sous la forme d'un data de 32 bits (data en utf8 de 1 à 4 bytes).
    char     first;    // Ok pour Ascii, i.e. lettre ordinaire sans accent.
} Character;

size_t char_sizeAsUTF8(char const* ref);
// Semblable à `char_sizeAsUTF8`, mais ajoute +3 si suivit d'un VariationSelector (0xEFB88X).
size_t char_sizeAsCaracter(char const* ref);

// Les grandes familles de characters `ordinaires`
enum {
    character_type_latin,
    character_type_greek,     // 1
    character_type_cyrillic,  // 2
    character_type_armenian,  // 3

    character_type_arabic,    // 4
    character_type_korean,    // 5
    character_type_kana,      // 6

    character_type__last_capitalizable_ = character_type_armenian,
};

Character Character_fromUTF8string(char const* c_str);
Character Character_fromUnicode32(uint32_t unicode);
size_t    character_size(Character c);

/// Conversion de utf8 vers unicode 32 (récupère les bits utiles de la mini-string).
uint32_t  character_toUnicode32(Character c);

// Pour le upper/lower, on ne fait que scanner la liste pour l'instant. (Besoin de hash map ?)
Character character_upperCased(Character c, unsigned character_type);
Character character_lowerCased(Character c, unsigned character_type);

bool character_isSpace(Character c);
bool character_isPunct(Character c);
/// Character qui termine un mot (pour détection de fin de ligne)
bool character_isWordFinal(Character c);
bool character_isEndLine(Character c);
bool character_isEmoji(Character c);

// MARK: Les char spéciaux et "importans"
#define spchar_null (Character){ 0 }
#define spchar_delete (Character){ "\b" }
#define spchar_deleteSymbol (Character){ "␈" }
#define spchar_questionMark (Character){ "?" }
#define spchar_tab (Character){ "\t" }
#define spchar_tabSymbol (Character) { "␉" }
#define spchar_return_ (Character) { "\r" }
#define spchar_newline_ (Character) { "\n" }
#define spchar_returnSymbol (Character) { "␍" }
#define spchar_space (Character) { " " }
#define spchar_spaceSymbol (Character) { "␠" }
#define spchar_spaceNobreak (Character) { " " }
#define spchar_spaceIdeographic (Character) { "　" }
#define spchar_spaceThin (Character) { "\u2009" }
#define spchar_bottomBracket (Character) { "⎵" }
#define spchar_underscore (Character) { "_" }
#define spchar_overline (Character) {"‾" }
#define spchar_openBox (Character) { "␣" }
#define spchar_interpunct (Character) { "·" }
#define spchar_dot (Character) { "•" }
#define spchar_butterfly (Character) { "🦋" }
#define spchar_dodo (Character) { "🦤" }
#define spchar_maru (Character) { "◯" }
#define spchar_batu (Character) { "❌" }
#define spchar_checkmark (Character) { "✓" }
#define spchar_greenCheckmark (Character) { "✅" }

// MARK: - String : Array de chars
char* String_createCopy(const char* src);
char* String_createCat(const char* src1, const char* src2);
char* String_createCat3(const char* src1, const char* src2, const char* src3);

char* String_createCopyTrimedOfSpaces(const char* srcOpt);

bool  string_startWithPrefix(const char* c_str, const char* prefix);
/// Nombre de char (utf8) dans la string (incluant les Variation selector).
size_t stringUTF8_lenght(const char* c_str);
void   stringUTF8_deleteLastChar(char* c_str);

// MARK: - String Array
/// Structure pratique pour passer une série de string.
/// Il s'agit de references unowned/shared, i.e. ne doivent pas être deallocated.
typedef struct {
    char const*const* stringArray;
    size_t            count;
} SharedStringsArray;

// MARK: Navigation dans une string utf8
// Ici on met des `char*` et non des `const char*` 
// car la navigation est pour éditer les strings, a priori...
void      charRef_moveToNextUTF8Char(char** ref);
void      charRef_moveToPreviousUTF8Char(char** ref);
void      charconstRef_moveToNextUTF8Char(char const** ref);
Character charconstRef_getCharacterAndMoveToNextCharacter(char const** cRef);

// MARK: - CharacterArray, une liste de caractères
typedef struct   CharacterArray CharacterArray;
CharacterArray*  CharacterArray_createFromString(const char* string);

size_t           characterarray_count(const CharacterArray* charArr);
Character const* characterarray_first(const CharacterArray* charArray);
Character const* characterarray_end(const CharacterArray* charArray);

#endif /* string_util_h */
