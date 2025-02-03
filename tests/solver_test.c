#include <stdio.h>

#include "solver_test.h"
#include "../src/solver.h"

typedef struct Pair Pair;
typedef Pair *PPair;

typedef struct Pair
{
    ArmPosition first;
    ArmPosition second;
    PPair skip_to;
} Pair;

typedef struct
{
    bool (*match)(PCValue lhs, PCValue rhs);
    int (*get_delta)(PCValue lhs, PCValue rhs);
    int n_pairs;
    Pair pairs[];
} Solvers;

int index_of_pair(PPair start, PPair pair)
{
    return pair
               ? pair - start
               : -1;
}

void show_solver(Solvers *solver)
{
    PPair pair;
    printf("Pairs: %zu\n", solver->n_pairs);

    for (size_t index = 0; index < solver->n_pairs; ++index)
    {
        pair = &solver->pairs[index];
        printf("%u) (%d x %d) : (%d x %d) => %d\n",
        index,
        pair->first.a,
        pair->first.b,
        pair->second.a,
        pair->second.b,
        index_of_pair(solver->pairs, pair->skip_to));
    }
}

int test_odd_solvers_build()
{
    Solvers *solver;
    printf("Odd solver creation\n\n");

    solver = build_solver(10, 3, GRID_ODD, MATCHING_STRICT, RESOLVER_MINIMAL, true);

    show_solver(solver);

    clean_solver(solver);

    return 0;
}

int test_solvers_build()
{
    int fails = 0;
    fails += test_odd_solvers_build();
    // fails += test_even_solvers_build();
    return fails;
}