//
//  string_utils.c
//
//  Created by Corentin Faucher on 2023-10-25.
//

#include "coq_string.h"


char* String_createCopy(const char* src) {
    size_t size = strlen(src) + 1;
    char* copy = coq_malloc(size);
    strcpy(copy, src);  // (inclue le null char).
    return copy;
}
char* String_createCat(const char* src1, const char* src2) {
    size_t size = strlen(src1) + strlen(src2) + 1;
    char* new = coq_malloc(size);
    strcpy(new, src1);
    strcat(new, src2);
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

/*-- Navigation dans string utf8 ---------------------------------------*/
size_t stringUTF8_charSizeAt(const char* c_str, size_t pos) {
    const char* ptr = &c_str[pos];
    // ASCII ordinaire...
    // 0xxx xxxx
    if(!(*ptr & 0x80))
        return 1;
    // Byte intermediare ?
    // 10xx xxxx
    if((*ptr & 0xC0) == 0x80) {
        printerror("UTF8 Inter-byte.");
        return 1;
    }
    // 110x xxxx ...
    if((*ptr & 0xE0) == 0xC0)
        return 2;
    // 1110 xxxx ...
    if((*ptr & 0xF0) == 0xE0)
        return 3;
    // 1111 0xxx ...
    if((*ptr & 0xF8) == 0xF0)
        return 4;
    printerror("Bad utf8 %d.", *ptr);
    return 1;
}
void   stringUTF8_moveToNextChar(const char* const c_str, const char** ref) {
    // Aller au next a priori.
    (*ref)++;
    // Se deplacer encore si on est sur byte "extra", i.e. avec 10xx xxxx.
    while((**ref & 0xC0) == 0x80) {
        (*ref)++;
    }
}
void   stringUTF8_setToLastChar(const char* const c_str, const char** ref) {
    // Aller à la fin
    *ref = c_str;
    while(**ref) (*ref)++;
    if(*ref == c_str) return;
    // Aller au utf8 précédent
    (*ref)--;
    // Remonter si byte intermediare 10xx xxxx
    while((*ref > c_str) && ((**ref & 0xC0) == 0x80)) (*ref)--;
}
void   stringUTF8_moveToPreviousChar(const char* const c_str, const char** ref) {
    if(*ref <= c_str) {
        printwarning("Already at begining.");
        return;
    }
    // Aller au previous a priori.
    (*ref)--;
    // Se deplacer encore si on est sur byte "intermedaire", i.e. avec 10xx xxxx.
    while((*ref > c_str) && (**ref & 0xC0) == 0x80) (*ref)--;
}
void   stringUTF8_deleteLastChar(char* const c_str) {
    // Aller à la fin
    char* c = c_str;
    while(*c) c++;
    if(c == c_str) {
        printwarning("Already empty string.");
        return;
    }
    // Aller au utf8 précédent
    c--;
    *c = 0;
    // Remonter si byte intermediare 10xx xxxx
    while((c > c_str) && ((*c & 0xC0) == 0x80)) {
        c--;
        *c = 0;
    }
}

size_t stringUTF8_lenght(const char* const c_str) {
    const char* c = c_str;
    size_t lenght = 0;
    while(*c) {
        stringUTF8_moveToNextChar(c_str, &c);
        lenght++;
    }
    return lenght;
}

bool   stringUTF8_isSingleEmoji(const char* c_str) {
    size_t size = stringUTF8_charSizeAt(c_str, 0);
    if(size != 4) return false;
    if(c_str[4] != 0) return false;
    // Bon on serait cense verifier le range...
    // mais a priori si un char prend 4 bytes c'est que c'est un emoji...
    return true;
}


