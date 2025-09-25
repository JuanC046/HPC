/**
 * dartbord.c
 * 
 * A program to estimate the value of PI using the Monte Carlo Dartboard method.
 * We simulate throwing darts at a square containing a circle and use
 * the ratio of darts landing inside the circle to estimate PI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/**
 * Generates a random double between 0 and 1
 */
double generate_random_coordinate() {
    return (double)rand() / RAND_MAX;
}

/**
 * Checks if a point (x,y) is inside a unit circle centered at (0,0)
 */
int is_inside_circle(double x, double y) {
    return (x * x + y * y) < 1.0;
}

/**
 * Performs a single dart throw and returns 1 if it lands inside the circle, 0 otherwise
 */
int throw_dart() {
    double x = generate_random_coordinate(); // Scale to [0, 1]
    double y = generate_random_coordinate(); // Scale to [0, 1]
    return is_inside_circle(x, y);
}

/**
 * Estimates PI by throwing n darts and calculating the ratio
 */
double estimate_pi(long n) {
    long hits = 0;
    
    for (long i = 0; i < n; i++) {
        hits += throw_dart();
    }
    
    // The ratio of hits to total throws, multiplied by 4, approximates PI
    return 4.0 * hits / n;
}

/**
 * Main function that takes the number of dart throws as a command line argument
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_throws>\n", argv[0]);
        return 1;
    }
    
    long n = atol(argv[1]);
    
    if (n <= 0) {
        printf("Error: Number of throws must be positive\n");
        return 1;
    }
    
    // Seed the random number generator
    srand(time(NULL));
    
    // Start time measurement
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Estimate PI
    double pi_estimate = estimate_pi(n);

    // End time measurement
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Estimated PI: %.10f\n", pi_estimate);
    printf("Actual PI:    %.10f\n", M_PI);
    printf("Difference:   %.10f\n", fabs(pi_estimate - M_PI));
    printf("Time:         %.9f seconds\n", execution_time);
    
    return 0;
}
