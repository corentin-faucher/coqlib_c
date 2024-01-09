//
//  coq_language.h
//  Pour la localisation des strings...
//
//  Created by Corentin Faucher on 2023-10-25.
//

#ifndef _coq_language_h
#define _coq_language_h

#include "_utils_.h"

typedef enum _Language {
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
    _language_undefined,
} Language;

void     Language_init(void);
Language Language_current(void);
void     Language_setCurrent(Language newCurrentLanguage);
/// La langue detecter sur l'OS. (English si non disponible.)
Language Language_getSystemLanguage(void);
/// Obtenir l'enum Language a partir du code iso, e.g. "en" -> language_english.
Language Language_getLanguageWithIso(const char* const iso);
//Language Language_getLanguageWithIso(char* iso);
bool     Language_currentIs(Language other);
/// De droite a gauche. Vrai pour l'arabe.
bool     Language_currentIsRightToLeft(void);
/// Direction d'ecriture. Arabe -1, autre +1.
float          Language_currentDirectionFactor(void);
const char*    Language_currentIso(void);
/// Le code iso, e.g. language_english -> "en".
const char*    language_iso(Language language);
const char*    language_name(Language language);

/*-- Pour la localisation des strings. ----------------*/
/// Localization d'une string dans la langue courante.
/// Apple : Utilise le Bundle de l'app et la resource Localizable.strings.
char*    String_createLocalized(const char* stringKey);
/// Version par defaut de la string, e.g. localization anglaise.
char*    String_createLocalizedDefault(const char* stringKey);


#endif /* language_h */
