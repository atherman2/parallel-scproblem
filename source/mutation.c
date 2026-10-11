#include <stdlib.h>
#include "data_structure.h"
#include "constants.h"
#include "mutation.h"

// Replaces RandomGenerator: xorshift32 generator, advances *seed and returns the next number
static unsigned int next_random(unsigned int* seed) {
    unsigned int x = (*seed == 0) ? 2463534242u : *seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *seed = x;
    return x;
}

// Replaces RandomGenerator::inRange: random integer in [min, max]
static int random_in_range(unsigned int* seed, int min, int max) {
    return min + (int)(next_random(seed) % (unsigned int)(max - min + 1));
}

/**
 * Mutation operator
 * Removes k (MUTATION_K) random columns from the solution.
 * Repairs by randomly picking columns that cover uncovered rows.
 * If a row has no available column to cover it, restores a removed column as fallback.
 */
void mutation(SCPSolution* solution, SCPInstance* instance, unsigned int* seed) {
    if (solution->num_columns_used == 0) return;

    // Stores removed columns in case we need them for fallback
    uint* removed_cols = malloc((MUTATION_K + 1) * sizeof(uint));
    int removed_count = 0;
    for (int i = 0; i < MUTATION_K && solution->num_columns_used > 0; i++) {
        int random_idx = random_in_range(seed, 0, (int)solution->num_columns_used - 1);
        uint col = solution->columns_used[random_idx];
        removed_cols[removed_count++] = col;
        scp_solution_remove_column(solution, (int)col, instance);
    }

    // Picks a random uncovered row and a random column that covers it
    uint* candidates = malloc(instance->num_columns * sizeof(uint));
    int max_attempts = (int)solution->num_uncovered_rows * 2;
    int failed_attempts = 0;

    while (solution->num_uncovered_rows > 0) {
        if (failed_attempts > max_attempts) {  // Fallback
            if (removed_count == 0) break;  // Invalid instance, avoids infinite loop
            removed_count--;
            scp_solution_add_column(solution, (int)removed_cols[removed_count], instance);
            failed_attempts = 0;
            continue;
        }

        // Picks a random uncovered row
        int row_idx = random_in_range(seed, 0, (int)solution->num_uncovered_rows - 1);
        uint row = solution->uncovered_rows[row_idx];

        int num_candidates = 0;
        for (uint j = 0; j < instance->rows[row].num_covered_by; j++) {
            uint col = instance->rows[row].covered_by[j];
            if (!solution->column_used[col])
                candidates[num_candidates++] = col;
        }

        if (num_candidates == 0) {
            failed_attempts++;
            continue;
        }

        // Picks and adds a random candidate
        uint chosen = candidates[random_in_range(seed, 0, num_candidates - 1)];
        scp_solution_add_column(solution, (int)chosen, instance);
        failed_attempts = 0;
    }

    free(candidates);
    free(removed_cols);
}


int mutate_children(SCPSolution* children, int num_children, SCPInstance* instance, unsigned int* seed) {
    if (num_children <= 0) return -1;

    int mutation_count = (int)(POPULATION_SIZE * MUTATION_RATE);
    if (mutation_count < 1) mutation_count = 1;
    if (mutation_count > num_children) mutation_count = num_children;

    // Replaces RandomGenerator::uniqueRangeSet: the first mutation_count indexes end up randomly selected, without repetition
    int* selected_children_indexes = malloc(num_children * sizeof(int));
    for (int i = 0; i < num_children; i++)
        selected_children_indexes[i] = i;
    for (int i = 0; i < mutation_count; i++) {
        int j = random_in_range(seed, i, num_children - 1);
        int tmp = selected_children_indexes[i];
        selected_children_indexes[i] = selected_children_indexes[j];
        selected_children_indexes[j] = tmp;
    }

    for (int i = 0; i < mutation_count; i++)
        mutation(&children[selected_children_indexes[i]], instance, seed);

    int best_idx = selected_children_indexes[0];
    for (int i = 1; i < mutation_count; i++)
        if (children[selected_children_indexes[i]].cost < children[best_idx].cost)
            best_idx = selected_children_indexes[i];

    free(selected_children_indexes);
    return best_idx;
}