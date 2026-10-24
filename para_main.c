#include "constants.h"
#include "data_structure.h"
#include "random_generator.h"
#include "parallel_genetic_algorithm.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[]) {
	char* file_path = (argc > 1)? argv[1] : "tests/Wren_03.dat";
	SCPInstance instance;
	if (!read_scp_instance(&instance, file_path)) return 1;
	int repeat = 1;
	while (repeat) {
    	SCPSolution best_sol;
        scp_solution_empty(&best_sol, &instance);
        best_sol.cost = INFINITY;
    	unsigned int seed = random_seed_from_time();
    	double* history = (double*) malloc(sizeof(double) * (MAX_GENERATION + 1));
    	parallel_genetic_algorithm(&best_sol, &instance, &seed, history);
    	scp_solution_print_columns_used(&best_sol);
    	free(history);
        bool_uchar valid_sol = verify_scp_solution(&best_sol, &instance);
        if (valid_sol) {
            printf("Parallel solution is valid and was sequentially verified\n");
        } else {
            printf("Parallel solution is not valid by the sequential verification\n");
        }
        scp_solution_free(&best_sol);
        printf("Press 1 to run the algorithm again, 0 to exit: ");
        scanf("%d", &repeat);
	}
	free_scp_instance(&instance);
	return 0;
}
