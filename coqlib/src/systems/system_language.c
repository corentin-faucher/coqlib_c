//
//  util_language.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-10-07.
//
#include "system_language.h"

#include "system_locale.h"
#include "../utils/util_base.h"
#include "../utils/util_map.h"

const Language  Language_defaultLanguage = language_english;
static Language        language_system_ =     language_english;
static Language        language_current_ =    language_english;
const char*    language_iso_strings_[language_total_language] = {
    "fr", "en", "ja", "de",
    "zh-Hans", "it", "es", "ar",
    "el", "ru", "sv", "zh-Hant",
    "pt", "ko", "vi",
};
const char*    language_code_strings_[language_total_language] = {
    "fr", "en", "ja", "de",
    "zh", "it", "es", "ar",
    "el", "ru", "sv", "zh",
    "pt", "ko", "vi",
};
const char*    language_name_strings_[language_total_language] = {
    "french", "english", "japanese", "german",
    "chinese simpl.", "italian", "spanish", "arabic",
    "greek", "russian", "swedish", "chinese trad.",
    "portuguese", "korean", "vietnamese",
};

void     Language_checkSystemLanguage(void) {
    const char* langCode = CoqLocale_getLanguageCode();
    if(strcmp(langCode, "zh") == 0) {  // Cas du chinois
        const char* scriptCode = CoqLocale_getScriptCode();
        if(strcmp(scriptCode, "Hant") == 0) {
            language_system_ = language_chineseTraditional;
        } else {
            language_system_ = language_chineseSimplified;
        }
    } else {
        language_system_ = Language_languageWithIso(langCode);
    }
}
Language Language_systemLanguage(void) {
    return language_system_;
}
Language Language_languageWithIso(const char* const iso) {
    for(Language language = 0; language < language_total_language; language++) {
        if(strcmp(language_iso_strings_[language], iso) == 0) {
            return language;
        }
    }
    printwarning("Language with iso %s not found.", iso);
    return Language_defaultLanguage;
}
Language Language_languageWithCode(const char* const code) {
    for(Language language = 0; language < language_total_language; language++) {
        if(strcmp(language_code_strings_[language], code) == 0) {
            return language;
        }
    }
    printwarning("Language with code %s not found.", code);
    return Language_defaultLanguage;
}
void     Language_setCurrent(Language newCurrentLanguage) {
    if(newCurrentLanguage >= language_total_language) {
        // language_undefined_language -> Get default.
        newCurrentLanguage = Language_systemLanguage();
    }
    if(newCurrentLanguage == language_current_)
        return;
    if(Language_system_tryToSetTo_(newCurrentLanguage))
        language_current_ = newCurrentLanguage;
}
Language Language_current(void) {
    return language_current_;
}
bool     Language_currentIs(Language const other) {
    return other == language_current_;
}
/// De droite a gauche. Vrai pour l'arabe.
bool     Language_currentIsRightToLeft(void) {
    return language_current_ == language_arabic;
}
/// Utilise un maru `◯` au lieu du check `✓` pour une bonne réponse (japonaise et corréen).
bool     Language_currentUseMaruCheck(void) {
    return (language_current_ == language_japanese) || (language_current_ == language_korean);
}
/// Direction d'ecriture. Arabe -1, autre +1.
float    Language_currentDirectionFactor(void) {
    return (language_current_ == language_arabic) ? -1.f : 1.f;
}
const char* Language_currentIso(void) {
    return language_iso_strings_[language_current_];
}
const char* Language_currentCode(void) {
    return language_code_strings_[language_current_];
}
/// Le code iso, e.g. english -> en.
const char*    language_iso(Language language) {
    return language_iso_strings_[language];
}
/// Le nom de la langue en anglais, e.g. language_english -> english.
const char*    language_name(Language language) {
    return language_name_strings_[language];
}

// MARK: - Font possibles par langues - 
static const char* const _defaultFontNames[] = {
    "American Typewriter",
    "Chalkboard SE",
    "Chalkduster",
    "Comic Sans MS",
    "Courier",
    "Futura",
    "Helvetica",
    "Luciole",
    "OpenDyslexic3",
    "PilGi",
    "Snell Roundhand",
    "Times New Roman",
    "Verdana",
};
static const char* const _japaneseFontNames[] = {
    "Hiragino Maru Gothic ProN",  // 0 -> defaut
    "American Typewriter",
    "Chalkboard SE",
    "Courier",
    "Futura",
    "Helvetica",
    "Klee",
    "LingWai TC",
    "OpenDyslexic3",
    "Osaka",
    "Toppan Bunkyu Gothic",
    "Toppan Bunkyu Midashi Gothic",
    "Toppan Bunkyu Midashi Mincho",
    "Toppan Bunkyu Mincho",
    "Tsukushi A Round Gothic",
    "Tsukushi B Round Gothic",
    "Verdana",
    "YuGothic",
    "YuKyokasho Yoko",
    "YuMincho",
};
static const char* const _koreanFontNames[] = {
    "AppleMyungjo",  // 0 -> defaut
    "Apple SD Gothic Neo",
    "BM Jua",
    "BM Kirang Haerang",
    "BM Yeonsung",
    "GungSeo",
    "HeadLineA",
    "Nanum Gothic",
//    "Nanum Myeongjo",
    "Nanum Pen Script",
    "PilGi",
};
static const char* const _arabicFontNames[] = {
    "American Typewriter",
    "Courier",
    "Farah",
};
static const char* const _chineseFontNames[] = {
    "American Typewriter",
    "Apple LiSung",
    "Baoli SC",
    "Chalkboard SE",
    "Courier",
    "Futura",
    "GB18030 Bitmap",
    "HanziPen SC",
    "Hei",
    "LingWai TC",
    "OpenDyslexic3",
    "PilGi",
    "PingFang SC",
    "Times New Roman",
    "Verdana",
    "Weibei SC",
};
static const char* const _russianFontNames[] = {
    "American Typewriter",
    "Chalkboard SE",
    "Helvetica",
    "Snell Roundhand",
    "Times New Roman",
    "Verdana",
};
static const char* const _greekFontNames[] = {
    "American Typewriter",
    "Chalkboard SE",
    "Helvetica",
    "Luciole",
    "Snell Roundhand",
    "Times New Roman",
    "Verdana",
};

SharedStringsArray LanguageFont_allFontNamesForLanguage(Language language) {
    switch(language) {
        case language_japanese:
            return (SharedStringsArray) {_japaneseFontNames, sizeof(_japaneseFontNames) / sizeof(char*)};
        case language_korean:
            return (SharedStringsArray) {_koreanFontNames, sizeof(_koreanFontNames) / sizeof(char*)};
        case language_arabic:
            return (SharedStringsArray) {_arabicFontNames, sizeof(_arabicFontNames) / sizeof(char*)};
        case language_chineseSimplified:
        case language_chineseTraditional:
            return (SharedStringsArray) {_chineseFontNames, sizeof(_chineseFontNames) / sizeof(char*)};
        case language_russian:
            return (SharedStringsArray) {_russianFontNames, sizeof(_russianFontNames) / sizeof(char*)};
        case language_greek:
            return (SharedStringsArray) {_greekFontNames, sizeof(_greekFontNames) / sizeof(char*)};
        default:
            return (SharedStringsArray) {_defaultFontNames, sizeof(_defaultFontNames) / sizeof(char*)};
    }
}
const char*        LanguageFont_defaultFontNameForLanguage(Language language) {
    switch(language) {
        case language_japanese:
            return _japaneseFontNames[0];
        case language_korean:
            return _koreanFontNames[0];
        case language_arabic:
            return _arabicFontNames[0];
        case language_chineseSimplified:
        case language_chineseTraditional:
            return _chineseFontNames[0];
        case language_russian:
            return _russianFontNames[0];
        case language_greek:
            return _greekFontNames[0];
        default:
            return _defaultFontNames[0];
    }
}
typedef char Char16Arr_[16]; // Test de typedef de array... probablement pas une bonne idée en général.
typedef struct { char name[32]; Char16Arr_ shortName; } StringPair_;
static const StringPair_ fontInfo_arr_[] = {
    {"American Typewriter", "Amer. Typ.",},
    {"Chalkboard SE", "Chalkboard", },
    {"Chalkduster", "Chalkduster",},
    {"Comic Sans MS", "Comic Sans", },
    {"Courier", "Courier",},
    {"Futura", "Futura",},
    {"Helvetica", "Helvetica", },
    {"Luciole", "Luciole", },
    {"Snell Roundhand", "Snell Round.",  },
    {"Times New Roman", "Times New R.",  },
    {"Verdana", "Verdana", },
    {"Nanum Gothic", "NanumGothic",},
    //    {"Nanum Myeongjo", "NanumMyeongjo", 1.f, 1.2f},
    {"Nanum Pen Script", "Nanum Pen", },
    {"BM Kirang Haerang", "Kirang",   },
    {"GungSeo", "GungSeo",            },
    {"PilGi", "PilGi",          },
    {"Hiragino Maru Gothic ProN", "Hiragino MGP",},
    {"Klee", "Klee", },
    {"OpenDyslexic3", "Op. Dyslex3", },
    {"Osaka", "Osaka", },
    {"Toppan Bunkyu Gothic", "Toppan BG", },
    {"Toppan Bunkyu Midashi Gothic", "Toppan BMG", },
    {"Toppan Bunkyu Midashi Mincho", "Toppan BMM", },
    {"Toppan Bunkyu Mincho", "Toppan BM", },
    {"Tsukushi A Round Gothic", "Tsukushi A", },
    {"Tsukushi B Round Gothic", "Tsukushi B", },
    {"YuKyokasho Yoko", "YuKyokasho", },
    {"YuMincho", "YuMincho", },
    {"Apple SD Gothic Neo", "Apple SD Goth.N.", },
    {"Apple LiSung", "Apple LiSung", },
    {"Baoli SC", "Baoli", },
    {"GB18030 Bitmap", "Bitmap", },
    {"HanziPen SC", "HanziPen", },
    {"Hei", "Hei", },
    {"LingWai TC", "LingWai", },
    {"AppleMyungjo", "Myungjo", },
    {"PingFang SC", "PingFang", },
    {"Weibei SC", "Weibei", },
    {"Farah", "Farah", },
};
const char* LanguageFont_getFontShortName(const char* fontNameOpt) {
    if(!fontNameOpt) return NULL;
    static StringMap* fontInfoOfNamed = NULL;
    // Init ?
    if(fontInfoOfNamed == NULL) {
        fontInfoOfNamed = Map_create(40, sizeof(Char16Arr_));
        const StringPair_* p =   fontInfo_arr_;
        const StringPair_* end = coq_simpleArrayEnd(fontInfo_arr_, StringPair_);
        while(p < end) {
            map_put(fontInfoOfNamed, p->name, p->shortName);
            p++;
        }
    }
    char const*const shortName = map_valueRefOptOfKey(fontInfoOfNamed, fontNameOpt);
    if(shortName == NULL) {
        printwarning("Font %s not found.", fontNameOpt);
        return NULL;
    }
    return shortName;
}

void Language_test_fontLanguage_(void) {
    SharedStringsArray sa = LanguageFont_allFontNamesForLanguage(language_japanese);
    const char*const*  p = sa.stringArray;
    const char*const* end = &sa.stringArray[sa.count];
    printf("Fontnames for japanese.\n");
    while(p < end) {
        printf("%s,\n", *p);
        p++;
    }
}
