


#include "../src/api/ast/node.h"
#include "../src/api/ast/position.h"
#include "../src/api/core/vm.h"
#include "../src/api/core/generator.h"

int test_variable() {
    vm_init();

    ast_node_t** names = malloc(sizeof(ast_node_t*) * 2);
    names[0] = ast_name_node(position_from_line_and_colm(1, 1), "pi");
    names[1] = NULL;

    ast_node_t** values = malloc(sizeof(ast_node_t*) * 2);
    values[0] = ast_double_node(position_from_line_and_colm(1, 1), 3.14);
    values[1] = NULL;

    ast_node_t** statements = malloc(sizeof(ast_node_t*) * 2);
    statements[0] = ast_var_statement(
        position_from_line_and_colm(1, 1),
        names,
        values
    );
    statements[1] = NULL;

    ast_node_t* program = ast_program_node(
        position_from_line_and_colm(1, 1),
        statements
    );

    generator_t* generator = generator_new("test.variable.hello", "var pi = 3.14;");
    uint8_t* bytecode = generator_generate(generator, program);
    generator_free(generator);
    vm_run_main(bytecode);

    return 0;
}

