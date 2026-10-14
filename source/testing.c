#include <stdio.h>
#include <stdlib.h>
#include "data_structure.h"
#include "genetic_algorithm.h"
#include "random_generator.h"
#include "util.h"

#define RUNS_PER_INSTANCE 10

int main(void) {
    // List of instances to test
    char* instance_files[] = {
        "Testing/Teste_01.dat",
        "Testing/Teste_02.dat",
        "Testing/Teste_03.dat",
        "Testing/Teste_04.dat",
        "Testing/Teste_05.dat",
        "Testing/Wren_01.dat",
        "Testing/Wren_02.dat",
        "Testing/Wren_03.dat",
        "Testing/Wren_04.dat",
    };
    const int num_files = (int)(sizeof(instance_files) / sizeof(instance_files[0]));

    make_directory("results");
#ifndef NO_LOCAL_SEARCH
    const char* csv_path = "results/results_with_ls.csv";
#else
    const char* csv_path = "results/results_without_ls.csv";
#endif
    FILE* csv = fopen(csv_path, "w");
    if (!csv) {
        fprintf(stderr, "Error opening %s\n", csv_path);
        return 1;
    }
    fprintf(csv, "instance,run,cost,time_seconds\n");

    unsigned int base_seed = random_seed_from_time();
    unsigned int stream_id = 0;

    for (int f = 0; f < num_files; f++) {
        SCPInstance instance;
        if (!read_scp_instance(&instance, instance_files[f])) {
            fprintf(stderr, "Error reading %s\n", instance_files[f]);
            continue;
        }

        printf("Running instance: %s\n", instance_files[f]);

        for (int run = 1; run <= RUNS_PER_INSTANCE; run++) {
            unsigned int seed;
            random_generator_init(&seed, base_seed, stream_id++);

            SCPSolution best;
            double start = wall_time_seconds();
            genetic_algorithm(&best, &instance, &seed, NULL);
            double elapsed = wall_time_seconds() - start;

            printf("  Run %d -> cost = %.2f | time = %gs\n", run, best.cost, elapsed);
            fprintf(csv, "%s,%d,%.2f,%g\n", instance_files[f], run, best.cost, elapsed);

            scp_solution_free(&best);
        }

        free_scp_instance(&instance);
    }

    fclose(csv);
    printf("\nResults saved in %s\n", csv_path);
    return 0;
}
