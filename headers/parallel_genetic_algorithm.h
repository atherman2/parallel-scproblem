#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "data_structure.h"
#include <pthread.h>

struct ThreadData {
    int thread_id;
    SCPInstance* instance;
    unsigned int seed;
    int best_local_idx;
    double best_local_cost;
};
typedef struct ThreadData ThreadData;

ThreadData* threads_data;
bool_uchar iterations_finished;
int k_threads, mutation_count;
int* selected_children_indexes;
SCPSolution *population, *children;
pthread_barrier_t thread_work_start, thread_work_done;
pthread_t* threads;
int * thread_ret;

void set_interval(int* start, int* end, int* interval_size, int total, int thread_id);

void* thread_genetic_algorithm(void* arg);

int best_idx_among_threads(SCPSolution* best, SCPInstance* instance);

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
