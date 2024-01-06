//
//  file_utils.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#include "_utils/_utils_file.h"

static char*  _FILE_buffer = NULL;
static size_t _FILE_buffer_size = 0;

const char* FILE_contentOpt(const char* path) {
    FILE_freeBuffer();
    if(!path) {  printerror("No path to open."); return NULL; }
    FILE* f = fopen(path, "r");
    if(!f) { printerror("Cannot open %s.", path); return NULL; }
    fseek(f, 0, SEEK_END);
    _FILE_buffer_size = ftell(f) + 1;
    rewind(f);
    _FILE_buffer = calloc(1, _FILE_buffer_size);
    if(!_FILE_buffer) {
        _FILE_buffer_size = 0;
        printerror("Cannot alloc %s.", path);
        return NULL;
    }
    size_t n = fread(_FILE_buffer, _FILE_buffer_size - 1, 1, f);
    fclose(f);
    if(n != 1) {
        FILE_freeBuffer();
        printerror("Cannot read %s.", path);
        return NULL;
    }
    return _FILE_buffer;
}
void     FILE_write(const char* path, const char* content) {
    if(!path) { printwarning("No path to open."); return; }
    if(!content) { printwarning("No content to write."); return; }
    FILE* f = fopen(path, "w");
    if(!f) { printerror("Cannot write at %s.", path); return; }
    fputs(content, f);
    fclose(f);
}
size_t FILE_bufferSize(void) {
    return _FILE_buffer_size;
}
void   FILE_freeBuffer(void) {
    if(_FILE_buffer) free(_FILE_buffer);
    _FILE_buffer = NULL;
    _FILE_buffer_size = 0;
}
