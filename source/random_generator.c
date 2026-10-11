#include "random_generator.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

void random_generator_init(void) {
    srand((unsigned int)time(NULL));
}

int random_in_range_int(int min, int max) {
    if (min > max) return min;
    return min + rand() % ((max - min) + 1);
}

double random_in_range_double(double min, double max) {
    if (min >= max) return min;
    double scale = rand() / (double)RAND_MAX;
    return min + scale * (max - min);
}

int* random_unique_range_set(int min, int max, int count) {
    if (count > (max - min + 1) || count < 0) {
        return NULL;
    }

    int* result = (int*)malloc(count * sizeof(int));
    if (!result) return NULL;

    int current_count = 0;
    while (current_count < count) {
        int num = random_in_range_int(min, max);
        bool exists = false;
        
        // Busca linear simples
        for (int i = 0; i < current_count; i++) {
            if (result[i] == num) {
                exists = true;
                break;
            }
        }
        
        if (!exists) {
            result[current_count++] = num;
        }
    }

    return result;
}

int* random_unique_range_shuffle(int min, int max, int count) {
    if (count > (max - min + 1) || count < 0) {
        return NULL;
    }

    int range_size = max - min + 1;
    
    int* pool = (int*)malloc(range_size * sizeof(int));
    if (!pool) return NULL;

    for (int i = 0; i < range_size; i++) {
        pool[i] = min + i;
    }

    // Eq ao std::shuffle (Algoritmo Fisher-Yates)
    for (int i = range_size - 1; i > 0; i--) {
        int j = random_in_range_int(0, i);
        int temp = pool[i];
        pool[i] = pool[j];
        pool[j] = temp;
    }

    int* result = (int*)malloc(count * sizeof(int));
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