#include <stdlib.h>
#include "crossover.h"

typedef struct {
    double ratio;  // cost / rows covered
    uint column;
} RatioEntry;

// Descending order: worst (highest) ratio first
static int compare_ratio_desc(const void* a, const void* b) {
    double x = ((const RatioEntry*)a)->ratio;
    double y = ((const RatioEntry*)b)->ratio;
    return (x < y) - (x > y);
}

/**
 * Fusion crossover
 * Merges both parents, sorts columns by their cost/benefit ratio (worst first), and drops redundant columns to minimize the final solution cost
 */
void crossover(SCPSolution* child, const SCPSolution* parent_a, const SCPSolution* parent_b, SCPInstance* instance) {
    scp_solution_reset(child, instance);

    // Combine all columns from parent A and B into the child
    for (uint i = 0; i < parent_a->num_columns_used; i++)
        scp_solution_add_column(child, (int)parent_a->columns_used[i], instance);

    for (uint i = 0; i < parent_b->num_columns_used; i++)
        if (!child->column_used[parent_b->columns_used[i]])
            scp_solution_add_column(child, (int)parent_b->columns_used[i], instance);

    // Sort columns by cost-efficiency ratio (cost / total rows covered), descending, to target and remove the worst columns first
    uint num_entries = child->num_columns_used;
    RatioEntry* entries = malloc((num_entries ? num_entries : 1) * sizeof(RatioEntry));
    for (uint i = 0; i < num_entries; i++) {
        const SCPColumn* column = &instance->columns[child->columns_used[i]];
        entries[i].column = child->columns_used[i];
        entries[i].ratio = column->cost / (double)column->num_covered_rows;
    }
    qsort(entries, num_entries, sizeof(RatioEntry), compare_ratio_desc);

    for (uint i = 0; i < num_entries; i++) {
        int col = (int)entries[i].column;
        scp_solution_remove_column(child, col, instance);

        // If removing the column leaves any row uncovered, restore it
        if (child->num_uncovered_rows > 0)
            scp_solution_add_column(child, col, instance);
    }

    free(entries);
}

int make_children(SCPSolution* children, int num_children, const int* parents,
                  const SCPSolution* population, SCPInstance* instance) {
    int best_idx = 0;
    for (int i = 0; i < num_children; i++) {
        crossover(&children[i], &population[parents[2 * i]], &population[parents[2 * i + 1]], instance);
        if (children[i].cost < children[best_idx].cost)
            best_idx = i;
    }
    return best_idx;
}
