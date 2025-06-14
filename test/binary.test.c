#include "../src/api/ast/node.h"
#include "../src/api/ast/position.h"
#include "../src/api/core/vm.h"
#include "../src/api/core/generator.h"

int test_binary_add() {
    vm_init();


    ast_node_t** statements = malloc(sizeof(ast_node_t*) * 4);
    statements[0] = ast_statement_expression(
        position_from_line_and_colm(1, 1),
        ast_binary_add_node(
            position_from_line_and_colm(1, 1),
            ast_string_node(position_from_line_and_colm(3, 1), "Hello"),
            ast_string_node(position_from_line_and_colm(2, 1), "World")
        )
    );
    statements[1] = ast_statement_expression(
        position_from_line_and_colm(1, 7),
        ast_binary_add_node(
            position_from_line_and_colm(1, 7),
            ast_int_node(position_from_line_and_colm(1, 7), 2),
            ast_double_node(position_from_line_and_colm(1, 11), 3.14)
        )
    );
    statements[2] = ast_statement_expression(
        position_from_line_and_colm(1, 7),
        ast_logical_or_node(
            position_from_line_and_colm(1, 7),
            ast_int_node(position_from_line_and_colm(1, 7), 0),
            ast_int_node(position_from_line_and_colm(1, 11), 3)
        )
    );
    statements[3] = NULL;
    
    ast_node_t* program = ast_program_node(
        position_from_line_and_colm(1, 1),
        statements
    );

    generator_t* generator = generator_new("test.binary.hello", "2 + 2;2 + 3.14;");
    uint8_t* bytecode = generator_generate(generator, program);
    generator_free(generator);
    vm_run_main(bytecode);

    return 0;
}

