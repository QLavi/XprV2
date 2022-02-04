#include "io_and_mem.h"

/* #define DEBUG_ALLOC */

void* x_alloc(size_t* old_ptr, size_t size, int line_no, char* filename) {
    if(old_ptr != NULL && size == 0) {

#ifdef DEBUG_ALLOC
        printf("%s:%i: free: %zu\n", filename, line_no, old_ptr[-1]);
        free(&old_ptr[-1]);
#else
        free(old_ptr);
#endif
        return NULL;
    }

    if(size != 0) {
#ifdef DEBUG_ALLOC
        size_t* new_ptr = realloc(old_ptr, sizeof(size_t) + size);
        *new_ptr = size;
        printf("%s:%i: alloc: %zu\n", filename, line_no, size);
#else
        void* new_ptr = realloc(old_ptr, size);
#endif

        if(new_ptr == NULL) {
            fprintf(stderr, "Out_Of_Memory_Error!\nAborted");
            exit(1);
        }
#ifdef DEBUG_ALLOC
        return &new_ptr[1];
#endif
        return new_ptr;
    }
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
