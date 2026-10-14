#include <stdio.h>
#include <stdlib.h>
#include "data_structure.h"
#include "genetic_algorithm.h"
#include "random_generator.h"
#include "util.h"

/* Usage: main [instance_file] [base_seed] */
int main(int argc, char* argv[]) {
    char* file_path = (argc > 1) ? argv[1] : "Testing/Teste_01.dat";
    unsigned int base_seed = (argc > 2) ? (unsigned int)strtoul(argv[2], NULL, 10) : random_seed_from_time();

    SCPInstance instance;
    if (!read_scp_instance(&instance, file_path))
        return 1;

    printf("Number of rows:    %u\n", instance.num_rows);
    printf("Number of columns: %u\n\n", instance.num_columns);
    printf("Base seed: %u\n", base_seed);

    unsigned int seed;
    random_generator_init(&seed, base_seed, 0);

    SCPSolution best;
    double start = wall_time_seconds();
    genetic_algorithm(&best, &instance, &seed, NULL);
    double elapsed = wall_time_seconds() - start;

    printf("Algorithm execution ended in %g seconds.\n", elapsed);
    printf("Columns used by solution:\n");
    scp_solution_print_columns_used(&best);
    printf("\nSolution cost: %.2f\n", best.cost);
    if (!is_scp_solution_valid(&best))
        printf("WARNING: the solution does not cover all rows.\n");

    scp_solution_free(&best);
    free_scp_instance(&instance);
    return 0;
}
