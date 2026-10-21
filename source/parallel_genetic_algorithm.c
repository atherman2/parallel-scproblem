#include <stdlib.h>
#include <stdio.h>
#include "parallel_genetic_algorithm.h"
#include "constants.h"
#include "population.h"
#include "selection.h"
#include "crossover.h"
#include "mutation.h"
#include "local_search.h"
#include "random_generator.h"
#include "parallel_local_search.h"

// Defines which part [start, end) of the array each thread will handle
void set_interval(int* start, int* end, int* interval_size, int total, int thread_id) {

    if (k_threads > total) {
        // In case there's more threads than items, onlt the first ones will work
        if (thread_id < total) {
            *interval_size = 1;
            *start = thread_id;
        } else {
            *interval_size = 0;
            *start = 0;
        }

    } else {
        int base_size = total / k_threads;
        *start = base_size * thread_id;

        // The last thread handles any remaining items
        if (thread_id == k_threads - 1) {
            *interval_size = base_size + (total % k_threads);
        } else {
            *interval_size = base_size;
        }
    }
    *end = *start + *interval_size;
}

/*
 * Worker thread. The threads are created once and stay alive during the whole
 * algorithm, so instead of doing a single job this function loops: it waits for
 * the main thread to release it, runs the task written in current_task, and
 * reports back through the done barrier. The loop ends when the main thread
 * sets TASK_STOP.
 */
void* thread_genetic_algorithm(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    SCPInstance* instance = data->instance;
    int start_idx, end_idx, interval_size;
    int running = 1;

    while (running) {
        // Wait until the main thread has chosen the task and released the threads
        pthread_barrier_wait(&thread_work_start);

        if (current_task == TASK_STOP) {
            running = 0;
        } else {
            if (current_task == TASK_FILL_POPULATION) {
                // This thread builds its slice of the initial population
                set_interval(&start_idx, &end_idx, &interval_size, POPULATION_SIZE, data->thread_id);
                if (interval_size > 0) {
                    int best_idx = fill_initial_population(&population[start_idx], interval_size, instance, &data->seed);
                    data->best_local_idx = best_idx + start_idx;
                    data->best_local_cost = population[data->best_local_idx].cost;
                } else {
                    data->best_local_idx = -1;   // empty slice, nothing to report
                }
            } else if (current_task == TASK_LOCAL_SEARCH) {
                // This thread improves its slice of ls_solutions and keeps the best of the slice
                set_interval(&start_idx, &end_idx, &interval_size, ls_num, data->thread_id);
                local_search_range(ls_solutions, start_idx, end_idx, instance,
                                   &data->best_local_idx, &data->best_local_cost);
            }

            // Tell the main thread this thread has finished the task
            pthread_barrier_wait(&thread_work_done);
        }
    }
    return NULL;
}

void best_among_threads(SCPSolution* best, SCPInstance* instance) {
    for (int thread_id = 0; thread_id < k_threads; thread_id++) {
        ThreadData* data = &threads_data[thread_id];
        update_best(best, &population[data->best_local_idx], instance);
    }
}

/*
 * Runs the local search on solutions[0..num_solutions-1] using all threads and
 * returns the index of the best solution (lowest cost, lowest index on ties).
 * Main thread only: it releases the workers, waits for them and picks the best.
 */
int apply_local_search_parallel(SCPSolution* solutions, int num_solutions) {
    // Tell the workers what to process and which task to run
    ls_solutions = solutions;
    ls_num = num_solutions;
    current_task = TASK_LOCAL_SEARCH;

    // Release the workers and wait until all of them finish their slices
    pthread_barrier_wait(&thread_work_start);
    pthread_barrier_wait(&thread_work_done);

    // Pick the best among the partial results, in thread order
    int best_idx = -1;
    double best_cost = 0.0;
    for (int thread_id = 0; thread_id < k_threads; thread_id++) {
        ThreadData* data = &threads_data[thread_id];

        if (data->best_local_idx < 0)   // Thread had an empty slice (more threads than solutions)
            continue;

        if (best_idx < 0 || data->best_local_cost < best_cost) {
            best_idx = data->best_local_idx;
            best_cost = data->best_local_cost;
        }
    }
    return best_idx;
}

void parallel_genetic_algorithm(SCPSolution* best, SCPInstance* instance, unsigned int* seed, double* convergence_history) {
    population = malloc(POPULATION_SIZE * sizeof(SCPSolution));
    children = malloc(PARENT_PAIRS_COUNT * sizeof(SCPSolution));
    int parents[2 * PARENT_PAIRS_COUNT];
    int history_size = 0;
    int best_idx;

    iterations_finished = 0;

    int num_children = PARENT_PAIRS_COUNT;
    mutation_count = (int)(POPULATION_SIZE * MUTATION_RATE);
    if (mutation_count < 1) mutation_count = 1;
    if (mutation_count > num_children) mutation_count = num_children;
    selected_children_indexes = random_unique_range_set(seed, 0, num_children - 1, mutation_count);

    for (int i = 0; i < PARENT_PAIRS_COUNT; i++)
        scp_solution_empty(&children[i], instance);

    k_threads = -1;
    while((k_threads < 1) || k_threads > POPULATION_SIZE) {
        printf("Thread count must be greater than 0 and lesser than %d\n", POPULATION_SIZE);
        printf("Insert the thread count: ");
        scanf("%d", &k_threads);
    }
    thread_barrier_init(&thread_work_start, NULL, k_threads + 1);
    thread_barrier_init(&thread_work_done, NULL, k_threads + 1);
    threads_data = (ThreadData*) malloc(sizeof(ThreadData) * k_threads);
    thread_ret = (int*) malloc(sizeof(int) * k_threads);
    for (int thread_id = 0; thread_id < k_threads; thread_id++) {
        ThreadData* data = &threads_data[thread_id];
        data->thread_id = thread_id;
        data->instance = instance;
        data->best_local_idx = -1;
        data->best_local_cost = -1.0;
        random_generator_init(&data->seed, base_seed, thread_id);
        thread_ret[thread_id] = pthread_create(threads + thread_id, NULL, thread_genetic_algorithm, (void*) data);
    }
    // Signalize threads can start working
    pthread_barrier_wait(&thread_work_start);
    // Wait threads finish filling the population
    pthread_barrier_wait(&thread_work_done);
    best_among_threads(best, instance);

#ifndef NO_LOCAL_SEARCH
    best_idx = apply_local_search_parallel(population, POPULATION_SIZE);
#else
    best_idx = best_of_population(population, POPULATION_SIZE);
#endif
    update_best(best, &population[best_idx], instance);
    if (convergence_history)
        convergence_history[history_size++] = best->cost;

    for (int generation = 1; generation <= MAX_GENERATION; generation++) {
        select_parents(parents, 2 * PARENT_PAIRS_COUNT, POPULATION_SIZE, population, seed);

        best_idx = make_children(children, PARENT_PAIRS_COUNT, parents, population, instance);
        update_best(best, &children[best_idx], instance);

        best_idx = mutate_children(children, PARENT_PAIRS_COUNT, instance, seed);
        update_best(best, &children[best_idx], instance);

#ifndef NO_LOCAL_SEARCH
        best_idx = apply_local_search(children, PARENT_PAIRS_COUNT, instance);
#else
        best_idx = best_of_population(children, PARENT_PAIRS_COUNT);
#endif
        update_best(best, &children[best_idx], instance);

        elitism(population, POPULATION_SIZE, children, PARENT_PAIRS_COUNT);

        if (convergence_history)
            convergence_history[history_size++] = best->cost;
    }

    for (int thread_id = 0; thread_id < k_threads; thread_id++)
        pthread_join(threads[t], NULL);
    free(threads);
    free(threads_data);
    free(thread_ret);
    pthread_barrier_destroy(&thread_work_start);
    pthread_barrier_destroy(&thread_work_done);

    for (int i = 0; i < POPULATION_SIZE; i++)
        scp_solution_free(&population[i]);
    for (int i = 0; i < PARENT_PAIRS_COUNT; i++)
        scp_solution_free(&children[i]);
    free(population);
    free(children);
    free(selected_children_indexes);
}
