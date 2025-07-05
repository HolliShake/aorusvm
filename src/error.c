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
    
    // Calculate line bounds with bounds checking
    int line_start = (_position->line_start > PADDING) ? _position->line_start - PADDING : 0;
    int line_end = _position->line_start + PADDING;
    
    // Count lines and adjust end if needed
    int line_count = 0;
    while (lines[line_count] != NULL) line_count++;
    if (line_end > line_count) line_end = line_count;

    // Pre-calculate max line number width
    int max_width = snprintf(NULL, 0, "%d", line_end);
    char line_num_buffer[32];

    // Build output format string more efficiently
    size_t format_size = 256; // Initial buffer size
    char* format = malloc(format_size);
    size_t format_len = 0;
    
    for (int i = line_start; i < line_end; i++) {
        // Format line number with padding
        int line_no = i + 1;
        int curr_width = snprintf(line_num_buffer, sizeof(line_num_buffer), "%d", line_no);
        int padding = max_width - curr_width;
        
        // Ensure buffer has enough space
        size_t needed = format_len + max_width + strlen(lines[i]) + 10;
        if (needed >= format_size) {
            format_size *= 2;
            format = realloc(format, format_size);
        }
        
        // Add line number and padding
        format_len += sprintf(format + format_len, "[%*s%s]%s%s",
            padding, "", line_num_buffer,
            (i == _position->line_start - 1) ? " |> " : " |  ",
            lines[i]);
            
        if (i < line_end - 1) {
            format[format_len++] = '\n';
            format[format_len] = '\0';
        }
    }

    fprintf(stderr,
        "DEBUG(%s::%s:%d)[%s:%d:%d]: %s\n%s\n",
        _caller_file,
        _caller_func,
        _caller_line,
        _fpath,
        _position->line_start,
        _position->colm_start,
        _message,
        format
    );
    
    free(format);
    for (int i = 0; lines[i] != NULL; i++) {
        free(lines[i]);
    }
    free(lines);
    exit(EXIT_FAILURE);
}