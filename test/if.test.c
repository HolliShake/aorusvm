#include "../src/api/ast/node.h"
#include "../src/api/ast/position.h"
#include "../src/api/core/vm.h"
#include "../src/api/core/generator.h"

int test_if_statement() {
    vm_init();

    ast_node_t** statements = malloc(sizeof(ast_node_t*) * 2);
    statements[0] = ast_if_statement(
        position_from_line_and_colm(1, 1),
        ast_logical_and_node(position_from_line_and_colm(1, 1), ast_boolean_node(position_from_line_and_colm(1, 1), true), ast_boolean_node(position_from_line_and_colm(1, 1), false)),
        ast_statement_expression(position_from_line_and_colm(1, 1), ast_string_node(position_from_line_and_colm(1, 1), "Hello")),
        ast_statement_expression(position_from_line_and_colm(1, 1), ast_string_node(position_from_line_and_colm(1, 1), "World"))
    );
    statements[1] = NULL;
    
    ast_node_t* program = ast_program_node(
        position_from_line_and_colm(1, 1),
        statements
    );

    generator_t* generator = generator_new("test.if.hello", "if (true) \"Hello\" else \"World\"");
    uint8_t* bytecode = generator_generate(generator, program);
    generator_free(generator);
    vm_run_main(bytecode);

    return 0;
}

