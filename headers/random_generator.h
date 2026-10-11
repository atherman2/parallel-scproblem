#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <stdlib.h>

void random_generator_init(void);

int random_in_range_int(int min, int max);
double random_in_range_double(double min, double max);

int* random_unique_range_set(int min, int max, int count);
int* random_unique_range_shuffle(int min, int max, int count);

#endif