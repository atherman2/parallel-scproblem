#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "data_structure.h"
#include <pthread.h>

struct ThreadData {
    int thread_id, k_threads;
    SCPInstance* instance;
    unsigned int seed;
    int best_local_idx;
    double best_local_cost;
};
typedef struct ThreadData ThreadData;

bool_uchar iteration_end;
int k_threads, mutation_count;
int* selected_children_indexes;
SCPSolution *population, *children;
pthread_barrier_t genetic_step_barrier;

void set_interval(int* start, int* end, int* interval_size, int total, int thread_id);

void* thread_worker(void* arg);
/*
 * Runs the parallel version of the genetic algorithm and stores the best solution found in *best
 * (allocated here; release it with scp_solution_free).
 *
 * seed:                 state of the random generator used by this run (see random_generator.h)
 * convergence_history:  NULL, or an array with room for MAX_GENERATION + 1 values that
 *                       receives the best known cost after the initial population and
 *                       after each generation
 */
void parallel_genetic_algorithm(SCPSolution* best, SCPInstance* instance, unsigned int* seed, double* convergence_history);

#endif
