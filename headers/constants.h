#ifndef CONSTANTS_H
#define CONSTANTS_H

/* Genetic algorithm parameters */

#define POPULATION_SIZE     100   /* individuals in the population */
#define COL_RCL_SIZE        10    /* restricted candidate list size (semi-greedy construction) */
#define PARENT_PAIRS_COUNT  10    /* each pair of parents produces one child */
#define TOURNAMENT_SIZE     3     /* participants in each parent selection tournament */
#define MAX_GENERATION      100
#define MUTATION_K          2     /* columns removed by each mutation */
#define MUTATION_RATE       0.05  /* fraction of POPULATION_SIZE mutated per generation */
#define ELITISM_RATE        0.1   /* fraction of the population preserved by elitism */

#endif /* CONSTANTS_H */
