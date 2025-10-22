/**
 * dartbord_omp_fixed.c
 *
 * A program to estimate the value of PI using the Monte Carlo Dartboard method.
 * We simulate throwing darts at a square containing a circle and use
 * the ratio of darts landing inside the circle to estimate PI.
 *
 * Fixed OpenMP implementation to avoid race conditions and improve thread safety.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

/**
 * Generates a random double between 0 and circle_radius
 * Using thread-safe random number generation
 */
double generate_random_coordinate(double circle_radius, unsigned int *seed)
{
    return ((double)rand_r(seed) / RAND_MAX) * circle_radius;
}

/**
 * Checks if a point (x,y) is inside a circle with radius circle_radius centered at (0,0)
 * @param x double x coordinate of the dart throw
 * @param y double y coordinate of the dart throw
 * @param circle_radius The radius of the circle being checked against.
 */
int is_inside_circle(double x, double y, double circle_radius)
{
    return (x * x + y * y) < (circle_radius * circle_radius);
}

/**
 * Performs a single dart throw and returns 1 if it lands inside the circle, 0 otherwise
 * @param circle_radius double radius of the circle
 * @param seed Pointer to thread-local random seed
 */
int throw_dart(double circle_radius, unsigned int *seed)
{
    double x = generate_random_coordinate(circle_radius, seed);
    double y = generate_random_coordinate(circle_radius, seed);
    return is_inside_circle(x, y, circle_radius);
}

/**
 * Estimates PI by throwing n darts and calculating the ratio
 * @param circle_radius double radius of the circle
 * @param n number of throws
 */
double estimate_pi(double circle_radius, long n)
{
    long hits = 0;

#pragma omp parallel
    {
        // Thread-local counter for hits
        long local_hits = 0;

        // Thread-local random seed based on thread ID
        unsigned int seed = time(NULL) ^ omp_get_thread_num();

// Distribute work among threads
#pragma omp for schedule(static)
        for (long i = 0; i < n; i++)
        {
            local_hits += throw_dart(circle_radius, &seed);
        }

// Safely combine results from all threads
#pragma omp atomic
        hits += local_hits;
    }

    // The ratio of hits to total throws, multiplied by 4, approximates PI
    return 4.0 * hits / n;
}

/**
 * Main function that takes the number of dart throws and circle radius as command line arguments
 */
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s <circle_radius> <number_of_throws> <num_threads>\n", argv[0]);
        return 1;
    }

    double circle_radius = atof(argv[1]);
    long n = atol(argv[2]);
    int num_threads = atoi(argv[3]);

    if (n <= 0)
    {
        printf("Error: Number of throws must be positive\n");
        return 1;
    }

    if (circle_radius <= 0)
    {
        printf("Error: Circle radius must be positive\n");
        return 1;
    }

    if (num_threads < 1)
    {
        printf("Error: Number of threads must be at least 1\n");
        return 1;
    }

    // Set the number of threads for OpenMP
    omp_set_num_threads(num_threads);

    // Start time measurement
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Estimate PI
    double pi_estimate = estimate_pi(circle_radius, n);

    // End time measurement
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Circle radius: %.2f\n", circle_radius);
    printf("Number of threads: %d\n", num_threads);
    printf("Number of throws: %ld\n", n);
    printf("Estimated PI: %.10f\n", pi_estimate);
    printf("Actual PI:    %.10f\n", M_PI);
    printf("Error:        %f%%\n", fabs(100.0 * (pi_estimate - M_PI) / M_PI));
    printf("Difference:   %.10f\n", fabs(pi_estimate - M_PI));
    printf("Time:         %.9f seconds\n", execution_time);

    return 0;
}