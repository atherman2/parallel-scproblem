#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structure.h"
#include "genetic_algorithm.h"
#include "constants.h"
#include "random_generator.h"
#include "util.h"

// Extracts a clean instance name, e.g. "Testing/Wren_04.dat" -> "Wren_04"
static void clean_instance_name(const char* path, char* out, size_t size) {
    const char* base = path;
    const char* slash = strrchr(path, '/');
    const char* backslash = strrchr(path, '\\');
    if (slash && slash + 1 > base) base = slash + 1;
    if (backslash && backslash + 1 > base) base = backslash + 1;

    snprintf(out, size, "%s", base);
    char* dot = strrchr(out, '.');
    if (dot) *dot = '\0';
}

int main(void) {
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
    make_directory("results/convergence");

    unsigned int base_seed = random_seed_from_time();
    double history[MAX_GENERATION + 1];

    for (int f = 0; f < num_files; f++) {
        SCPInstance instance;
        if (!read_scp_instance(&instance, instance_files[f])) {
            fprintf(stderr, "Error reading %s\n", instance_files[f]);
            continue;
        }

        unsigned int seed;
        random_generator_init(&seed, base_seed, (unsigned int)f);

        SCPSolution best;
        genetic_algorithm(&best, &instance, &seed, history);
        const int history_size = MAX_GENERATION + 1;

        char clean_name[256];
        char out_path[512];
        clean_instance_name(instance_files[f], clean_name, sizeof(clean_name));
#ifndef NO_LOCAL_SEARCH
        snprintf(out_path, sizeof(out_path), "results/convergence/%s_with_ls.csv", clean_name);
#else
        snprintf(out_path, sizeof(out_path), "results/convergence/%s_without_ls.csv", clean_name);
#endif

        FILE* csv = fopen(out_path, "w");
        if (csv) {
            fprintf(csv, "generation,best_cost\n");
            for (int gen = 0; gen < history_size; gen++)
                fprintf(csv, "%d,%.2f\n", gen, history[gen]);
            fclose(csv);
        } else {
            fprintf(stderr, "Error opening %s\n", out_path);
        }

        printf("%s -> final cost = %.2f (%d generations) saved to %s\n",
               instance_files[f], best.cost, history_size, out_path);

        scp_solution_free(&best);
        free_scp_instance(&instance);
    }

    return 0;
}
