//
//  string_utils.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-25.
//

#include "string_utils.h"


char* string_createCopy(const char* src) {
    size_t size = strlen(src) + 1;
    char* copy = coq_malloc(size);
    strcpy(copy, src);
    return copy;
}

Bool  string_startWithPrefix(const char* c_str, const char* prefix) {
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
