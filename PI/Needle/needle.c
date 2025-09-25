/**
 * needle.c
 *
 * A program to estimate the value of PI using the Monte Carlo simulation of Buffon's needle problem
 * - Randomly drop needles on a surface with parallel lines
 * - Count how many needles cross a line
 * - Use the ratio to estimate π
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NEEDLE_LENGTH 1.0 // Length of the needle
#define LINE_DISTANCE 1.0 // Distance between parallel lines

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
 * @return 1 if needle crosses a line, 0 otherwise
 */
int drop_needle()
{
    // Random position of the needle's center (distance from nearest line)
    double position = random_double() * LINE_DISTANCE;

    // Random angle of the needle with respect to the lines (in radians)
    double angle = random_double() * M_PI;

    // Calculate the half-length projection on the x-axis
    double half_length_projection = (NEEDLE_LENGTH / 2.0) * sin(angle);

    // Check if needle crosses a line (either at position=0 or position=LINE_DISTANCE)
    return (position - half_length_projection < 0) || (position + half_length_projection > LINE_DISTANCE);
}

/**
 * Runs the Monte Carlo simulation with n needles
 *
 * @param n Number of needles to drop
 * @return Estimated value of π
 */
double estimate_pi(long n)
{
    long crossings = 0;
    for (long i = 0; i < n; i++)
    {
        crossings += drop_needle();
    }

    // For needles of length equal to line distance,
    // π can be estimated as: 2 * (number of tosses) / (number of crossings)
    return (2.0 * n) / crossings;
}


int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s <number_of_throws>\n", argv[0]);
        return 1;
    }

    long n = atol(argv[1]);
    
    if (n <= 0) {
        printf("Error: Number of throws must be positive\n");
        return 1;
    }

    // Seed random number generator
    srand(time(NULL));

    // Run simulation and measure time
    clock_t start = clock();
    double pi_estimate = estimate_pi(n);
    clock_t end = clock();

    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    // Print results
    printf("Estimated π: %f\n", pi_estimate);
    printf("Actual π:    %f\n", M_PI);
    printf("Error:       %f%%\n", fabs(100.0 * (pi_estimate - M_PI) / M_PI));
    printf("Needles:     %ld\n", n);
    printf("Time:        %.2f seconds\n", elapsed);

    return EXIT_SUCCESS;
}

