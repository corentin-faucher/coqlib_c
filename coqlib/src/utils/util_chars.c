//
//  string_utils.c
//
//  Created by Corentin Faucher on 2023-10-25.
//
#include "util_chars.h"

#include <ctype.h>
#include "util_base.h"

// MARK: - Char
size_t char_sizeAsUTF8(const char* const ref) {
    // (On y va du plus probable au moins probable...)
    // ASCII ordinaire...
    // 0xxx xxxx
    if(!(*ref & 0x80))
        return 1;
    // Byte intermediare ?
    // 10xx xxxx
    if((*ref & 0xC0) == 0x80) {
        printerror("UTF8 Inter-byte.");
        return 1;
    }
    // 110x xxxx ...
    if((*ref & 0xE0) == 0xC0)
        return 2;
    // 1110 xxxx ...
    if((*ref & 0xF0) == 0xE0)
        return 3;
    // 1111 0xxx ...
    if((*ref & 0xF8) == 0xF0)
        return 4;
    // 1111 10xx ...
    if((*ref & 0xFC) == 0xF8)
        return 5;
    // 1111 110x ...
    if((*ref & 0xFE) == 0xFC)
        return 6;
    printerror("Bad utf8 %d.", *ref);
    return 1;
}

static inline bool char_isUTF8VariationSelector_(char const*const c) {
        return ((unsigned char)c[0] == 0xef) 
            && ((unsigned char)c[1] == 0xb8)
            && ((c[2] & 0xF0) == 0x80); 
}
size_t char_sizeAsCaracter(char const*const ref) {
    size_t size = char_sizeAsUTF8(ref);
    if(char_isUTF8VariationSelector_(&ref[size]))
        size += 3;
    return size;
}

// MARK: - String : Array de chars
char* String_createCopy(const char* src) {
    size_t size = strlen(src) + 1;
    
    char* copy = coq_calloc(1, size);
    strcpy(copy, src);  // (inclue le null char).
    return copy;
}
char* String_createCat(const char* src1, const char* src2) {
    size_t size1 = strlen(src1);
    size_t size2 = strlen(src2);
    char* new = coq_calloc(1, size1 + size2 + 1);
    memcpy(new, src1, size1);
    memcpy(new + size1, src2, size2);
    return new;
}
char* String_createCat3(const char* src1, const char* src2, const char* src3) {
    size_t size1 = strlen(src1);
    size_t size2 = strlen(src2);
    size_t size3 = strlen(src3);
    char* new = coq_calloc(1, size1 + size2 + size3 + 1);
    memcpy(new, src1, size1);
    memcpy(new + size1, src2, size2);
    memcpy(new + size1 + size2, src3, size3);
    return new;
}
char* String_createCopyTrimedOfSpaces(const char*const src) {
    if(!src) return NULL;
    size_t const src_size = strlen(src) + 1;
    char*const new = coq_calloc(1, src_size);
    char* dest = new;
    char const* c = src;
    char const*const c_end = &src[src_size];
    while(c < c_end) {
        Character ch = charconstRef_getCharacterAndMoveToNextCharacter(&c);
        if(character_isSpace(ch) || character_isEndLine(ch))
            continue;
        for(int i = 0; ch.c_str[i] && i < CHARACTER_MAX_SIZE; i++) {
            *dest = ch.c_str[i];
            dest++;
        }
    }
    return new;
}

bool  string_startWithPrefix(const char* c_str, const char* prefix) {
    while(*prefix && *c_str) {
        if(*prefix != *c_str) return false;
        prefix ++;
        c_str ++;
    }
    if(!*prefix) return true;
    return false;
//    size_t prefix_len = strlen(prefix);
//    return strncmp(c_str, prefix, prefix_len);
}

// MARK: Navigation dans une string utf8
void      charRef_moveToNextUTF8Char(char** const ref) {
    // Aller au next a priori.
    (*ref)++;
    // Se deplacer encore si on est sur byte "extra", i.e. avec 10xx xxxx.
    while((**ref & 0xC0) == 0x80) {
        (*ref)++;
    }
}
void      charconstRef_moveToNextUTF8Char(char const** const ref) {
    // Aller au next a priori.
    (*ref)++;
    // Se deplacer encore si on est sur byte "extra", i.e. avec 10xx xxxx.
    while((**ref & 0xC0) == 0x80) {
        (*ref)++;
    }
}
Character charconstRef_getCharacterAndMoveToNextCharacter(char const**const cRef) {
    Character c = { .c_str[0] = **cRef };
    // Aller au next a priori.
    (*cRef)++;
    // Cas single Ascii, exit.
    if(!(**cRef & 0x80))
        return c;
    int i = 1;
    // Se deplacer encore si on est sur byte "extra", i.e. avec 10xx xxxx.
    while((**cRef & 0xC0) == 0x80 && i < CHARACTER_MAX_SIZE) {
        c.c_str[i] = **cRef;
        (*cRef)++; i++;
    }
    if(char_isUTF8VariationSelector_(*cRef) && i < CHARACTER_MAX_SIZE - 2) {
        c.c_str[i] =   **cRef; (*cRef)++;
        c.c_str[i+1] = **cRef; (*cRef)++;
        c.c_str[i+2] = **cRef; (*cRef)++;
    }
    return c;
}
void   charRef_moveToPreviousUTF8Char(char** const ref) {
    // Aller au previous a priori.
    (*ref)--;
    // Se deplacer encore si on est sur byte "intermedaire", i.e. avec 10xx xxxx.
    int i = 0;
    while((i < 4) && ((**ref & 0xC0) == 0x80)) {
        (*ref)--;
        i++;
    } 
}
//void   charRef_moveToEnd(const char** ref) {
//    while(**ref) (*ref)++;
//}
void   stringUTF8_deleteLastChar(char* const c_str) {
    // Aller à la fin
    char* c = c_str;
    while(*c) c++;
    if(c == c_str) {
        printwarning("Already empty string.");
        return;
    }
    char* const end = c;
    // Aller au utf8 précédent
    charRef_moveToPreviousUTF8Char(&c);
    if(c < c_str) {
        printerror("Bad utf8 string.");
        return;
    }
    // Effacer
    while(c < end) {
        *c = 0;
        c++;
    }
}

size_t stringUTF8_lenght(const char* const c_str) {
    char* c = (char*)c_str;
    size_t lenght = 0;
    while(*c) {
        charRef_moveToNextUTF8Char(&c);
        lenght++;
    }
    return lenght;
}

// MARK: - Character, structure d'un caractère quelconque (pas juste ASCII)
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
static const char character_armenians_[][4] = {
    "Ա", "ա", "Բ", "բ", "Գ", "գ", "Դ", "դ", "Ե", "ե", "Զ", "զ", "Է", "է", "Ը", "ը", "Թ", "թ", "Ժ", "ժ", "Ի", "ի", "Լ", "լ",
    "Խ", "խ", "Ծ", "ծ", "Կ", "կ", "Հ", "հ", "Ձ", "ձ", "Ղ", "ղ", "Ճ", "ճ", "Մ", "մ", "Յ", "յ", "Ն", "ն", "Շ", "շ", "Ո", "ո",
    "Չ", "չ", "Պ", "պ", "Ջ", "ջ", "Ռ", "ռ", "Ս", "ս", "Վ", "վ", "Տ", "տ", "Ր", "ր", "Ց", "ց", "Ւ", "ւ", "Փ", "փ", "Ք", "ք",
    "Օ", "օ", "Ֆ", "ֆ",
};
Character character_upperCased(Character c, unsigned character_type) {
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
Character character_lowerCased(Character c, unsigned character_type) {
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

bool character_isSpace(Character const c) {
    static const char character_spaces_[][4] = {
        " ", "\t", " ", "　", "\u2009",
    };
    const char (*p)[4] =    character_spaces_;
    const char (*end)[4] = &character_spaces_[sizeof(character_spaces_) / sizeof(character_spaces_[0])];
    for(; p < end; p++) {
        if(c.c_data4 == *(uint32_t*)p) return true;
    }
    return false;
}
bool character_isPunct(Character const c) {
    static const char character_puncts_[][4] = {
        ".", ",", ";", ":", "!", "?",
    };
    const char (*p)[4] =    character_puncts_;
    const char (*end)[4] = &character_puncts_[sizeof(character_puncts_) / sizeof(character_puncts_[0])];
    for(; p < end; p++) {
        if(c.c_data4 == *(uint32_t*)p) return true;
    }
    return false;
}
bool character_isWordFinal(Character const c) {
    static const char character_final_[][4] = {
        " ", "\t", " ", "　", "\u2009", ".", ",", ";", ":", "!", "?", "-",
    };
    const char (*p)[4] =    character_final_;
    const char (*end)[4] = &character_final_[sizeof(character_final_) / sizeof(character_final_[0])];
    for(; p < end; p++) {
        if(c.c_data4 == *(uint32_t*)p) return true;
    }
    return false;
}
bool character_isEndLine(Character const c) {
    if(c.c_data4 == spchar_return_.c_data4) return true;
    if(c.c_data4 == spchar_newline_.c_data4) return true;
    return false;
}
bool character_isEmoji(Character const c) {
    // 17 bits, 4 octets.
    //     1111 0xxx + 10xx xxxx + 10xx xxxx + 10xx xxxx
    // min 1111 0000 + 1001 1111 + 1000 1100 + 1000 0000
    // max 1111 0000 + 1001 1111 + 1001 1111 + 1011 1111
    // Range 1F300 - 1F5FF Symbols et pictogrammes divers
    //       1F600 - 1F64F Emojis
    //       1F650 - 1F7FF Autres symbols divers...
    if((unsigned char)c.c_str[0] != 0xF0) return false;
    if((unsigned char)c.c_str[1] != 0x9F) return false;
    if((unsigned char)c.c_str[2] < 0x8C) return false;
    if((unsigned char)c.c_str[2] > 0x9F) return false;
    if((c.c_str[3] & 0xC0) != 0x80) {
        printerror("Bad char %s.", c.c_str); return false;
    }
    return true;
}

uint32_t  character_toUnicode32(Character const c) {
    if(!(c.c_str[0] & 0x80)) // Cas "ascii"... 0xxx xxxx
        return c.c_str[0];
    // Byte intermediare ? // 10xx xxxx
    if((c.c_str[0] & 0xC0) == 0x80) {
        printerror("UTF8 Inter-byte.");
        return 0;
    }
    // Cas 110x xxxx + 10xx xxxx -> 11 bits utiles.
    if((c.c_str[0] & 0xE0) == 0xC0) {
        return (((uint32_t)c.c_str[0] & 0x1F) << 6) |
                ((uint32_t)c.c_str[1] & 0x3F);
    }
    // Cas 1110 xxxx + 10xx xxxx + 10xx xxxx -> 16 bits.
    if((c.c_str[0] & 0xF0) == 0xE0) {
        return (((uint32_t)c.c_str[0] & 0x0F) << 12) |
               (((uint32_t)c.c_str[1] & 0x3F) << 6) |
                ((uint32_t)c.c_str[2] & 0x3F);
    }
    // Cas 1111 0xxx + 10xx xxxx + 10xx xxxx + 10xx xxxx -> 21 bits.
    if((c.c_str[0] & 0xF8) == 0xF0) {
        return (((uint32_t)c.c_str[0] & 0x07) << 18) |
               (((uint32_t)c.c_str[1] & 0x3F) << 12) |
               (((uint32_t)c.c_str[2] & 0x3F) << 6) |
                ((uint32_t)c.c_str[3] & 0x3F);
    }
    // Cas 5 chars/26 bits : 1111 10xx + 10xx xxxx +... 
    if((c.c_str[0] & 0xFC) == 0xF8) {
        return (((uint32_t)c.c_str[0] & 0x03) << 24) |
               (((uint32_t)c.c_str[1] & 0x3F) << 18) |
               (((uint32_t)c.c_str[2] & 0x3F) << 12) |
               (((uint32_t)c.c_str[3] & 0x3F) <<  6) |
                ((uint32_t)c.c_str[4] & 0x3F);
    }
    // Cas 31 bits / 6 chars : 1111 110x + 10xx xxxx + ...
    if((c.c_str[0] & 0xFE) == 0xFC) {
        return (((uint32_t)c.c_str[0] & 0x01) << 30) |
               (((uint32_t)c.c_str[1] & 0x3F) << 24) |
               (((uint32_t)c.c_str[2] & 0x3F) << 18) |
               (((uint32_t)c.c_str[3] & 0x3F) << 12) |
               (((uint32_t)c.c_str[4] & 0x3F) << 6) |
                ((uint32_t)c.c_str[5] & 0x3F);
    }
    return 0u;
}

Character Character_fromUnicode32(uint32_t const unicode) {
    Character c = {};
    // Cas ASCII 0xxx xxxx (7 bits)
    if(unicode < 0x0080) {
        c.c_str[0] = unicode;
        return c;
    }
    // Cas 11 bits / 2 chars : 110x xxxx + 10xx xxxx
    if(unicode < 0x0800) {
        c.c_str[0] = 0xC0 | ((unicode >> 6) & 0x1F);
        c.c_str[1] = 0x80 | ( unicode       & 0x3F);
        return c;
    }
    // Cas 16 bits / 3 chars : 1110 xxxx + 10xx xxxx + 10xx xxxx
    if(unicode <  0x010000) {
        c.c_str[0] = 0xE0 | ((unicode >> 12) & 0x0F);
        c.c_str[1] = 0x80 | ((unicode >>  6) & 0x3F);
        c.c_str[2] = 0x80 | ( unicode        & 0x3F);
        return c;
    }
    // Cas 21 bits / 4 chars : 
    // 1111 0xxx + 10xx xxxx + 10xx xxxx + 10xx xxxx
    if(unicode <  0x200000) {
        c.c_str[0] = 0xF0 | ((unicode >> 18) & 0x07);
        c.c_str[1] = 0x80 | ((unicode >> 12) & 0x3F);
        c.c_str[2] = 0x80 | ((unicode >>  6) & 0x3F);
        c.c_str[3] = 0x80 | ( unicode        & 0x3F);
        return c;
    }
    // Cas 26 bits / 5 chars :
    // 1111 10xx + 10xx xxxx + 10xx xxxx + 10xx xxxx + 10xx xxxx
    if(unicode <  0x4000000) {
        c.c_str[0] = 0xF8 | ((unicode >> 24) & 0x03);
        c.c_str[1] = 0x80 | ((unicode >> 18) & 0x3F);
        c.c_str[2] = 0x80 | ((unicode >> 12) & 0x3F);
        c.c_str[3] = 0x80 | ((unicode >>  6) & 0x3F);
        c.c_str[4] = 0x80 | ( unicode        & 0x3F);
        return c;
    }
    // Cas 31 bits / 6 chars :
    // 1111 110x + 10xx xxxx + 10xx xxxx+10xx xxxx+10xx xxxx+10xx xxxx
    if(unicode & 0x80000000) {
        printerror("Bad unicode ?");
        return spchar_questionMark;
    }
    c.c_str[0] = 0xFC | ((unicode >> 30) & 0x01);
    c.c_str[1] = 0x80 | ((unicode >> 24) & 0x3F);
    c.c_str[2] = 0x80 | ((unicode >> 18) & 0x3F);
    c.c_str[3] = 0x80 | ((unicode >> 12) & 0x3F);
    c.c_str[3] = 0x80 | ((unicode >>  6) & 0x3F);
    c.c_str[4] = 0x80 | ( unicode        & 0x3F);
    return c;
}

Character Character_fromUTF8string(char const*const c_str) {
    Character c;
    memcpy(c.c_str, c_str, char_sizeAsCaracter(c_str));
    return c;
}

size_t    character_size(Character const c) {
    for(size_t size = 1; size < CHARACTER_MAX_SIZE + 1; size++)
        if(c.c_str[size] == 0) return size;
    printwarning("No 0 in Chararter %d.", c.c_data4);
    return CHARACTER_MAX_SIZE;
}

// MARK: - CharacterArray, une liste de caractères
typedef struct CharacterArray {
    size_t const count;
    Character    chars[1];
} CharacterArray;
CharacterArray*   CharacterArray_createFromString(const char*const string) {
    size_t maxCount = 512;
    CharacterArray* ca = coq_callocArray(CharacterArray, Character, maxCount);
    const char* c = string;
    size_t charIndex = 0;
    while(*c) {
        if(charIndex >= maxCount) {
            maxCount += 512;
            ca = coq_realloc(ca, sizeof(CharacterArray) + (maxCount - 1) * sizeof(Character));
        }
        ca->chars[charIndex] = charconstRef_getCharacterAndMoveToNextCharacter(&c);
        charIndex++;
    }
    *(size_t*)&ca->count = charIndex;
    charIndex = charIndex ? charIndex : 1;
    ca = coq_realloc(ca, sizeof(CharacterArray) + (charIndex - 1) * sizeof(Character));
    return ca;
}
size_t            characterarray_count(const CharacterArray*const charArr) {
    return charArr->count;
}
Character const * characterarray_first(const CharacterArray*const charArray) {
    return charArray->chars;
}
Character const * characterarray_end(const CharacterArray*const charArray) {
    return &charArray->chars[charArray->count];
}

