//
//  file_utils.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-08.
//

#include "utils_file.h"
#include "utils_base.h"
//#include <sys/stat.h>
//#include <unistd.h>

static char*  FILE_buffer_ = NULL;
static size_t FILE_buffer_size_ = 0;

//int         FILE_existAt(const char* path) {
//    struct stat st;
//    stat(path, &st);
//    if(S_ISDIR(st.st_mode))
//        return file_exist_dir;
//    if(S_ISREG(st.st_mode))
//        return file_exist_file;
//    return file_exist_none;
//}
const char* FILE_stringContentOptAt(const char* path) {
    FILE_freeBuffer();
    if(!path) {  printerror("No path to open."); return NULL; }
    FILE* f = fopen(path, "r");
    if(!f) { return NULL; }
    fseek(f, 0, SEEK_END);
    // Ajouter + 1 pour le `\0` de fin de string.
    FILE_buffer_size_ = ftell(f) + 1;
    rewind(f);
    FILE_buffer_ = calloc(1, FILE_buffer_size_);
    if(!FILE_buffer_) {
        FILE_buffer_size_ = 0;
        printerror("Cannot alloc %s.", path);
        return NULL;
    }
    size_t n = fread(FILE_buffer_, FILE_buffer_size_ - 1, 1, f);
    fclose(f);
    if(n != 1) {
        FILE_freeBuffer();
        printerror("Cannot read %s.", path);
        return NULL;
    }
    return FILE_buffer_;
}
const void* FILE_bufferContentOptAt(const char* path) {
    FILE_freeBuffer();
    if(!path) {  printerror("No path to open."); return NULL; }
    FILE* f = fopen(path, "rb");
    if(!f) { return NULL; }
    fseek(f, 0, SEEK_END);
    FILE_buffer_size_ = ftell(f);
    rewind(f);
    FILE_buffer_ = calloc(1, FILE_buffer_size_);
    if(!FILE_buffer_) {
        FILE_buffer_size_ = 0;
        printerror("Cannot alloc %s.", path);
        return NULL;
    }
    size_t n = fread(FILE_buffer_, FILE_buffer_size_, 1, f);
    fclose(f);
    if(n != 1) {
        FILE_freeBuffer();
        printerror("Cannot read %s.", path);
        return NULL;
    }
    return FILE_buffer_;
}
void     FILE_writeString(const char* path, const char* content) {
    if(!path) { printwarning("No path to open."); return; }
    if(!content) { printwarning("No content to write."); return; }
    FILE* f = fopen(path, "w");
    if(!f) { printerror("Cannot write at %s.", path); return; }
    fputs(content, f);
    fclose(f);
}
void   FILE_writeData(const char* path, const void* buffer, size_t buffer_size) {
    if(!path) { printwarning("No path to open."); return; }
    if(!buffer) { printwarning("No content to write."); return; }
    FILE* f = fopen(path, "wb");
    if(!f) { printerror("Cannot write at %s.", path); return; }
    fwrite(buffer, buffer_size, 1, f);
    fclose(f);
}
size_t FILE_bufferSize(void) {
    return FILE_buffer_size_;
}
void   FILE_freeBuffer(void) {
    if(!FILE_buffer_) return;
    free(FILE_buffer_);
    FILE_buffer_ = NULL;
    FILE_buffer_size_ = 0;
}
