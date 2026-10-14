#ifndef CROSSOVER_H
#define CROSSOVER_H

#include "data_structure.h"

/*
 * Fusion crossover: merges both parents, then tries to drop columns with the worst
 * cost/coverage ratio first, as long as the solution stays valid.
 * 'child' must be allocated; it is reset before being built.
 */
void crossover(SCPSolution* child, const SCPSolution* parent_a, const SCPSolution* parent_b, SCPInstance* instance);

/*
 * Builds children[i] from population[parents[2i]] and population[parents[2i+1]]
 * for i in [0, num_children). Returns the index of the cheapest child.
 */
int make_children(SCPSolution* children, int num_children, const int* parents,
                  const SCPSolution* population, SCPInstance* instance);

#endif
