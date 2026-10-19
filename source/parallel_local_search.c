#include "parallel_local_search.h"
#include "local_search.h"

/*
 * Applies the swap local search to every solution in the range [start, end)
 * and finds the cheapest one in that range.
 *
 * Each parallel thread calls this function for its own slice of the array, so
 * different threads never touch the same solution and no mutex is needed.
 *
 * Output:
 *   *best_idx: index of the best solution in the WHOLE array
 *   *best_cost: cost of that solution
 */
void local_search_range(SCPSolution* solutions, int start, int end, SCPInstance* instance,
                        int* best_idx, double* best_cost) {
    *best_idx = -1;
    *best_cost = 0.0;

    for (int i = start; i < end; i++) {
        swap_local_search(&solutions[i], instance); // Runs the local search on solution i

        // Keep it if it is the first one seen, or cheaper than the best so far
        if (*best_idx < 0 || solutions[i].cost < *best_cost) {
            *best_idx = i;
            *best_cost = solutions[i].cost;
        }
    }
}
