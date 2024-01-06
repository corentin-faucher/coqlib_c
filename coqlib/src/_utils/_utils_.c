//
//  utils.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#include "_utils/_utils_.h"

static int32_t __total_alloc = 0;

void print_trace(uint32_t depth) {
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

void _print_here(const char* filename, uint32_t line) {
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

void    *_coq_malloc_(size_t __size, const char* filename, uint32_t line) {
    void* new = malloc(__size);
    __total_alloc++;
    size_t chunks = (__size + 15) / 16;
    printf("🦖✅ Malloc: %p, size %zu chk, count %d -> %s line %d\n", new, chunks, __total_alloc, filename, line);
    return new;
}
void    *_coq_calloc_(size_t __count, size_t __size, const char* filename, uint32_t line) {
    void* new = calloc(__count, __size);
    __total_alloc++;
    size_t chunks = (__size*__count + 15) / 16;
    printf("🦖✅ Calloc: %p, size %zu chk, count %d -> %s line %d\n", new, chunks, __total_alloc, filename, line);
    return new;
}
void     _coq_free_(void* ptr, const char* filename, uint32_t line) {
    __total_alloc--;
    printf("🦖❌ Free: %p, count %d -> %s line %d\n", ptr, __total_alloc, filename, line);
    free(ptr);
}
void    *_coq_realloc_(void *ptr, size_t __size, const char* filename, uint32_t line) {
    void* new = realloc(ptr, __size);
    size_t chunks = (__size + 15) / 16;
    printf("🦖  Realloc: %p -> %p, size %zu chk (count %d) -> %s line %d\n", ptr, new, chunks, __total_alloc, filename, line);
    return new;
}
