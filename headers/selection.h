#ifndef SELECTION_H
#define SELECTION_H

#include "data_structure.h"

/*
 * Tournament selection (TOURNAMENT_SIZE participants). Fills parents[0..num_parents-1]
 * with distinct population indices. Requires num_parents <= population_size.
 */
void select_parents(int* parents, int num_parents, int population_size,
                    const SCPSolution* population, unsigned int* seed);

/*
 * Survivor selection: keeps the ELITISM_RATE best individuals of the population and
 * replaces the other slots, in index order, with the children. Replacement swaps the
 * solutions, so the children array ends up holding the discarded individuals
 * (all buffers stay valid and reusable).
 */
void elitism(SCPSolution* population, int population_size, SCPSolution* children, int num_children);

#endif
