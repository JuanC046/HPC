/**
 * dartboard_threads.c
 * 
 * A program to estimate the value of PI using the Monte Carlo Dartboard method
 * with POSIX threads. Each thread performs a portion of the dart throws and
 * the results are combined to calculate PI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

/**
 * Structure to pass arguments to thread function
 */
typedef struct {
    double circle_radius;
    long throws_per_thread;
    long hits;
    unsigned int seed;
} ThreadData;

/**
 * Generates a random double between 0 and circle_radius
 * Using thread-safe random number generation
 */
double generate_random_coordinate(double circle_radius, unsigned int *seed) {
    return ((double)rand_r(seed) / RAND_MAX) * circle_radius;
}

/**
 * Checks if a point (x,y) is inside a circle with radius circle_radius centered at (0,0)
 * @param x double x coordinate of the dart throw
 * @param y double y coordinate of the dart throw
 * @param circle_radius The radius of the circle being checked against.
 */
int is_inside_circle(double x, double y, double circle_radius) {
    return (x * x + y * y) < (circle_radius * circle_radius);
}

/**
 * Performs a single dart throw and returns 1 if it lands inside the circle, 0 otherwise
 * @param circle_radius double radius of the circle
 * @param seed Pointer to the thread-specific random seed
 */
int throw_dart(double circle_radius, unsigned int *seed) {
    double x = generate_random_coordinate(circle_radius, seed);
    double y = generate_random_coordinate(circle_radius, seed);
    return is_inside_circle(x, y, circle_radius);
}

/**
 * Thread function that performs dart throws and counts hits
 * @param arg Pointer to ThreadData structure
 */
void *thread_throw_darts(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->hits = 0;
    
    for (long i = 0; i < data->throws_per_thread; i++) {
        data->hits += throw_dart(data->circle_radius, &data->seed);
    }
    
    pthread_exit(NULL);
}

/**
 * Estimates PI by throwing n darts using multiple threads and calculating the ratio
 * @param circle_radius double radius of the circle
 * @param n number of throws
 * @param num_threads number of threads to use
 */
double estimate_pi(double circle_radius, long n, int num_threads) {
    // Adjust number of threads if there are more threads than throws
    if (num_threads > n) {
        num_threads = n;
        printf("Warning: Adjusted number of threads to %d (equal to number of throws)\n", num_threads);
    }
    
    // Calculate throws per thread
    long throws_per_thread = n / num_threads;
    long remaining_throws = n % num_threads;
    
    // Create thread array and thread data array
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = (ThreadData *)malloc(num_threads * sizeof(ThreadData));
    
    if (threads == NULL || thread_data == NULL) {
        printf("Error: Memory allocation failed\n");
        exit(1);
    }
    
    // Create and launch threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].circle_radius = circle_radius;
        thread_data[i].throws_per_thread = throws_per_thread;
        
        // Distribute remaining throws among the first 'remaining_throws' threads
        if (i < remaining_throws) {
            thread_data[i].throws_per_thread++;
        }
        
        // Initialize thread-specific random seed
        thread_data[i].seed = time(NULL) ^ (i + 1);
        
        int rc = pthread_create(&threads[i], NULL, thread_throw_darts, &thread_data[i]);
        if (rc) {
            printf("Error: Unable to create thread %d, return code: %d\n", i, rc);
            exit(1);
        }
    }
    
    // Wait for all threads to complete
    long total_hits = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_hits += thread_data[i].hits;
    }
    
    // Calculate PI estimate
    double pi_estimate = 4.0 * total_hits / n;
    
    // Clean up
    free(threads);
    free(thread_data);
    
    return pi_estimate;
}

/**
 * Main function that takes the circle radius, number of dart throws, and number of threads
 * as command line arguments
 */
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <circle_radius> <number_of_throws> <number_of_threads>\n", argv[0]);
        return 1;
    }
    
    double circle_radius = atof(argv[1]);
    long total_throws = atol(argv[2]);
    int num_threads = atoi(argv[3]);
    
    if (total_throws <= 0) {
        printf("Error: Number of throws must be positive\n");
        return 1;
    }
    
    if (circle_radius <= 0) {
        printf("Error: Circle radius must be positive\n");
        return 1;
    }
    
    if (num_threads <= 0) {
        printf("Error: Number of threads must be positive\n");
        return 1;
    }
    
    // Start time measurement
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Estimate PI using threads
    double pi_estimate = estimate_pi(circle_radius, total_throws, num_threads);
    
    // End time measurement
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Print results
    printf("Circle radius: %.2f\n", circle_radius);
    printf("Total throws: %ld\n", total_throws);
    printf("Number of threads: %d\n", num_threads);
    printf("Estimated PI: %.10f\n", pi_estimate);
    printf("Actual PI:    %.10f\n", M_PI);
    printf("Error:        %f%%\n", fabs(100.0 * (pi_estimate - M_PI) / M_PI));
    printf("Difference:   %.10f\n", fabs(pi_estimate - M_PI));
    printf("Time:         %.9f seconds\n", execution_time);
    
    return 0;
}