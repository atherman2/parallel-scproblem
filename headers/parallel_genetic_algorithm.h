#ifndef PARALLEL_GENETIC_ALGORITHM_H
#define PARALLEL_GENETIC_ALGORITHM_H

#include "data_structure.h"
#include <pthread.h>

/* Task the worker threads must run next. Set by the main thread before the start barrier. */
#define TASK_FILL_POPULATION 0   /* builds the initial population */
#define TASK_LOCAL_SEARCH    1   /* runs the local search on ls_solutions */
#define TASK_MUTATION        2   /* mutates the selected children */
#define TASK_STOP            3   /* algorithm finished, threads must exit */

struct ThreadData {
    int thread_id;
    SCPInstance* instance;
    unsigned int seed;
    int best_local_idx;
    double best_local_cost;
};
typedef struct ThreadData ThreadData;

void set_interval(int* start, int* end, int* interval_size, int total, int thread_id);

void* thread_genetic_algorithm(void* arg);

void update_best(SCPSolution* best, const SCPSolution* candidate, const SCPInstance* instance);

int best_idx_among_threads(SCPSolution* best, SCPInstance* instance);

int apply_local_search_parallel(SCPSolution* solutions, int num_solutions);

int mutate_children_parallel(int num_children, unsigned int* seed);

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
