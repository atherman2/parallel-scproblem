#include <stdlib.h>
#include "selection.h"
#include "constants.h"
#include "random_generator.h"

void select_parents(int* parents, int num_parents, int population_size,
                    const SCPSolution* population, unsigned int* seed) {
    int count = 0;
    while (count < num_parents) {
        int* participants = random_unique_range_set(seed, 0, population_size - 1, TOURNAMENT_SIZE);
        int best = participants[0];
        for (int i = 1; i < TOURNAMENT_SIZE; i++)
            if (population[participants[i]].cost < population[best].cost)
                best = participants[i];
        free(participants);

        // Avoids selecting the same individual twice
        int already_selected = 0;
        for (int i = 0; i < count; i++) {
            if (parents[i] == best) {
                already_selected = 1;
                break;
            }
        }
        if (!already_selected)
            parents[count++] = best;
    }
}

void elitism(SCPSolution* population, int population_size, SCPSolution* children, int num_children) {
    int elite_size = (int)(population_size * ELITISM_RATE);
    int* elite_indexes = malloc((elite_size > 0 ? elite_size : 1) * sizeof(int));
    bool_uchar* is_elite = calloc(population_size, sizeof(bool_uchar));

    // Finds the elite_size cheapest individuals, replacing the worst elite whenever a better one shows up
    for (int i = 0; i < elite_size; i++)
        elite_indexes[i] = i;
    for (int i = elite_size; i < population_size && elite_size > 0; i++) {
        int worst_pos = 0;
        for (int e = 1; e < elite_size; e++)
            if (population[elite_indexes[e]].cost > population[elite_indexes[worst_pos]].cost)
                worst_pos = e;
        if (population[i].cost < population[elite_indexes[worst_pos]].cost)
            elite_indexes[worst_pos] = i;
    }
    for (int e = 0; e < elite_size; e++)
        is_elite[elite_indexes[e]] = 1;

    // Non-elite slots receive the children
    int next_child = 0;
    for (int j = 0; j < population_size && next_child < num_children; j++) {
        if (is_elite[j]) continue;
        SCPSolution tmp = population[j];
        population[j] = children[next_child];
        children[next_child] = tmp;
        next_child++;
    }

    free(elite_indexes);
    free(is_elite);
}
