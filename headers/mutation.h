#ifndef MUTATION_H
#define MUTATION_H

#include "data_structure.h"

// seed is the state of the random number generator
void mutation(SCPSolution* solution, SCPInstance* instance, unsigned int* seed);

// Returns the index of the best mutated child
int mutate_children(SCPSolution* children, int num_children, SCPInstance* instance, unsigned int* seed);

#endif