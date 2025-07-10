#include <assert.h>
#include "../src/api/ast/node.h"
#include "../src/api/ast/position.h"
#include "../src/api/core/object.h"
#include "../src/api/core/vm.h"
#include "../src/api/core/generator.h"

void test_function_node() {
    // Create test position
    position_t* pos = position_from_line_and_colm(1, 1);
    
    // Create function name node
    ast_node_t* name = ast_name_node(pos, "add");
    
    // Create parameter list with two params
    ast_node_list_t params = malloc(sizeof(ast_node_t*) * 3);
    params[0] = ast_name_node(pos, "a");
    params[1] = ast_name_node(pos, "b");
    params[2] = NULL;
    
    // Create function body that adds params and returns
    ast_node_list_t body = malloc(sizeof(ast_node_t*) * 2);
    ast_node_t* add = ast_binary_add_node(pos,
        ast_name_node(pos, "a"),
        ast_name_node(pos, "b")
    );
    body[0] = ast_return_statement_node(pos, add);
    body[1] = NULL;
    
    // Create function node
    ast_node_t* func = ast_async_function_node(pos, name, params, body);

    ast_node_list_t arguments = malloc(sizeof(ast_node_t*) * 3);
    arguments[0] = ast_int_node(pos, 1);
    arguments[1] = ast_int_node(pos, 20);
    arguments[2] = NULL;

    ast_node_t* call = ast_call_node(pos, ast_name_node(pos, "add"), arguments);

    ast_node_list_t statements = malloc(sizeof(ast_node_t*) * 3);
    statements[0] = func;
    statements[1] = ast_expression_statement_node(pos, call);
    statements[2] = NULL;

    ast_node_t* program = ast_program_node(
        position_from_line_and_colm(1, 1),
        statements
    );
    
    generator_t* generator = generator_new("test.function.add", "func add(a, b) { a + b; }");
    uint8_t* bytecode = generator_generate(generator, program);
    generator_free(generator);
    vm_run_main(bytecode);
}


