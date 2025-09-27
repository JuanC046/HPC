/**
 * needle_processes.c
 *
 * A program to estimate the value of PI using the Monte Carlo simulation of Buffon's needle problem
 * - Randomly drop needles on a surface with parallel lines
 * - Count how many needles cross a line
 * - Use the ratio to estimate π
 * - Uses multiple processes to parallelize the computation
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

// Structure to pass arguments to processes via shared memory
typedef struct {
    double line_distance;
    double needle_length;
    long needles_per_process;
    long* crossings;  // Pointer to shared memory for storing crossings
} ProcessData;
/**
 * Simulates dropping needles in a child process
 *
 * @param data Process data containing simulation parameters
 * @param process_id ID of the current process
 */
void drop_needles_process(ProcessData* data, int process_id)
{
    long local_crossings = 0;

    // Each process needs its own random seed
    unsigned int seed = (unsigned int)(getpid() + time(NULL));
    for (long i = 0; i < data->needles_per_process; i++)
    {
        // Generate random position and angle
        double position = (double)rand_r(&seed) / RAND_MAX * data->line_distance;
        double angle = (double)rand_r(&seed) / RAND_MAX * M_PI;

        double half_length_projection = (data->needle_length / 2.0) * sin(angle);

        if ((position - half_length_projection < 0) || (position + half_length_projection > data->line_distance))
        {
            local_crossings++;
        }
    }

    // Store the result in the shared memory at the process's index
    data->crossings[process_id] = local_crossings;

    // Child process exits after completing its work
    exit(0);
}

/**
 * Runs the Monte Carlo simulation with n needles using multiple processes
 *
 * @param line_distance distance between the parallel lines
 * @param needle_length Length of the needle
 * @param n Number of needles to drop
 * @param num_processes Number of processes to use
 * @return Estimated value of π
 */
double estimate_pi(double line_distance, double needle_length, long n, int num_processes)
{
    // Create shared memory for crossings array
    long* crossings = mmap(NULL, num_processes * sizeof(long),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (crossings == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    // Initialize shared memory
    for (int i = 0; i < num_processes; i++) {
        crossings[i] = 0;
    }
    // Calculate needles per process
    long needles_per_process = n / num_processes;
    long remaining_needles = n % num_processes;

    // Process data structure
    ProcessData data;
    data.line_distance = line_distance;
    data.needle_length = needle_length;
    data.crossings = crossings;

    // Create child processes
    pid_t pids[num_processes];

    for (int i = 0; i < num_processes; i++) {
        // Adjust needles per process to distribute remaining needles
        data.needles_per_process = needles_per_process;
        if (i < remaining_needles) {
            data.needles_per_process++;
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            // Fork failed
            perror("Fork failed");
            exit(1);
        } else if (pids[i] == 0) {
            // Child process
            drop_needles_process(&data, i);
            // Child never reaches here due to exit() in drop_needles_process
        }
    }

    // Parent process waits for all children to complete
    for (int i = 0; i < num_processes; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Calculate total crossings
    long total_crossings = 0;
    for (int i = 0; i < num_processes; i++) {
        total_crossings += crossings[i];
    }

    // Clean up shared memory
    munmap(crossings, num_processes * sizeof(long));
    // Calculate pi estimate
    return (2.0 * needle_length * n) / (total_crossings * line_distance);
}

int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc != 5)
    {
        printf("Usage: %s <line_distance> <needle_length> <number_of_throws> <number_of_processes>\n", argv[0]);
        return 1;
    }

    double line_distance = atof(argv[1]);
    double needle_length = atof(argv[2]);
    long n = atol(argv[3]);
    int num_processes = atoi(argv[4]);

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
    if (num_processes <= 0)
    {
        printf("Error: Number of processes must be positive\n");
        return 1;
    }

    // Seed random number generator
    srand(time(NULL));

    // Run simulation and measure time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    double pi_estimate = estimate_pi(line_distance, needle_length, n, num_processes);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Print results
    printf("Estimated π: %f\n", pi_estimate);
    printf("Actual π:    %f\n", M_PI);
    printf("Error:       %f%%\n", fabs(100.0 * (pi_estimate - M_PI) / M_PI));
    printf("Difference:  %.10f\n", fabs(pi_estimate - M_PI));
    printf("Needles:     %ld\n", n);
    printf("Processes:   %d\n", num_processes);
    printf("Time:        %.9f seconds\n", execution_time);

    return EXIT_SUCCESS;
}

