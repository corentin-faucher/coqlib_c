//
//  char_and_keycode.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#include "util_char_and_keycode.h"
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

const Character spchar_delete = { "\b" };
const Character spchar_deleteSymbol = { "␈" };
const Character spchar_tab = { "\t" };
const Character spchar_tabSymbol = { "␉" };
const Character spchar_return_ = { "\r" };
const Character spchar_returnSymbol = { "␍" };
const Character spchar_space = { " " };
const Character spchar_spaceSymbol = { "␠" };
const Character spchar_spaceNobreak = { " " };
const Character spchar_spaceIdeographic = { "　" };
const Character spchar_spaceThin = { "\u2009" };
const Character spchar_bottomBracket = { "⎵" };
const Character spchar_underscore = { "_" };
const Character spchar_openBox = { "␣" };
const Character spchar_interpunct = { "·" };
const Character spchar_dot = { "•" };
const Character spchar_butterfly = { "🦋" };
const Character spchar_dodo = { "🦤" };

const uint16_t MKC_keycode_of_mkc[] = {
#if TARGET_OS_OSX == 1
    0x12, 0x13, 0x14, 0x15, 0x17, 0x16, 0x1A, 0x1C, 0x19, 0x1D, 0x1B, 0x18, // 1, 2, 3, ...
    0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x10, 0x20, 0x22, 0x1F, 0x23, 0x21, 0x1E, // Q, W, E, ...
    0x00, 0x01, 0x02, 0x03, 0x05, 0x04, 0x26, 0x28, 0x25, 0x29, 0x27,       // A, S, D, ...
    0x06, 0x07, 0x08, 0x09, 0x0B, 0x2D, 0x2E, 0x2B, 0x2F, 0x2C,             // Z, X, ...
    0x32, 0x0A, 0x5E, 0x5D, 0x2A, 0x2A, keycode_space,
#else
    0x1E,0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, // 1, 2, 3, ...
    0x14,0x1A, 0x08, 0x15, 0x17, 0x1C, 0x18, 0x0C, 0x12, 0x13, 0x2F, 0x30, // Q, W, E, ...
    0x04,0x16, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x33, 0x34,       // A, S, D, ...
    0x1D,0x1B, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38,             // Z, X, ...
    0x35,0x64, 0x87, 0x89, 0x31, 0x32, keycode_space,
#endif
    // ANSI_Grave, ISO_Section, JIS_Underscore, JIS_Yen, ANSI_Backslash, ISO_Backslash, Space
    keycode_delete, keycode_return_, keycode_tab,
    keycode_empty, keycode_empty, keycode_empty, keycode_empty, keycode_empty, // 55~59
    keycode_capsLock, keycode_control, keycode_shift, keycode_option, keycode_command, // 60~64
    keycode_rightControl, keycode_rightShift, keycode_rightOption, keycode_rightCommand, // 65~68
    keycode_empty, keycode_escape, keycode_JIS_Eisu, keycode_JIS_kana, keycode_empty,    // 69~73
    // Clavier numerique
#if TARGET_OS_OSX == 1
    // 74~79  -> ".", enter, "+", "-", "*", "/".
    65, 76, 69, 78, 67, 75,
    // 80~89  -> 0 à 9
    82, 83, 84, 85, 86, 87, 88, 89, 91, 92,
#else
    // 74~79  -> ".", enter, "+", "-", "*", "/".
    99, 88, 87, 86, 85, 84,
    // 80~89  -> 0 à 9
    98, 89, 90, 91, 92, 93, 94, 95, 96, 97,
#endif
    // Flèches
    keycode_arrowLeft, keycode_arrowRight, keycode_arrowDown, keycode_arrowUp,
    // (Reste, 94~99)
    keycode_empty, keycode_empty, keycode_empty, keycode_empty, keycode_empty, keycode_empty,
};

const uint16_t MKC_of_keycode[] = {
#if TARGET_OS_OSX == 1    
    24, 25, 26, 27, 29, 28, 35, 36, 37, 38, 46, 39, 12, 13, 14, 15, 17, 16, 0, 1, 2, 3, 5, 4, 11,
    8, 6, 10, 7, 9, 23, 20, 18, 22, 19, 21, 53, 32, 30, 34, 31, 33, 50, 42, 44, 40, 41, 43,
    54, 51, 45, 52, 99, 70, 68, 64, 62, 60, 63, 61, 66, 67, 65, 99, 99, 74, 99, 78, 99, 76,
    99, 99, 99, 99, 99, 79, 75, 99, 77, 99, 99, 99, 80, 81, 82, 83, 84, 85, 86, 87, 99,
    88, 89, 48, 47, 99, 99, 99, 99, 99, 99, 99, 71, 99, 72, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 90, 91, 92, 93, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 
#else
    99, 99, 99, 99, 24, 39, 37, 26, 14, 27, 28, 29, 19, 30, 31, 32, 41, 40, 20, 21, 12, 15, 25,
    16, 18, 38, 13, 36, 17, 35, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 53, 70, 52, 54, 51, 10, 11, 22, 23,
    49, 50, 33, 34, 45, 42, 43, 44, 60, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 91, 90, 92, 93, 99, 79, 78, 77, 76, 75, 81, 82, 83, 84, 85, 86,
    87, 88, 89, 80, 74, 46, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 47, 99, 48, 99, 99, 99,
    99, 99, 99, 72, 71, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 61, 62, 63, 64, 65, 66, 67, 68, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
#endif
};

const uint16_t MKC_homerow_mkcs[] = {
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
};

const uint16_t MKC_prefered_order_mkcs[] = {
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
    45, 46, 47, 48, 49, 50, 51,
};


void test_print_mkcOfKeycode_(void) {
    uint16_t mkcOfKeycode[256] = {[0 ... 255] = mkc_empty};
    for(uint16_t mkc = 0; mkc < mkc_total_mkcs; mkc ++) {
        uint16_t keycode = MKC_keycode_of_mkc[mkc];
        mkcOfKeycode[keycode] = mkc;
    }
    printf("mkc of keycodes:\n");
    for(uint16_t keycode = 0; keycode < 256; keycode++) {
        printf("%d, ", mkcOfKeycode[keycode]);
    }
    printf("\n🐔\n");
}

const char character_latins_[][4] = {
    // (Cas ASCII superflu...)
//    "A", "a", "B", "b", "C", "c", "D", "d", "E", "e", "F", "f", "G", "g", "H", "h", "I", "i", "J", "j", "K", "k", "L", "l",
//    "M", "m", "N", "n", "O", "o", "P", "p", "Q", "q", "R", "r", "S", "s", "T", "t", "U", "u", "V", "v", "W", "w", "X", "x",
//    "Y", "y", "Z", "z", 
    "À", "à", "Á", "á", "Â", "â", "Ã", "ã", "Ä", "ä", "Å", "å", "Æ", "æ", "Ç", "ç", "È", "è", "É", "é", 
    "Ê", "ê", "Ë", "ë", "Ì", "ì", "Í", "í", "Î", "î", "Ï", "ï", "Ð", "ð", "Ñ", "ñ", "Ò", "ò", "Ó", "ó", "Ô", "ô", "Õ", "õ", 
    "Ö", "ö", "Ø", "ø", "Ù", "ù", "Ú", "ú", "Û", "û", "Ü", "ü", "Ý", "ý", "Þ", "þ", "Ā", "ā", "Ă", "ă", "Ą", "ą", "Ć", "ć", 
    "Ĉ", "ĉ", "Ċ", "ċ", "Č", "č", "Ď", "ď", "Đ", "đ", "Ē", "ē", "Ĕ", "ĕ", "Ė", "ė", "Ę", "ę", "Ě", "ě", "Ĝ", "ĝ", "Ğ", "ğ", 
    "Ġ", "ġ", "Ģ", "ģ", "Ĥ", "ĥ", "Ħ", "ħ", "Ĩ", "ĩ", "Ī", "ī", "Ĭ", "ĭ", "Į", "į", "Ĳ", "ĳ", "Ĵ", "ĵ", "Ķ", "ķ", "Ĺ", "ĺ", 
    "Ļ", "ļ", "Ľ", "ľ", "Ŀ", "ŀ", "Ł", "ł", "Ń", "ń", "Ņ", "ņ", "Ň", "ň", "Ŋ", "ŋ", "Ō", "ō", "Ŏ", "ŏ", "Ő", "ő", "Œ", "œ",
    "Ŕ", "ŕ", "Ŗ", "ŗ", "Ř", "ř", "Ś", "ś", "Ŝ", "ŝ", "Ş", "ş", "Š", "š", "Ţ", "ţ", "Ť", "ť", "Ŧ", "ŧ", "Ũ", "ũ", "Ū", "ū", 
    "Ŭ", "ŭ", "Ů", "ů", "Ű", "ű", "Ų", "ų", "Ŵ", "ŵ", "Ŷ", "ŷ", "Ÿ", "ÿ", "Ź", "ź", "Ż", "ż", "Ž", "ž", "Ɓ", "ɓ", "Ƃ", "ƃ", 
    "Ƅ", "ƅ", "Ɔ", "ɔ", "Ƈ", "ƈ", "Ɗ", "ɗ", "Ƌ", "ƌ", "Ǝ", "ɘ", "Ə", "ə", "Ɛ", "ɛ", "Ƒ", "ƒ", "Ɠ", "ɠ", "Ɣ", "ɣ", "Ɩ", "ɩ", 
    "Ɨ", "ɨ", "Ƙ", "ƙ", "Ɯ", "ɯ", "Ɲ", "ɲ", "Ơ", "ơ", "Ƣ", "ƣ", "Ƥ", "ƥ", "Ƨ", "ƨ", "Ʃ", "ʃ", "Ƭ", "ƭ", "Ʈ", "ʈ", "Ư", "ư", 
    "Ʊ", "ʊ", "Ʋ", "ʋ", "Ƴ", "ƴ", "Ƶ", "ƶ", "Ʒ", "ʒ", "Ƹ", "ƹ", "Ƽ", "ƽ", "Ǆ", "ǆ", "Ǉ", "ǉ", "Ǌ", "ǌ", "Ǎ", "ǎ", "Ǐ", "ǐ", 
    "Ǒ", "ǒ", "Ǔ", "ǔ", "Ǖ", "ǖ", "Ǘ", "ǘ", "Ǚ", "ǚ", "Ǜ", "ǜ", "Ǟ", "ǟ", "Ǡ", "ǡ", "Ǣ", "ǣ", "Ǥ", "ǥ", "Ǧ", "ǧ", "Ǩ", "ǩ", 
    "Ǫ", "ǫ", "Ǭ", "ǭ", "Ǯ", "ǯ", "Ǳ", "ǳ", "Ǵ", "ǵ", "Ǹ", "ǹ", "Ǻ", "ǻ", "Ǽ", "ǽ", "Ǿ", "ǿ", "Ȁ", "ȁ", "Ȃ", "ȃ", "Ȅ", "ȅ", 
    "Ȇ", "ȇ", "Ȉ", "ȉ", "Ȋ", "ȋ", "Ȍ", "ȍ", "Ȏ", "ȏ", "Ȑ", "ȑ", "Ȓ", "ȓ", "Ȕ", "ȕ", "Ȗ", "ȗ", "Ș", "ș", "Ț", "ț", "Ȝ", "ȝ", 
    "Ȟ", "ȟ", "Ƞ", "ƞ", "Ȣ", "ȣ", "Ȥ", "ȥ", "Ȧ", "ȧ", "Ȩ", "ȩ", "Ȫ", "ȫ", "Ȭ", "ȭ", "Ȯ", "ȯ", "Ȱ", "ȱ", "Ȳ", "ȳ", "Ⱥ", "ⱥ", 
    "Ȼ", "ȼ", "Ƚ", "ƚ", "Ⱦ", "ⱦ", "Ɂ", "ɂ", "Ƀ", "ƀ", "Ʉ", "ʉ", "Ʌ", "ʌ", "Ɇ", "ɇ", "Ɉ", "ɉ", "Ɍ", "ɍ", "Ɏ", "ɏ", 
};
// Grec
const char character_greeks_[][4] = {
    "Ͱ", "ͱ", "Ͳ", "ͳ", "Ͷ", "ͷ", "Ά", "ά", "Έ", "έ", "Ή", "ή", "Ί", "ί", "Ό", "ό", "Ύ", "ύ", "Ώ", "ώ", "Α", "α", "Β", "β", 
    "Γ", "γ", "Δ", "δ", "Ε", "ε", "Ζ", "ζ", "Η", "η", "Θ", "θ", "Ι", "ι", "Κ", "κ", "Λ", "λ", "Μ", "μ", "Ν", "ν", "Ξ", "ξ", 
    "Ο", "ο", "Π", "π", "Ρ", "ρ", "Σ", "σ", "Τ", "τ", "Υ", "υ", "Φ", "φ", "Χ", "χ", "Ψ", "ψ", "Ω", "ω", "Ϊ", "ϊ", "Ϋ", "ϋ", 
    "Ϣ", "ϣ", "Ϥ", "ϥ", "Ϧ", "ϧ", "Ϩ", "ϩ", "Ϫ", "ϫ", "Ϭ", "ϭ", "Ϯ", "ϯ", "Ϸ", "ϸ", "Ϻ", "ϻ", "Ͻ", "ͻ", "Ͼ", "ͼ", "Ͽ", "ͽ", 
};
// Cyrillic
const char character_cyrillics_[][4] = {
    "Ѐ", "ѐ", "Ё", "ё", "Ђ", "ђ", "Ѓ", "ѓ", "Є", "є", "Ѕ", "ѕ", "І", "і", "Ї", "ї", "Ј", "ј", "Љ", "љ", "Њ", "њ", "Ћ", "ћ", 
    "Ќ", "ќ", "Ѝ", "ѝ", "Ў", "ў", "Џ", "џ", "А", "а", "Б", "б", "В", "в", "Г", "г", "Д", "д", "Е", "е", "Ж", "ж", "З", "з", 
    "И", "и", "Й", "й", "К", "к", "Л", "л", "М", "м", "Н", "н", "О", "о", "П", "п", "Р", "р", "С", "с", "Т", "т", "У", "у", 
    "Ф", "ф", "Х", "х", "Ц", "ц", "Ч", "ч", "Ш", "ш", "Щ", "щ", "Ъ", "ъ", "Ы", "ы", "Ь", "ь", "Э", "э", "Ю", "ю", "Я", "я", 
    "Ѡ", "ѡ", "Ѣ", "ѣ", "Ѥ", "ѥ", "Ѧ", "ѧ", "Ѩ", "ѩ", "Ѫ", "ѫ", "Ѭ", "ѭ", "Ѯ", "ѯ", "Ѱ", "ѱ", "Ѳ", "ѳ", "Ѵ", "ѵ", "Ѷ", "ѷ", 
    "Ѹ", "ѹ", "Ѻ", "ѻ", "Ѽ", "ѽ", "Ѿ", "ѿ", "Ҁ", "ҁ", "Ҋ", "ҋ", "Ҍ", "ҍ", "Ҏ", "ҏ", "Ґ", "ґ", "Ғ", "ғ", "Ҕ", "ҕ", "Җ", "җ", 
    "Ҙ", "ҙ", "Қ", "қ", "Ҝ", "ҝ", "Ҟ", "ҟ", "Ҡ", "ҡ", "Ң", "ң", "Ҥ", "ҥ", "Ҧ", "ҧ", "Ҩ", "ҩ", "Ҫ", "ҫ", "Ҭ", "ҭ", "Ү", "ү", 
    "Ұ", "ұ", "Ҳ", "ҳ", "Ҵ", "ҵ", "Ҷ", "ҷ", "Ҹ", "ҹ", "Һ", "һ", "Ҽ", "ҽ", "Ҿ", "ҿ", "Ӂ", "ӂ", "Ӄ", "ӄ", "Ӆ", "ӆ", "Ӈ", "ӈ", 
    "Ӊ", "ӊ", "Ӌ", "ӌ", "Ӎ", "ӎ", "Ӑ", "ӑ", "Ӓ", "ӓ", "Ӕ", "ӕ", "Ӗ", "ӗ", "Ә", "ә", "Ӛ", "ӛ", "Ӝ", "ӝ", "Ӟ", "ӟ", "Ӡ", "ӡ", 
    "Ӣ", "ӣ", "Ӥ", "ӥ", "Ӧ", "ӧ", "Ө", "ө", "Ӫ", "ӫ", "Ӭ", "ӭ", "Ӯ", "ӯ", "Ӱ", "ӱ", "Ӳ", "ӳ", "Ӵ", "ӵ", "Ӷ", "ӷ", "Ӹ", "ӹ", 
    "Ӻ", "ӻ", "Ӽ", "ӽ", "Ӿ", "ӿ", "Ԁ", "ԁ", "Ԃ", "ԃ", "Ԅ", "ԅ", "Ԇ", "ԇ", "Ԉ", "ԉ", "Ԋ", "ԋ", "Ԍ", "ԍ", "Ԏ", "ԏ", "Ԑ", "ԑ", 
    "Ԓ", "ԓ", "Ԕ", "ԕ", "Ԗ", "ԗ", "Ԙ", "ԙ", "Ԛ", "ԛ", "Ԝ", "ԝ", "Ԟ", "ԟ", "Ԡ", "ԡ", "Ԣ", "ԣ", "Ԥ", "ԥ", "Ԧ", "ԧ", 
};

// Armenian
const char character_armenians_[][4] = {
    "Ա", "ա", "Բ", "բ", "Գ", "գ", "Դ", "դ", "Ե", "ե", "Զ", "զ", "Է", "է", "Ը", "ը", "Թ", "թ", "Ժ", "ժ", "Ի", "ի", "Լ", "լ", 
    "Խ", "խ", "Ծ", "ծ", "Կ", "կ", "Հ", "հ", "Ձ", "ձ", "Ղ", "ղ", "Ճ", "ճ", "Մ", "մ", "Յ", "յ", "Ն", "ն", "Շ", "շ", "Ո", "ո", 
    "Չ", "չ", "Պ", "պ", "Ջ", "ջ", "Ռ", "ռ", "Ս", "ս", "Վ", "վ", "Տ", "տ", "Ր", "ր", "Ց", "ց", "Ւ", "ւ", "Փ", "փ", "Ք", "ք", 
    "Օ", "օ", "Ֆ", "ֆ", 
};

Character const character_upperCased(Character c, unsigned character_type) {
    // Pas de conversion si utf8 de taille > 2, i.e. avec 111x xxxx dans premier byte.
    if((c.first & 0xE0) == 0xE0)
        return c;
    // (Pas de corversion pour les langues sans masjuscules/minuscules.)
    if(character_type > character_type__last_capitalizable_)
        return c;
    // Cas trivial ASCII (1 byte), i.e. avec 0xxx xxxx.
    if((c.first & 0x80) == 0) {
        c.first = toupper(c.first);
        return c;
    }
    
    // TODO: On pourait détecter le `character_type` à l'aide du range unicode des différentes alphabets...
    const char (*p)[4];
    const char (*end)[4];
    switch(character_type) {
        case character_type_greek: {
            p =    character_greeks_;
            end = &character_greeks_[sizeof(character_greeks_) / sizeof(character_greeks_[0])];
        } break;
        case character_type_cyrillic: {
            p =    character_cyrillics_;
            end = &character_cyrillics_[sizeof(character_cyrillics_) / sizeof(character_cyrillics_[0])];
        } break;
        case character_type_armenian: {
            p =    character_armenians_;
            end = &character_armenians_[sizeof(character_armenians_) / sizeof(character_armenians_[0])];
        } break;
        default: {
            p =    character_latins_;
            end = &character_latins_[sizeof(character_latins_) / sizeof(character_latins_[0])];
        } break;
    }
    bool upper = true;
    while(p < end) {
        if(c.c_data4 == *(uint32_t*)p) {
            if(upper) return c;
            p --;  // (se remet sur le upper cased)
            return (Character) { .c_data4 = *(uint32_t*)p };
        }
        p ++;
        upper = !upper;
    }
    printf("⁉️ Cannot uppercase %s.", c.c_str);
    return c;
}
Character const character_lowerCased(Character c, unsigned character_type) {
    // Pas de conversion si utf8 de taille > 2, i.e. avec 111x xxxx dans premier byte.
    if((c.first & 0xE0) == 0xE0)
        return c;
    // (Pas de corversion pour les langues sans masjuscules/minuscules.)
    if(character_type > character_type__last_capitalizable_)
        return c;
    // Cas trivial ASCII (1 byte), i.e. avec 0xxx xxxx.
    if((c.first & 0x80) == 0) {
        c.first = tolower(c.first);
        return c;
    }
    const char (*p)[4];
    const char (*end)[4];
    switch(character_type) {
        case character_type_greek: {
            p =    character_greeks_;
            end = &character_greeks_[sizeof(character_greeks_) / sizeof(character_greeks_[0])];
        } break;
        case character_type_cyrillic: {
            p =    character_cyrillics_;
            end = &character_cyrillics_[sizeof(character_cyrillics_) / sizeof(character_cyrillics_[0])];
        } break;
        case character_type_armenian: {
            p =    character_armenians_;
            end = &character_armenians_[sizeof(character_armenians_) / sizeof(character_armenians_[0])];
        } break;
        default: {
            p =    character_latins_;
            end = &character_latins_[sizeof(character_latins_) / sizeof(character_latins_[0])];
        } break;
    }
    bool upper = true;
    while(p < end) {
        if(c.c_data4 == *(uint32_t*)p) {
            if(!upper) return c;
            p ++;  // (se remet sur le lower cased)
            return (Character) { .c_data4 = *(uint32_t*)p };
        }
        p ++;
        upper = !upper;
    }
    printf("⁉️ Cannot lowercase %s.", c.c_str);
    return c;
}
