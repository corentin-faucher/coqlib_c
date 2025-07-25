//
//  system_language.h
//  Pour la localisation des strings...
//
//  Created by Corentin Faucher on 2023-10-25.
//
#ifndef COQ_UTIL_LANGUAGE_H
#define COQ_UTIL_LANGUAGE_H

#include "../utils/util_string.h"

typedef enum {
    language_french,
    language_english,
    language_japanese,
    language_german,
    
    language_chineseSimplified,
    language_italian,
    language_spanish,
    language_arabic,
    
    language_greek,
    language_russian,
    language_swedish,
    language_chineseTraditional,
    
    language_portuguese,
    language_korean,
    language_vietnamese,
    // Autres langues importantes... ?
    // language_hindi,
    // language_swahili,
    // language_indonesian,
    // language_dutch,
    
    language_total_language,
    language_undefined_language,
} Language;

extern const Language Language_defaultLanguage; // language_english

/// (private -> init inclus dans `CoqSystem_init`)
void     Language_init_(void);
bool     Language_system_tryToSetTo_(Language const language);

/// Sychronise avec la langue actuel du system.
void     Language_checkSystemLanguage(void);
/// Langue de l'OS
Language Language_systemLanguage(void);
/// Langue setter par l'usager.
Language Language_current(void);
/// Setter la langue courante, init par défaut à celle de l'OS (ou anglais...)
void     Language_setCurrent(Language newCurrentLanguage);
bool     Language_currentIs(Language other);
/// De droite a gauche. Vrai pour l'arabe.
bool     Language_currentIsRightToLeft(void);
/// Direction d'ecriture. Arabe -1, autre +1.
float    Language_currentDirectionFactor(void);
/// Utilise un maru `◯` au lieu du check `✓` pour une bonne réponse (japonaise et corréen).
bool     Language_currentUseMaruCheck(void);
const char* Language_currentIso(void);
const char* Language_currentCode(void);
/// Obtenir l'enum Language a partir du code iso, e.g. "en" -> `language_english`.
Language    Language_languageWithIso(const char* iso);
/// Semblable à `Language_languageWithIso` mais ne contient que le code de langue.
/// Différent seulement pour le chinois où "zh" retourne `language_chineseSimplified`.
Language    Language_languageWithCode(const char* const code);
/// Le code iso, e.g. language_english -> "en".
const char* language_iso(Language language);
const char* language_name(Language language);

// MARK: - Pour la localisation des strings. ----------------
/// Localization d'une string dans la langue courante.
/// Apple : Utilise le Bundle de l'app et la resource Localizable.strings.
const char* String_createLocalized(const char* stringKey);
void        String_copyLocalizedTo(const char* stringKey, char* dstBuffer, size_t size_max_opt);
/// Version par defaut de la string, e.g. localization anglaise.
const char* String_createLocalizedDefault(const char* stringKey);

// MARK: - Les fonts possible pour les différentes langues -
SharedStringsArray LanguageFont_allFontNamesForLanguage(Language language);
const char*        LanguageFont_defaultFontNameForLanguage(Language language);
const char*        LanguageFont_getFontShortName(const char* fontNameOpt);

void Language_test_fontLanguage_(void);


#endif
