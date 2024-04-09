//
//  utils.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#include "utils_base.h"

#define UNW_LOCAL_ONLY
#include <libunwind.h>  // Info de la stack pour debugging.

static int32_t alloc_count_ = 0;

void print_trace_(uint32_t depth) {
    char sym[256];
    unw_context_t context;
    unw_cursor_t  cursor;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    uint32_t i = 0;
    while(unw_step(&cursor) > 0  && i < depth) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if(pc == 0) break;
        if(unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0)
            printf(" -> %s", sym);
        else
            printf(" -> ⁉️");
        i++;
    }
    printf(".\n");
}

void print_here_(const char* filename, uint32_t line) {
    char sym[256];
    unw_context_t context;
    unw_cursor_t  cursor;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    if(unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if(pc == 0) return;
        if(unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0)
            printf("🐔 Now at %s in %s line %d.\n", sym, filename, line);
        else
            printf("🐔 Now at ⁉️ in %s line %d.\n", filename, line);
        return;
    }
    printf("🐔 ⁉️\n");
}

void    *coq_malloc_(size_t __size, const char* filename, uint32_t line) {
    void* new = malloc(__size);
    alloc_count_++;
    size_t chunks = (__size + 15) / 16;
    printf("🦖✅ Malloc: %p, size %zu chk, count %d -> %s line %d\n", new, chunks, alloc_count_, filename, line);
    return new;
}
void    *coq_calloc_(size_t __count, size_t __size, const char* filename, uint32_t line) {
    void* new = calloc(__count, __size);
    alloc_count_++;
    size_t chunks = (__size*__count + 15) / 16;
    printf("🦖✅ Calloc: %p, size %zu chk, count %d -> %s line %d\n", new, chunks, alloc_count_, filename, line);
    return new;
}
void     coq_free_(void* ptr, const char* filename, uint32_t line) {
    alloc_count_--;
    printf("🦖❌ Free: %p, count %d -> %s line %d\n", ptr, alloc_count_, filename, line);
    free(ptr);
}
void    *coq_realloc_(void * const ptr, size_t __size, const char* filename, uint32_t line) {
    printf("🦖  Realloc: %p", ptr);
    void* new = realloc(ptr, __size);
    size_t chunks = (__size + 15) / 16;
    printf(" size %zu chk (count %d) -> %s line %d\n", chunks, alloc_count_, filename, line);
    return new;
}
