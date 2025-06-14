#include <stdio.h>
#include <stdlib.h>
#include "src/api/core/vm.h"
#include "test/binary.test.c"
#include "test/variable.test.c"
#include "test/if.test.c"

int main(int argc, char *argv[]) {
    // test_binary_add();
    // test_variable();
    test_if_statement();
    // test_vm();
    return 0;
}
