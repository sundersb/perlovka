#include <stdio.h>
#include "../src/balance.h"

struct balance_test {
    int a;
    int b;
    SignBalance balance;
} balance_tests[] = {
    {-1, -1, BALANCE_NEGATIVE },
    {-1, 0, BALANCE_SOFT_NEGATIVE},
    {-1, 1, BALANCE_DIFFERENT},
    {0, -1, BALANCE_SOFT_NEGATIVE},
    {0, 0, BALANCE_ZERO},
    {0, 1, BALANCE_SOFT_POSITIVE},
    {1, -1, BALANCE_DIFFERENT},
    {1, 0, BALANCE_SOFT_POSITIVE},
    {1, 1, BALANCE_POSITIVE},
};

struct sign_test {
    int balance;
    bool (*func) (SignBalance);
    bool expected;
} sign_tests[] = {
    {BALANCE_POSITIVE, is_positive, true},
    {BALANCE_SOFT_POSITIVE, is_positive, true},
    {BALANCE_NEGATIVE, is_negative, true},
    {BALANCE_SOFT_NEGATIVE, is_negative, true},

    {BALANCE_ZERO, is_positive, false},
    {BALANCE_DIFFERENT, is_positive, false},
    {BALANCE_NEGATIVE, is_positive, false},
    {BALANCE_SOFT_NEGATIVE, is_positive, false},

    {BALANCE_ZERO, is_negative, false},
    {BALANCE_DIFFERENT, is_negative, false},
    {BALANCE_POSITIVE, is_negative, false},
    {BALANCE_SOFT_POSITIVE, is_negative, false},
};

struct complement_test {
    int a;
    int b;
    bool (*func) (SignBalance, SignBalance);
    bool expected;
} should_be_soft_complement[] = {
    {BALANCE_NEGATIVE, BALANCE_POSITIVE, are_soft_complement, true},
    {BALANCE_NEGATIVE, BALANCE_SOFT_POSITIVE, are_soft_complement, true},
    {BALANCE_SOFT_NEGATIVE, BALANCE_POSITIVE, are_soft_complement, true},
    {BALANCE_POSITIVE, BALANCE_NEGATIVE, are_soft_complement, true},
    {BALANCE_POSITIVE, BALANCE_SOFT_NEGATIVE, are_soft_complement, true},
    {BALANCE_SOFT_POSITIVE, BALANCE_NEGATIVE, are_soft_complement, true},
},
should_not_be_soft_complement[] = {
    {BALANCE_NEGATIVE, BALANCE_DIFFERENT, are_soft_complement, false},
    {BALANCE_NEGATIVE, BALANCE_ZERO, are_soft_complement, false},
    {BALANCE_NEGATIVE, BALANCE_NEGATIVE, are_soft_complement, false},
    {BALANCE_NEGATIVE, BALANCE_SOFT_NEGATIVE, are_soft_complement, false},

    {BALANCE_SOFT_NEGATIVE, BALANCE_DIFFERENT, are_soft_complement, false},
    {BALANCE_SOFT_NEGATIVE, BALANCE_ZERO, are_soft_complement, false},
    {BALANCE_SOFT_NEGATIVE, BALANCE_NEGATIVE, are_soft_complement, false},
    {BALANCE_SOFT_NEGATIVE, BALANCE_SOFT_NEGATIVE, are_soft_complement, false},
    {BALANCE_SOFT_NEGATIVE, BALANCE_SOFT_POSITIVE, are_soft_complement, false},

    {BALANCE_DIFFERENT, BALANCE_NEGATIVE, are_soft_complement, false},
    {BALANCE_DIFFERENT, BALANCE_SOFT_NEGATIVE, are_soft_complement, false},
    {BALANCE_DIFFERENT, BALANCE_ZERO, are_soft_complement, false},
    {BALANCE_DIFFERENT, BALANCE_POSITIVE, are_soft_complement, false},
    {BALANCE_DIFFERENT, BALANCE_SOFT_POSITIVE, are_soft_complement, false},
    {BALANCE_DIFFERENT, BALANCE_DIFFERENT, are_soft_complement, false},

    {BALANCE_ZERO, BALANCE_NEGATIVE, are_soft_complement, false},
    {BALANCE_ZERO, BALANCE_SOFT_NEGATIVE, are_soft_complement, false},
    {BALANCE_ZERO, BALANCE_ZERO, are_soft_complement, false},
    {BALANCE_ZERO, BALANCE_POSITIVE, are_soft_complement, false},
    {BALANCE_ZERO, BALANCE_SOFT_POSITIVE, are_soft_complement, false},
    {BALANCE_ZERO, BALANCE_DIFFERENT, are_soft_complement, false},

    {BALANCE_POSITIVE, BALANCE_DIFFERENT, are_soft_complement, false},
    {BALANCE_POSITIVE, BALANCE_ZERO, are_soft_complement, false},
    {BALANCE_POSITIVE, BALANCE_POSITIVE, are_soft_complement, false},
    {BALANCE_POSITIVE, BALANCE_SOFT_POSITIVE, are_soft_complement, false},

    {BALANCE_SOFT_POSITIVE, BALANCE_DIFFERENT, are_soft_complement, false},
    {BALANCE_SOFT_POSITIVE, BALANCE_ZERO, are_soft_complement, false},
    {BALANCE_SOFT_POSITIVE, BALANCE_POSITIVE, are_soft_complement, false},
    {BALANCE_SOFT_POSITIVE, BALANCE_SOFT_POSITIVE, are_soft_complement, false},
    {BALANCE_SOFT_POSITIVE, BALANCE_SOFT_NEGATIVE, are_soft_complement, false},
};

int test_balancer()
{
    int balance;
    struct balance_test *test;
    int index;
    int fails = 0;
    size_t length = sizeof(balance_tests) / sizeof(balance_tests[0]);
    
    printf("Balancer\n");
    
    for (index = 0; index < length; ++index)
    {
        test = &balance_tests[index];

        printf("%u) balance %i:%i (%u)", index + 1, test->a, test->b, test->balance);

        balance = balance_of(test->a, test->b);

        if (balance != test->balance)
        {
            printf("- fail\n");
            ++fails;
        }
        else
        {
            printf(" - OK\n");
        }
    }
    
    printf("\n");
    
    return fails;
}

int test_signs()
{
    int fails = 0;
    int index;
    struct sign_test *test;
    bool actual;
    size_t size = sizeof(sign_tests) / sizeof(sign_tests[0]);

    printf("Signs\n");
    
    for (index = 0; index < size; ++index)
    {
        test = &sign_tests[index];
        printf("%u) %u -> %u", index+1, test->balance, test->expected);

        actual = test->func(test->balance);

        if (actual == test->expected)
        {
            printf(" - OK\n");
        }
        else
        {
            ++fails;
            printf(" - FAIL!\n");
        }
    }
    
    printf("\n");

    return fails;
}

int test_complement()
{
    struct complement_test *test;
    size_t size = sizeof(should_be_soft_complement) / sizeof(should_be_soft_complement[0]);
    int fails = 0;
    int index;
    bool actual;
    
    printf("Soft complement\n");
    
    for (index = 0; index < size; ++index)
    {
        test = &should_be_soft_complement[index];
        printf("%u) %u:%u -> %u", index+1, test->a, test->b, test->expected);

        actual = test->func(test->a, test->b);

        if (actual == test->expected)
        {
            printf(" - OK\n");
        }
        else
        {
            ++fails;
            printf(" - FAIL!\n");
        }
    }
    
    printf("\n");
    
    printf("Should not soft complement\n");
    size = sizeof(should_not_be_soft_complement) / sizeof(should_not_be_soft_complement[0]);
    
    for (index = 0; index < size; ++index)
    {
        test = &should_not_be_soft_complement[index];
        printf("%u) %u:%u -> %u", index+1, test->a, test->b, test->expected);

        actual = test->func(test->a, test->b);

        if (actual == test->expected)
        {
            printf(" - OK\n");
        }
        else
        {
            ++fails;
            printf(" - FAIL!\n");
        }
    }
    
    printf("\n");

    return fails;
}
