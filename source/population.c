#include <stdlib.h>
#include <string.h>
#include "population.h"
#include "constants.h"
#include "random_generator.h"

static int rcl_min_position(const ColumnEntry* rcl, int size) {
    int min_pos = 0;
    for (int i = 1; i < size; i++)
        if (rcl[i].score < rcl[min_pos].score)
            min_pos = i;
    return min_pos;
}

/**
 * Semi-greedy step: builds a restricted candidate list with the COL_RCL_SIZE best columns
 * (uncovered rows covered / cost) and adds one of them at random.
 * Returns 0 if no column can cover any remaining row.
 */
static bool_uchar semi_greedy_add_column(uint* unused_columns, uint* num_unused,
                                         SCPSolution* solution, SCPInstance* instance,
                                         int* col_uncovered_count, bool_uchar* row_is_covered,
                                         unsigned int* seed) {
    ColumnEntry rcl[COL_RCL_SIZE];
    int rcl_size = 0;
    int min_pos = 0;  // position of the worst entry in the RCL (meaningful once it is full)

    for (uint i = 0; i < *num_unused; i++) {
        int col = (int)unused_columns[i];
        int count = col_uncovered_count[col];
        if (count == 0) continue;
        double score = (double)count / instance->columns[col].cost;

        if (rcl_size < COL_RCL_SIZE) {
            rcl[rcl_size].score = score;
            rcl[rcl_size].column = col;
            rcl_size++;
            if (rcl_size == COL_RCL_SIZE) min_pos = rcl_min_position(rcl, rcl_size);
        } else if (score > rcl[min_pos].score) {
            rcl[min_pos].score = score;
            rcl[min_pos].column = col;
            min_pos = rcl_min_position(rcl, rcl_size);
        }
    }

    if (rcl_size == 0) return 0;

    int chosen_col = rcl[random_in_range_int(seed, 0, rcl_size - 1)].column;
    scp_solution_add_column(solution, chosen_col, instance);

    // Updates how many uncovered rows each column still covers
    const SCPColumn* chosen = &instance->columns[chosen_col];
    for (uint i = 0; i < chosen->num_covered_rows; i++) {
        uint row_idx = chosen->covered_rows[i];
        if (row_is_covered[row_idx]) continue;

        row_is_covered[row_idx] = 1;
        const SCPRow* row = &instance->rows[row_idx];
        for (uint j = 0; j < row->num_covered_by; j++)
            col_uncovered_count[row->covered_by[j]]--;
    }

    // Removes the chosen column from the unused list (last element takes its place)
    for (uint i = 0; i < *num_unused; i++) {
        if ((int)unused_columns[i] == chosen_col) {
            unused_columns[i] = unused_columns[*num_unused - 1];
            (*num_unused)--;
            break;
        }
    }
    return 1;
}

void random_solution(SCPSolution* sol, SCPInstance* instance, unsigned int* seed) {
    uint num_unused = instance->num_columns;
    uint* unused_columns = malloc(instance->num_columns * sizeof(uint));
    int* col_uncovered_count = malloc(instance->num_columns * sizeof(int));
    bool_uchar* row_is_covered = calloc(instance->num_rows, sizeof(bool_uchar));

    memcpy(unused_columns, instance->columns_range, instance->num_columns * sizeof(uint));
    for (uint c = 0; c < instance->num_columns; c++)
        col_uncovered_count[c] = (int)instance->columns[c].num_covered_rows;

    while (sol->num_uncovered_rows > 0) {
        if (!semi_greedy_add_column(unused_columns, &num_unused, sol, instance,
                                    col_uncovered_count, row_is_covered, seed))
            break;  // Infeasible instance, avoids infinite loop
    }

    free(unused_columns);
    free(col_uncovered_count);
    free(row_is_covered);
}

int fill_initial_population(SCPSolution* population, int population_size, SCPInstance* instance, unsigned int* seed) {
    int best_idx = 0;
    for (int i = 0; i < population_size; i++) {
        scp_solution_empty(&population[i], instance);
        random_solution(&population[i], instance, seed);
        if (population[i].cost < population[best_idx].cost)
            best_idx = i;
    }
    return best_idx;
}

int best_of_population(const SCPSolution* solutions, int num_solutions) {
    int best_idx = 0;
    for (int i = 1; i < num_solutions; i++)
        if (solutions[i].cost < solutions[best_idx].cost)
            best_idx = i;
    return best_idx;
}
