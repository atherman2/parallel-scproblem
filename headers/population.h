#ifndef POPULATION_H
#define POPULATION_H

#include "data_structure.h"

typedef struct {
    double score;
    int column;
} ColumnEntry;

/* Builds a solution with a semi-greedy randomized construction. 'sol' must be empty. */
void random_solution(SCPSolution* sol, SCPInstance* instance, unsigned int* seed);

/*
 * Allocates and fills population[0..population_size-1] with random solutions.
 * Returns the index of the cheapest one.
 */
int fill_initial_population(SCPSolution* population, int population_size, SCPInstance* instance, unsigned int* seed);

/* Returns the index of the cheapest solution */
int best_of_population(const SCPSolution* solutions, int num_solutions);

#endif
