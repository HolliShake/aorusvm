#include "../src/api/core/generator.h"
#include "../src/api/core/global.h"
#include "../src/api/core/vm.h"
#include "filereader.h"
#include "parser.h"

// The built-in print function.
void print_function(size_t _arg_count) {
    for (size_t i = 0; i < _arg_count; i++) {
        object_t* top = vm_pop();
        char* str = object_to_string(top);
        printf("%s", str);
        free(str);
        if (i < _arg_count - 1) {
            printf(" ");
        }
    }
    vm_load_null();
}

// he built-in println function.
void println_function(size_t _arg_count) {
    for (size_t i = 0; i < _arg_count; i++) {
        object_t* top = vm_pop();
        char* str = object_to_string(top);
        printf("%s", str);
        free(str);
        if (i < _arg_count - 1) {
            printf(" ");
        }
    }
    printf("\n");
    vm_load_null();
}

// The built-in scan function.
void scan_function(size_t _arg_count) {
    for (size_t i = 0; i < _arg_count; i++); // ignore the arguments
    char* input = malloc(1024);
    printf(">> ");
    scanf("%[^\n]", input);
    vm_push(object_new_string(input));
    free(input);
}

void custom_name_resolver(env_t* _env, char* _name) {
    // if (strcmp(_name, "print") == 0) {
    //    vm_push(object_new_native_function(1, (vm_native_function) print_function));
    //    return;
    // } else if (strcmp(_name, "println") == 0) {
    //     vm_push(object_new_native_function(1, (vm_native_function) println_function));
    //     return;
    // } else if (strcmp(_name, "scan") == 0) {
    //     vm_push(object_new_native_function(0, (vm_native_function) scan_function));
    //     return;
    // }
    vm_name_resolver(_env, _name);
}

int main(int argc, char** argv) {
    char* fpath = "./example.lang";
    char* content = file_read(fpath);
    parser_t* parser = parser_new(fpath, content);
    ast_node_t* node = parser_parse(parser);
    generator_t* generator = generator_new(fpath, content);
    uint8_t* bytecode = generator_generate(generator, node);
    generator_free(generator);
    vm_init();
    vm_set_name_resolver((vm_name_resolver_t) custom_name_resolver);
    vm_define_global("print", object_new_native_function(1, (vm_native_function) print_function));
    vm_define_global("println", object_new_native_function(1, (vm_native_function) println_function));
    vm_define_global("scan", object_new_native_function(0, (vm_native_function) scan_function));
    vm_run_main(bytecode);
    parser_free(parser);
    return 0;
}