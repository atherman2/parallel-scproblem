#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include "data_structure.h"

bool_uchar column_covers_all(int column_idx, const uint* rows_to_cover, uint num_rows_to_cover, const SCPInstance* instance);

void swap_local_search(SCPSolution* solution, SCPInstance* instance);

// Returns the index of the best solution
int apply_local_search(SCPSolution* solutions, int num_solutions, SCPInstance* instance);

#endif