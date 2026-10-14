#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "data_structure.h"

/*
 * Runs the genetic algorithm and stores the best solution found in *best
 * (allocated here; release it with scp_solution_free).
 *
 * seed:                 state of the random generator used by this run (see random_generator.h)
 * convergence_history:  NULL, or an array with room for MAX_GENERATION + 1 values that
 *                       receives the best known cost after the initial population and
 *                       after each generation
 */
void genetic_algorithm(SCPSolution* best, SCPInstance* instance, unsigned int* seed, double* convergence_history);

#endif
