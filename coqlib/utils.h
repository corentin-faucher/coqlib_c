//
//  utils.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef utils_h
#define utils_h
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define printdebug(format, ...) \
printf("🐛 Debug: "format" -> %s line %d\n", ##__VA_ARGS__, __FILENAME__, __LINE__)
#define printerror(format, ...) \
printf("❌ Error: "format" -> %s line %d\n", ##__VA_ARGS__, __FILENAME__, __LINE__)
#define printwarning(format, ...) \
printf("⚠️ Warn.: "format" -> %s line %d\n", ##__VA_ARGS__, __FILENAME__, __LINE__)

//#define guard_return(type, ptr, returned_value) \
//if(value == NULL) return returned_value;

#endif /* utils_h */
