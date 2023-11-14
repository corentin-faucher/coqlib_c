//
//  utils.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef utils_h
#define utils_h
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libunwind.h>

#define true 1
#define false 0
typedef int32_t Bool;

void print_trace(uint32_t depth);
void _print_here(const char* filename, uint32_t line);

//#define COQ_ALLOC

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#ifdef DEBUG
#define printdebug(format, ...) \
printf("🐛 Debug: "format" -> %s line %d\n", ##__VA_ARGS__, __FILENAME__, __LINE__)
#else
#define printdebug(format, ...)
#endif
#define printerror(format, ...) \
printf("❌ Error: "format" -> %s line %d", ##__VA_ARGS__, __FILENAME__, __LINE__), print_trace(4)
#define printwarning(format, ...) \
printf("⚠️ Warn.: "format" -> %s line %d", ##__VA_ARGS__, __FILENAME__, __LINE__), print_trace(4)
#define printhere()\
_print_here(__FILENAME__, __LINE__);

void    *_coq_malloc_(size_t __size, const char* filename, uint32_t line);
void    *_coq_calloc_(size_t __count, size_t __size, const char* filename, uint32_t line);
void     _coq_free_(void* ptr, const char* filename, uint32_t line);
void    *_coq_realloc_(void *__ptr, size_t __size, const char* filename, uint32_t line);

#ifdef COQ_ALLOC
#define coq_malloc(size) \
_coq_malloc_(size, __FILENAME__, __LINE__)
#define coq_calloc(count, size) \
_coq_calloc_(count, size, __FILENAME__, __LINE__)
#define coq_free(ptr) \
_coq_free_(ptr, __FILENAME__, __LINE__)
#define coq_realloc(ptr, size) \
_coq_realloc_(ptr, size, __FILENAME__, __LINE__)
#else
#define coq_malloc(size) \
malloc(size)
#define coq_calloc(count, size) \
calloc(count, size)
#define coq_free(ptr) \
free(ptr)
#define coq_realloc(ptr, size) \
realloc(ptr, size)
#endif

#endif /* utils_h */
