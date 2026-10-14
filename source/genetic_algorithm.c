#include <stdlib.h>
#include "genetic_algorithm.h"
#include "constants.h"
#include "population.h"
#include "selection.h"
#include "crossover.h"
#include "mutation.h"
#include "local_search.h"

// Keeps in 'best' the cheaper of the two solutions
static void update_best(SCPSolution* best, const SCPSolution* candidate, const SCPInstance* instance) {
    if (candidate->cost < best->cost)
        scp_solution_copy(best, candidate, instance);
}

void genetic_algorithm(SCPSolution* best, SCPInstance* instance, unsigned int* seed, double* convergence_history) {
    SCPSolution* population = malloc(POPULATION_SIZE * sizeof(SCPSolution));
    SCPSolution* children = malloc(PARENT_PAIRS_COUNT * sizeof(SCPSolution));
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
