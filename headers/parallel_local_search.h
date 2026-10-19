#ifndef PARALLEL_LOCAL_SEARCH_H
#define PARALLEL_LOCAL_SEARCH_H

#include "data_structure.h"

/* Applies swap_local_search to solutions[start...end-1] and returns the best one in the range. */
void local_search_range(SCPSolution* solutions, int start, int end, SCPInstance* instance,
                        int* best_idx, double* best_cost);

#endif
