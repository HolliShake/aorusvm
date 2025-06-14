#include "error.h"

#define PADDING 4

INTERNAL char** get_lines(char* _fdata) {
    size_t len = strlen(_fdata);
    
    // Count newlines in one pass
    int line_count = 1;
    for (size_t i = 0; i < len; i++) {
        line_count += (_fdata[i] == '\n');
    }

    // Allocate array of line pointers
    char** lines = (char**) malloc(sizeof(char*) * (line_count + 1));
    ASSERTNULL(lines, ERROR_ALLOCATING_STRING);

    // Split into lines in a single pass
    int curr_line = 0;
    size_t line_start = 0;
    
    for (size_t i = 0; i <= len; i++) {
        if (i == len || _fdata[i] == '\n') {
            // Allocate and copy line
            size_t line_len = i - line_start;
            char* line = (char*) malloc(line_len + 1);
            ASSERTNULL(line, ERROR_ALLOCATING_STRING);
            memcpy(line, _fdata + line_start, line_len);
            line[line_len] = '\0';
            
            lines[curr_line++] = line;
            line_start = i + 1;
        }
    }

    lines[curr_line] = NULL;
    return lines;
}

void throw_error(const char* _caller_file, const char* _caller_func, int _caller_line, char* _fpath, char* _fdata, char* _message, position_t* _position) {
    char** lines = get_lines(_fdata);
    int line_count;
    for (line_count = 0; lines[line_count] != NULL; line_count++);
    
    int line_start = (_position->line_start - PADDING < 0)
        ? 0
        : _position->line_start - PADDING;
        
    int line_end = (_position->line_start + PADDING > line_count)
        ? line_count
        : _position->line_start + PADDING;

    char* line_end_str = string_format("%d", line_end);

    char* format = string_allocate("");

    for (int i = line_start; i < line_end; i++) {
        int line_no = i + 1;
        char* line_no_str = string_format("%d", line_no);
        int diff = strlen(line_end_str) - strlen(line_no_str);
        char* padding = string_allocate("");
        for (int j = 0; j < diff; j++) padding = string_append_char(padding, ' ');
        format = string_append(format, string_format("[%s%s]", padding, line_no_str));
        free(padding);
        if (i == _position->line_start - 1) {
            format = string_append(format, " |> ");
        } else {
            format = string_append(format, " |  ");
        }
        format = string_append(format, string_format("%s", lines[i]));
        if (i < line_end - 1) format = string_append(format, "\n");
    }
    fprintf(stderr, 
        "DEBUG(%s::%s:%d)[%s:%d:%d]: %s\n%s", 
        _caller_file, 
        _caller_func,
        _caller_line, 
        _fpath, 
        _position->line_start, 
        _position->colm_start, 
        _message,
        format
    );
    exit(EXIT_FAILURE);
}