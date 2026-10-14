#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <stdlib.h>

/*
 * xorshift32 generator. The state ("seed") is passed explicitly to every function,
 * so there is no global state and each thread can own an independent generator.
 */

/* Returns a seed derived from the current time (use as a base seed) */
unsigned int random_seed_from_time(void);

/* Initializes a generator state from a base seed and a stream id (e.g. thread or run index) */
void random_generator_init(unsigned int* seed, unsigned int base_seed, unsigned int stream_id);

/* Advances the state and returns the next raw 32-bit value */
unsigned int random_next(unsigned int* seed);

int random_in_range_int(unsigned int* seed, int min, int max);
double random_in_range_double(unsigned int* seed, double min, double max);

/* Both return a malloc'd array of 'count' distinct values in [min, max] (caller frees), or NULL on error */
int* random_unique_range_set(unsigned int* seed, int min, int max, int count);
int* random_unique_range_shuffle(unsigned int* seed, int min, int max, int count);

#endif
