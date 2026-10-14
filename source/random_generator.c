#include "random_generator.h"
#include <stdlib.h>
#include <time.h>

unsigned int random_seed_from_time(void) {
    return (unsigned int)time(NULL) ^ ((unsigned int)clock() * 2654435761u);
}

void random_generator_init(unsigned int* seed, unsigned int base_seed, unsigned int stream_id) {
    // Mixes base seed and stream id so that nearby streams get unrelated states
    unsigned int x = base_seed + stream_id * 0x9E3779B9u;
    x ^= x >> 16;
    x *= 0x7FEB352Du;
    x ^= x >> 15;
    x *= 0x846CA68Bu;
    x ^= x >> 16;
    *seed = (x == 0) ? 2463534242u : x;  // xorshift state must never be 0
}

unsigned int random_next(unsigned int* seed) {
    unsigned int x = (*seed == 0) ? 2463534242u : *seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *seed = x;
    return x;
}

int random_in_range_int(unsigned int* seed, int min, int max) {
    if (min >= max) return min;
    return min + (int)(random_next(seed) % ((unsigned int)(max - min) + 1u));
}

double random_in_range_double(unsigned int* seed, double min, double max) {
    if (min >= max) return min;
    double scale = (random_next(seed) >> 8) / 16777216.0;  // [0, 1)
    return min + scale * (max - min);
}

int* random_unique_range_set(unsigned int* seed, int min, int max, int count) {
    if (count > (max - min + 1) || count < 0) {
        return NULL;
    }

    int* result = (int*)malloc((count > 0 ? count : 1) * sizeof(int));
    if (!result) return NULL;

    int current_count = 0;
    while (current_count < count) {
        int num = random_in_range_int(seed, min, max);
        int exists = 0;

        // simple linear search
        for (int i = 0; i < current_count; i++) {
            if (result[i] == num) {
                exists = 1;
                break;
            }
        }

        if (!exists) {
            result[current_count++] = num;
        }
    }

    return result;
}

int* random_unique_range_shuffle(unsigned int* seed, int min, int max, int count) {
    if (count > (max - min + 1) || count < 0) {
        return NULL;
    }

    int range_size = max - min + 1;

    int* pool = (int*)malloc(range_size * sizeof(int));
    if (!pool) return NULL;

    for (int i = 0; i < range_size; i++) {
        pool[i] = min + i;
    }

    // Fisher-Yates shuffle
    for (int i = range_size - 1; i > 0; i--) {
        int j = random_in_range_int(seed, 0, i);
        int temp = pool[i];
        pool[i] = pool[j];
        pool[j] = temp;
    }

    int* result = (int*)malloc((count > 0 ? count : 1) * sizeof(int));
    if (!result) {
        free(pool);
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        result[i] = pool[i];
    }

    free(pool);
    return result;
}
