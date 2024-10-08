//
//  _util_.h
//  Include et fonctions de base pratiques pour le debugging.
//
//  Created by Corentin Faucher on 2023-10-12.
//
#ifndef COQ_UTIL_BASE_H
#define COQ_UTIL_BASE_H

#include <stdio.h>  // printf, sprintf, file, etc.
#include <stdlib.h> // malloc, free, etc.
#include <string.h> // strcat, strcmp, etc.
#include <stddef.h> 
#include <stdint.h>

//#if __APPLE__
//// Pour les `TARGET_OS_OSX`, etc.
//#include <TargetConditionals.h>
//#endif

#ifdef DEBUG
// Pour suivre les alloc/dealloc...
// #define COQ_ALLOC
#endif

#pragma mark - Log -----------

#define COQ__FILENAME__                                                        \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#ifdef DEBUG
#define printdebug(format, ...)                                                \
  printf("🐛 Debug: " format " -> %s line %d\n", ##__VA_ARGS__,                \
         COQ__FILENAME__, __LINE__)
#else
#define printdebug(format, ...)
#endif
#define printerror(format, ...)                                                \
  printf("❌ Error: " format " -> %s line %d", ##__VA_ARGS__, COQ__FILENAME__, \
         __LINE__),                                                            \
      print_trace_(4)
#define printwarning(format, ...)                                              \
  printf("⚠️ Warn.: " format " -> %s line %d", ##__VA_ARGS__, COQ__FILENAME__,  \
         __LINE__),                                                            \
      print_trace_(4)
#define printhere() print_here_(COQ__FILENAME__, __LINE__);


#pragma mark - Alloc -----------
// Pour avoir la taile d'une struct de type array, i.e. struct { int const count; int arr[1] };
#define coq_arrayTypeSize(arrayType, elementType, elementCount) \
    sizeof(arrayType) + (elementCount-1)*sizeof(elementType)

#ifdef COQ_ALLOC
#define coq_malloc(size) coq_malloc_(size, COQ__FILENAME__, __LINE__)
#define coq_calloc(count, size)                                                \
  coq_calloc_(count, size, COQ__FILENAME__, __LINE__)
#define coq_callocTyped(type) (type*const)coq_calloc_(1, sizeof(type), COQ__FILENAME__, __LINE__)
#define coq_callocArray(arrayType, elementType, elementCount) \
    (arrayType*const)coq_calloc_(1, sizeof(arrayType) + (elementCount-1)*sizeof(elementType), COQ__FILENAME__, __LINE__)
#define coq_free(ptr) coq_free_(ptr, COQ__FILENAME__, __LINE__)
#define coq_realloc(ptr, size)                                                 \
  coq_realloc_(ptr, size, COQ__FILENAME__, __LINE__)
#else
#define coq_malloc(size) malloc(size)
#define coq_calloc(count, size) calloc(count, size)
#define coq_callocTyped(type) (type*const)calloc(1, sizeof(type))
#define coq_callocArray(arrayType, elementType, elementCount) \
    (arrayType*const)calloc(1, coq_arrayTypeSize(arrayType, elementType, elementCount))
#define coq_free(ptr) free(ptr)
#define coq_realloc(ptr, size) realloc(ptr, size)
#endif


// Test de unwrap dans le style de swift: if let a = aOpt {...}... Sans doute superflu...
#define if_let(type, var, init_val) { type var = init_val; if(var) {
#define if_let_end }}

// "Private"... 
void  print_trace_(unsigned depth);
void  print_here_(const char *filename, unsigned line);
void *coq_malloc_(size_t __size, const char *filename, uint32_t line);
void *coq_calloc_(size_t __count, size_t __size, const char *filename,
                  uint32_t line);
void coq_free_(void *ptr, const char *filename, uint32_t line);
void *coq_realloc_(void *__ptr, size_t __size, const char *filename,
                   uint32_t line);

#endif /* util_h */
