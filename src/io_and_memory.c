#include "io_and_mem.h"

void* x_alloc(void* old_ptr, size_t size) {
    if(old_ptr != NULL && size == 0) {
        free(old_ptr);
        return NULL;
    }

    if(size != 0) {
        void* new_ptr = realloc(old_ptr, size);
        if(new_ptr == NULL) {
            fprintf(stderr, "Out_Of_Memory_Error!\nAborted");
            exit(1);
        }
        return new_ptr;
    }
    return NULL;
}

char* load_file(char* filename) {
    FILE* fptr = fopen(filename, "r");
    if(fptr == NULL) {
        fprintf(stderr, "Unable to Open File: '%s'\n", filename);
        exit(1);
    }

    fseek(fptr, 0, SEEK_END);
    size_t size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    char* src = ALLOC(char, size +1);
    size_t read_size = fread(src, 1, size, fptr);
    if(read_size < size) {
        fprintf(stderr, "'%s' reading failed. read_size != file_size\n", filename);
        exit(1);
    }
    src[size] = '\0';
    fclose(fptr);
    return src;
}
