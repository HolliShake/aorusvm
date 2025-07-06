#include "../src/api/core/generator.h"
#include "../src/api/core/global.h"
#include "../src/api/core/vm.h"
#include "filereader.h"
#include "parser.h"


// The built-in print function.
void print_function(size_t _arg_count) {
    for (size_t i = 0; i < _arg_count; i++) {
        object_t* top = vm_pop();
        printf("%s", object_to_string(top));
        if (i < _arg_count - 1) {
            printf(" ");
        }
    }
    printf("\n");
    vm_load_null();
}

void custom_name_resolver(env_t* _env, char* _name) {
    if (strcmp(_name, "print") == 0) {
       vm_push(object_new_native_function(1, (vm_native_function) print_function));
    } else {
        // Default name resolver (important!).
        vm_name_resolver(_env, _name);
    }
}

int main(int argc, char** argv) {
    const char* fpath = "./example.lang";
    char* content = file_read(fpath);
    parser_t* parser = parser_new(fpath, content);
    ast_node_t* node = parser_parse(parser);
    generator_t* generator = generator_new(fpath, content);
    uint8_t* bytecode = generator_generate(generator, node);
    generator_free(generator);
    vm_init();
    vm_set_name_resolver((vm_name_resolver_t) custom_name_resolver);
    vm_run_main(bytecode);
    parser_free(parser);
    return 0;
}