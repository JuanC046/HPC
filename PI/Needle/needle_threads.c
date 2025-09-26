/**
 * needle_threads.c
 *
 * A program to estimate the value of PI using the Monte Carlo simulation of Buffon's needle problem
 * - Randomly drop needles on a surface with parallel lines
 * - Count how many needles cross a line
 * - Use the ratio to estimate π
 * - Uses multiple threads to parallelize the computation
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

// Structure to pass arguments to threads
typedef struct {
    double line_distance;
    double needle_length;
    long needles_per_thread;
    long crossings;  // Each thread will store its count here
} ThreadData;

/**
 * Generates a random double between 0 and 1
 */
double random_double()
{
    return (double)rand() / RAND_MAX;
}

/**
 * Simulates dropping a single needle and checks if it crosses a line
 *
 * @param line_distance distance between the parallel lines
 * @param needle_length Length of the needle
 * @return 1 if needle crosses a line, 0 otherwise
 */
int drop_needle(double line_distance, double needle_length)
{
    // Random position of the needle's center (distance from nearest line)
    double position = random_double() * line_distance;

    // Random angle of the needle with respect to the lines (in radians)
    double angle = random_double() * M_PI;

    // Calculate the half-length projection on the x-axis
    double half_length_projection = (needle_length / 2.0) * sin(angle);

    // Check if needle crosses a line (either at position=0 or position=line_distance)
    return (position - half_length_projection < 0) || (position + half_length_projection > line_distance);
}

/**
 * Thread function that performs a portion of the needle drops
 */
void* thread_drop_needles(void* arg)
{
    ThreadData* data = (ThreadData*)arg;
    long local_crossings = 0;

    // Each thread needs its own random seed
    unsigned int seed = (unsigned int)pthread_self();
    for (long i = 0; i < data->needles_per_thread; i++)
    {
        // Use thread-local random function to avoid race conditions
        double position = (double)rand_r(&seed) / RAND_MAX * data->line_distance;
        double angle = (double)rand_r(&seed) / RAND_MAX * M_PI;

        double half_length_projection = (data->needle_length / 2.0) * sin(angle);

        if ((position - half_length_projection < 0) || (position + half_length_projection > data->line_distance))
        {
            local_crossings++;
        }
    }

    // Store the result in the thread's data structure
    data->crossings = local_crossings;

    return NULL;
}

/**
 * Runs the Monte Carlo simulation with n needles using multiple threads
 *
 * @param line_distance distance between the parallel lines
 * @param needle_length Length of the needle
 * @param n Number of needles to drop
 * @param num_threads Number of threads to use
 * @return Estimated value of π
 */
double estimate_pi(double line_distance, double needle_length, long n, int num_threads)
{
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData* thread_data = malloc(num_threads * sizeof(ThreadData));

    // Calculate needles per thread
    long needles_per_thread = n / num_threads;
    long remaining_needles = n % num_threads;

    // Create and start threads
    for (int i = 0; i < num_threads; i++)
    {
        thread_data[i].line_distance = line_distance;
        thread_data[i].needle_length = needle_length;
        thread_data[i].needles_per_thread = needles_per_thread;

        // Distribute any remaining needles to the first few threads
        if (i < remaining_needles)
        {
            thread_data[i].needles_per_thread++;
        }

        thread_data[i].crossings = 0;

        pthread_create(&threads[i], NULL, thread_drop_needles, &thread_data[i]);
    }

    // Wait for all threads to complete
    long total_crossings = 0;
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
        total_crossings += thread_data[i].crossings;
    }

    // Clean up
    free(threads);
    free(thread_data);

    // Calculate pi estimate
    return (2.0 * needle_length * n) / (total_crossings * line_distance);
}

int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc != 5)
    {
        printf("Usage: %s <line_distance> <needle_length> <number_of_throws> <number_of_threads>\n", argv[0]);
        return 1;
    }

    double line_distance = atof(argv[1]);
    double needle_length = atof(argv[2]);
    long n = atol(argv[3]);
    int num_threads = atoi(argv[4]);

    if (n <= 0)
    {
        printf("Error: Number of throws must be positive\n");
        return 1;
    }
    if (needle_length > line_distance)
    {
        printf("Error: Needle length must not exceed line distance\n");
        return 1;
    }
    if (num_threads <= 0)
    {
        printf("Error: Number of threads must be positive\n");
        return 1;
    }

    // Seed random number generator
    srand(time(NULL));

    // Run simulation and measure time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    double pi_estimate = estimate_pi(line_distance, needle_length, n, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Print results
    printf("Estimated π: %f\n", pi_estimate);
    printf("Actual π:    %f\n", M_PI);
    printf("Error:       %f%%\n", fabs(100.0 * (pi_estimate - M_PI) / M_PI));
    printf("Needles:     %ld\n", n);
    printf("Threads:     %d\n", num_threads);
    printf("Time:        %.9f seconds\n", execution_time);

    return EXIT_SUCCESS;
}

