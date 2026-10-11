#include <stdlib.h>
#include "data_structure.h"
#include "local_search.h"

/**
 * Checks if a given column covers all rows in the provided list.
 * Returns true if the column covers every row.
 */
bool_uchar column_covers_all(int column_idx, const uint* rows_to_cover, uint num_rows_to_cover, const SCPInstance* instance) {
	const SCPColumn* column = &instance->columns[column_idx];

	for (uint i = 0; i < num_rows_to_cover; i++) {
		if (!column->covers_row[rows_to_cover[i]])
			return 0;
	}

	return 1;
}

/** Local Search Swap
 * Tries to improve the current solution by replacing columns with cheaper ones.
 * For each column in the solution, attempts to remove it.
 * If the removal breaks coverage, it looks for a cheaper column that can cover all uncovered rows. If found, it replaces the original; otherwise, it restores it.
 */
void swap_local_search(SCPSolution* solution, SCPInstance* instance) {
	uint num_columns_to_check = solution->num_columns_used;
	if (num_columns_to_check == 0) return;

	// Copy of columns_used, since the original changes during the loop
	uint* columns_to_check = malloc(num_columns_to_check * sizeof(uint));
	for (uint i = 0; i < num_columns_to_check; i++)
		columns_to_check[i] = solution->columns_used[i];

	for (uint i = 0; i < num_columns_to_check; i++) {
		int column_a_idx = (int)columns_to_check[i];
		double column_a_cost = instance->columns[column_a_idx].cost;

		// Try removing column A
		scp_solution_remove_column(solution, column_a_idx, instance);

		// If solution is still valid, the column was redundant and we keep it removed
		if (solution->num_uncovered_rows == 0)
			continue;

		// Try to find a better column to replace it
		int best_b_idx = -1;
		double best_b_cost = column_a_cost;

		for (uint column_b_idx = 0; column_b_idx < instance->num_columns; column_b_idx++) {
			if (solution->column_used[column_b_idx])
				continue;	// Skip columns already in the solution

			double column_b_cost = instance->columns[column_b_idx].cost;

			if (column_b_cost >= best_b_cost)
				continue;	// Skip if not cheaper than current best

			// Check if this column covers all currently uncovered rows
			if (column_covers_all((int)column_b_idx, solution->uncovered_rows, solution->num_uncovered_rows, instance)) {
				best_b_idx = (int)column_b_idx;
				best_b_cost = column_b_cost;
			}
		}

		if (best_b_idx != -1)
			scp_solution_add_column(solution, best_b_idx, instance); // If a better column was found, add it
		else
			scp_solution_add_column(solution, column_a_idx, instance); // Or else, restore the original column
	}

	free(columns_to_check);
}

int apply_local_search(SCPSolution* solutions, int num_solutions, SCPInstance* instance) {
	if (num_solutions <= 0) return -1;

	swap_local_search(&solutions[0], instance);
	int best_idx = 0;
	for (int i = 1; i < num_solutions; i++) {
		swap_local_search(&solutions[i], instance);
		if (solutions[i].cost < solutions[best_idx].cost)
			best_idx = i;
	}
	return best_idx;
}