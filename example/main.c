#include "../src/api/core/generator.h"
#include "../src/api/core/global.h"
#include "../src/api/core/vm.h"
#include "filereader.h"
#include "parser.h"

int main(int argc, char** argv) {
    const char* fpath = "./example.lang";
    char* content = file_read(fpath);
    parser_t* parser = parser_new(fpath, content);
    ast_node_t* node = parser_parse(parser);
    generator_t* generator = generator_new(fpath, content);
    uint8_t* bytecode = generator_generate(generator, node);
    
    generator_free(generator);
    vm_init();
    vm_run_main(bytecode);
    parser_free(parser);
    return 0;
}