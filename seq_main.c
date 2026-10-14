#include "constants.h"
#include "data_structure.h"
#include "random_generator.h"
#include "genetic_algorithm.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
	char* file_path = (argc > 1)? argv[1] : "tests/Wren03.dat";
	SCPInstance instance;
	if (!read_scp_instance(&instance, file_path)) return 1;
	SCPSolution best_sol;
	unsigned int seed = random_seed_from_time();
	double* history = (double*) malloc(sizeof(double) * (MAX_GENERATION + 1));
	genetic_algorithm(&best_sol, &instance, &seed, history);
	printf("%f\n", best_sol.cost);
	scp_solution_print_columns_used(&best_sol);
	free(history);
	return 0;
}
