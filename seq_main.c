#include "constants.h"
#include "data_structure.h"
#include "random_generator.h"
#include "genetic_algorithm.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[]) {
	char* file_path = (argc > 1)? argv[1] : "tests/Wren_03.dat";
	SCPInstance instance;
	if (!read_scp_instance(&instance, file_path)) return 1;
	SCPSolution best_sol;
	scp_solution_empty(&best_sol, &instance);
	unsigned int seed = random_seed_from_time();
	double* history = (double*) malloc(sizeof(double) * (MAX_GENERATION + 1));
	clock_t start = clock();
	genetic_algorithm(&best_sol, &instance, &seed, history);
	clock_t end = clock();
	double elapsed_seconds = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Sequential genetic algorithm finished in %f seconds\nSolution found cost: %f\nColumns used: ", elapsed_seconds, best_sol.cost);
	bool_uchar valid_sol = verify_scp_solution(&best_sol, &instance);
	scp_solution_print_columns_used(&best_sol);
	if (valid_sol) {
        printf("Sequential solution is valid and was sequentially verified\n");
    } else {
        printf("Sequential solution is not valid by the sequential verification\n");
    }
	free(history);
	return 0;
}
