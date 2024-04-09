//
//  string_utils.c
//
//  Created by Corentin Faucher on 2023-10-25.
//

#include "utils_string.h"
#include "utils_base.h"


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
size_t charRef_sizeAsUTF8(const char* const ref) {
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
    printerror("Bad utf8 %d.", *ref);
    return 1;
}
void   charRef_moveToNextUTF8Char(char** const ref) {
    // Aller au next a priori.
    (*ref)++;
    // Se deplacer encore si on est sur byte "extra", i.e. avec 10xx xxxx.
    while((**ref & 0xC0) == 0x80) {
        (*ref)++;
    }
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

bool   stringUTF8_isSingleEmoji(const char* c_str) {
    size_t size = charRef_sizeAsUTF8(c_str);
    if(size != 4) return false;
    if(c_str[4] != 0) return false;
    // Bon on serait cense verifier le range...
    // mais a priori si un char prend 4 bytes c'est que c'est un emoji...
    return true;
}


