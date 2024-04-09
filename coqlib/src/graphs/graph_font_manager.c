//
//  font_manager.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//

#include "graph_font_manager.h"
#include "../coq_map.h"

static const CoqFontInfo fontInfo_arr_[] = {
    {"American Typewriter", "Amer. Typ.",  {{1.f, 1.2f }}},
    {"Chalkboard SE", "Chalkboard", {{1, 1.25}}},
    {"Chalkduster", "Chalkduster", {{1.2, 1.55}}},
    {"Comic Sans MS", "Comic Sans", {{1, 1.45}}},
    {"Courier", "Courier", {{0.5, 1.3}}},
    {"Futura", "Futura", {{0.0, 1.4}}},
    {"Helvetica", "Helvetica", {{0.3, 1.1}}},
    {"Luciole", "Luciole", {{0.6, 1.3}}},
    {"Snell Roundhand", "Snell Round.", {{6, 1.85}}},
    {"Times New Roman", "Times New R.", {{0.8, 1.3}}},
    {"Verdana", "Verdana", {{0.5, 1.2}}},
    {"Nanum Gothic", "NanumGothic", {{0.2, 1.3}}},
    //    {"Nanum Myeongjo", "NanumMyeongjo", 1.f, 1.2f},
    {"Nanum Pen Script", "Nanum Pen", {{0.2, 1.3}}},
    {"BM Kirang Haerang", "Kirang",   {{0, 1.3}}},
    {"GungSeo", "GungSeo",            {{0.8, 1.3}}},
    {"PilGi", "PilGi",                {{0.3, 1.6}}},
    {"Hiragino Maru Gothic ProN", "Hiragino MGP",{{0.5, 1.3}}},
    {"Klee", "Klee", {{0.2, 1.3}}},
    {"OpenDyslexic3", "Op. Dyslex3", {{1.0, 1.4}}},
    {"Osaka", "Osaka", {{0.5, 1.3}}},
    {"Toppan Bunkyu Gothic", "Toppan BG", {{0.5, 1.3}}},
    {"Toppan Bunkyu Midashi Gothic", "Toppan BMG", {{0.5, 1.3}}},
    {"Toppan Bunkyu Midashi Mincho", "Toppan BMM", {{0.5, 1.3}}},
    {"Toppan Bunkyu Mincho", "Toppan BM", {{0.5, 1.3}}},
    {"Tsukushi A Round Gothic", "Tsukushi A", {{0.5, 1.3}}},
    {"Tsukushi B Round Gothic", "Tsukushi B", {{0.5, 1.3}}},
    {"YuKyokasho Yoko", "YuKyokasho", {{0.5, 1.3}}},
    {"YuMincho", "YuMincho", {{0.5, 1.3}}},
    {"Apple SD Gothic Neo", "Apple SD Goth.N.", {{0.5, 1.3}}},
    {"Apple LiSung", "Apple LiSung", {{0.8, 1.3}}},
    {"Baoli SC", "Baoli", {{0.1, 1.3}}},
    {"GB18030 Bitmap", "Bitmap", {{0.5, 1}}},
    {"HanziPen SC", "HanziPen", {{0.5, 1.3}}},
    {"Hei", "Hei", {{0.25, 1.3}}},
    {"LingWai TC", "LingWai", {{0.9, 1.2}}},
    {"AppleMyungjo", "Myungjo", {{0.5, 1.25}}},
    {"PingFang SC", "PingFang", {{0.5, 1.3}}},
    {"Weibei SC", "Weibei", {{0.5, 1.3}}},
    {"Farah", "Farah", {{0.5, 1.3}}},
};
StringMap* _fontInfoOfName = NULL;

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

void            Font_init(void) {
    if(_fontInfoOfName)
        return;
    _fontInfoOfName = Map_create(40, sizeof(CoqFontInfo));
    size_t count = sizeof(fontInfo_arr_)/sizeof(CoqFontInfo);
    const CoqFontInfo* p = &fontInfo_arr_[0];
    const CoqFontInfo* end = &fontInfo_arr_[count];
    while(p < end) {
        map_put(_fontInfoOfName, p->name, p);
        p++;
    }
}
SharedStringsArray Font_allFontNamesForLanguage(Language language) {
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
const char*        Font_defaultFontNameForLanguage(Language language) {
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
const CoqFontInfo* Font_getFontInfoOf(const char* fontName) {
    if(_fontInfoOfName == NULL) {
        printerror("Font manager not init.");
        return &fontInfo_arr_[0];
    }
    const CoqFontInfo* ci = (const CoqFontInfo*)map_valueRefOptOfKey(_fontInfoOfName, fontName);
    if(ci == NULL) {
        printwarning("Font %s not found.", fontName);
        return &fontInfo_arr_[0];
    }
    return ci;
}

void _test_font_manager(void) {
    SharedStringsArray sa = Font_allFontNamesForLanguage(language_japanese);
    const char*const*  p = sa.stringArray;
    const char*const* end = &sa.stringArray[sa.count];
    printf("Fontnames for japanese.\n");
    while(p < end) {
        printf("%s,\n", *p);
        p++;
    }
}
