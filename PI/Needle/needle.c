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
 * Runs the Monte Carlo simulation with n needles
 *
 * @param line_distance distance between the parallel lines
 * @param needle_length Length of the needle
 * @param n Number of needles to drop
 * @return Estimated value of π
 */
double estimate_pi(double line_distance, double needle_length, long n)
{
    long crossings = 0;
    for (long i = 0; i < n; i++)
    {
        crossings += drop_needle(line_distance, needle_length);
    }

    // For needles of length equal to line distance,
    // General formula for any needle length (when needle_length ≤ line_distance)
    return (2.0 * needle_length * n) / (crossings * line_distance);
}

int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc != 4)
    {
        printf("Usage: %s <line_distance> <needle_length> <number_of_throws>\n", argv[0]);
        return 1;
    }

    long line_distance = atol(argv[1]);
    long needle_length = atol(argv[2]);
    long n = atol(argv[3]);

    if (n <= 0)
    {
        printf("Error: Number of throws must be positive\n");
        return 1;
    }
    if (needle_length > line_distance){
        printf("Error: Needle length must not exceed line distance\n");
        return 1;
    }

    // Seed random number generator
    srand(time(NULL));

    // Run simulation and measure time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    double pi_estimate = estimate_pi(line_distance, needle_length, n);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Print results
    printf("Estimated π: %f\n", pi_estimate);
    printf("Actual π:    %f\n", M_PI);
    printf("Error:       %f%%\n", fabs(100.0 * (pi_estimate - M_PI) / M_PI));
    printf("Needles:     %ld\n", n);
    printf("Time:        %.9f seconds\n", execution_time);

    return EXIT_SUCCESS;
}
