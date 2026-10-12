#include "data_structure.h"

/**
 * Removes a column from the solution.
 * Updates columns_used, column_used and cost, and adds to uncovered_rows
 * every row of the column that no other column in the solution covers.
 */
void scp_solution_remove_column(SCPSolution* sol, int column_idx, SCPInstance* instance) {
	// Removes column_idx from columns_used (order is not preserved: last element takes its place)
	for (uint i = 0; i < sol->num_columns_used; i++) {
		if ((int)sol->columns_used[i] == column_idx) {
			sol->columns_used[i] = sol->columns_used[sol->num_columns_used - 1];
			sol->num_columns_used--;
			break;
		}
	}
	sol->column_used[column_idx] = 0;

	SCPColumn* column = &instance->columns[column_idx];
	sol->cost -= column->cost;

	for (uint i = 0; i < column->num_covered_rows; i++) {
		uint row_idx = column->covered_rows[i];
		SCPRow* row = &instance->rows[row_idx];

		bool_uchar still_covered = 0;
		for (uint j = 0; j < row->num_covered_by; j++) {
			uint other_column_idx = row->covered_by[j];
			if ((int)other_column_idx != column_idx && sol->column_used[other_column_idx]) {
				still_covered = 1;
				break;
			}
		}

		if (!still_covered)
			sol->uncovered_rows[sol->num_uncovered_rows++] = row_idx;
	}
}

/**
 * Adds a column to the solution.
 * Updates columns_used, column_used and cost, and removes from uncovered_rows
 * the rows that the column covers.
 */
void scp_solution_add_column(SCPSolution* sol, int column_idx, SCPInstance* instance) {
	sol->columns_used[sol->num_columns_used++] = (uint)column_idx;
	sol->column_used[column_idx] = 1;

	SCPColumn* column = &instance->columns[column_idx];
	sol->cost += column->cost;

	for (uint i = 0; i < column->num_covered_rows; i++) {
		uint row_idx = column->covered_rows[i];

		// Searches the row in uncovered_rows and removes it (last element takes its place)
		for (uint j = 0; j < sol->num_uncovered_rows; j++) {
			if (sol->uncovered_rows[j] == row_idx) {
				sol->uncovered_rows[j] = sol->uncovered_rows[sol->num_uncovered_rows - 1];
				sol->num_uncovered_rows--;
				break;
			}
		}
	}
}