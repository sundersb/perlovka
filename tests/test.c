#include <stdio.h>
#include "balance_test.h"
#include "solver_test.h"

int main()
{
    int test_result = 0;

    printf("\nTesting perlovka\n\n");
    
    // test_result += test_balancer();
    // test_result += test_signs();
    // test_result += test_complement();
    test_result += test_solvers_build();
    
    if (test_result)
    {
        printf("%u tests FAILED", test_result);
    }
    else
    {
        printf("Success!\n");
    }
    
    return 0;
}
