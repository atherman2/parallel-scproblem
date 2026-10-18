#include <stdlib.h>
#include "parallel_genetic_algorithm.h"
#include "constants.h"
#include "population.h"
#include "selection.h"
#include "crossover.h"
#include "mutation.h"
#include "local_search.h"

void set_interval(int* start, int* end, int* interval_size, int total, int thread_id) {
    if (thread_id == (k_threads - 1)) *interval_size = (total/k_threads) + (total % k_threads);
    else *interval_size = total/k_threads;
    *start = *interval_size * thread_id;
    *end = *start + *interval_size;
}

void* thread_worker(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    SCPInstance* instance = data->instance;
    int best_candidate_idx;
    int population_start_idx, population_end_idx, population_interval_size;
    int mutation_start_idx, mutation_end_idx, mutation_interval_size;
    int children_start_idx, children_end_idx, children_interval_size;
    // Set the indexes interval of operations of the thread
    set_interval(&population_start_idx, &population_end_idx, &population_interval_size, POPULATION_SIZE, data->thread_id);
    set_interval(&mutation_start_idx, &mutation_end_idx, &mutation_interval_size, mutation_count, data->thread_id);
    set_interval(&children_start_idx, &children_end_idx, &children_interval_size, PARENT_PAIRS_COUNT, data->thread_id);
    best_candidate_idx = fill_initial_population(population[population_start_idx], population_interval_size, data->instance, &data->seed);
    data->best_local_idx = best_candidate_idx + population_start_idx;
    data->best_local_cost = population[data->best_local_idx].cost;
    pthread_barrier_wait(&genetic_step_barrier);
}

void parallel_genetic_algorithm(SCPSolution* best, SCPInstance* instance, unsigned int* seed, double* convergence_history) {
    population = malloc(POPULATION_SIZE * sizeof(SCPSolution));
    children = malloc(PARENT_PAIRS_COUNT * sizeof(SCPSolution));
    int parents[2 * PARENT_PAIRS_COUNT];
    int history_size = 0;
    int best_idx;

    best_idx = fill_initial_population(population, POPULATION_SIZE, instance, seed);
    scp_solution_empty(best, instance);
    scp_solution_copy(best, &population[best_idx], instance);

    for (int i = 0; i < PARENT_PAIRS_COUNT; i++)
        scp_solution_empty(&children[i], instance);

#ifndef NO_LOCAL_SEARCH
    best_idx = apply_local_search(population, POPULATION_SIZE, instance);
#else
    best_idx = best_of_population(population, POPULATION_SIZE);
#endif
    update_best(best, &population[best_idx], instance);
    if (convergence_history)
        convergence_history[history_size++] = best->cost;

    for (int generation = 1; generation <= MAX_GENERATION; generation++) {
        select_parents(parents, 2 * PARENT_PAIRS_COUNT, POPULATION_SIZE, population, seed);

        best_idx = make_children(children, PARENT_PAIRS_COUNT, parents, population, instance);
        update_best(best, &children[best_idx], instance);

        best_idx = mutate_children(children, PARENT_PAIRS_COUNT, instance, seed);
        update_best(best, &children[best_idx], instance);

#ifndef NO_LOCAL_SEARCH
        best_idx = apply_local_search(children, PARENT_PAIRS_COUNT, instance);
#else
        best_idx = best_of_population(children, PARENT_PAIRS_COUNT);
#endif
        update_best(best, &children[best_idx], instance);

        elitism(population, POPULATION_SIZE, children, PARENT_PAIRS_COUNT);

        if (convergence_history)
            convergence_history[history_size++] = best->cost;
    }

    for (int i = 0; i < POPULATION_SIZE; i++)
        scp_solution_free(&population[i]);
    for (int i = 0; i < PARENT_PAIRS_COUNT; i++)
        scp_solution_free(&children[i]);
    free(population);
    free(children);
}
