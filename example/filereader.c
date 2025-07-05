#include "filereader.h"


char* file_read(char* _path) {
    FILE* file = fopen(_path, "r");
    if (file == NULL) {
        fprintf(stderr, "[%s:%d] failed to open file: %s\n", __FILE__, __LINE__, _path);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char* )malloc(size + 1);
    if (data == NULL) {
        fclose(file);
        fprintf(stderr, "[%s:%d] failed to allocate memory\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    fread(data, 1, size, file);
    data[size] = '\0';
    fclose(file);
    return data;
}

