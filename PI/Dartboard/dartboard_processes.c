/**
 * dartboard_processes.c
 * 
 * A program to estimate the value of PI using the Monte Carlo Dartboard method
 * with POSIX processes. Each process performs a portion of the dart throws and
 * the results are combined to calculate PI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/**
 * Structure to pass arguments to child processes via shared memory
 */
typedef struct {
    double circle_radius;
    long throws_per_process;
    long hits;
    unsigned int seed;
} ProcessData;

/**
 * Generates a random double between 0 and circle_radius
 * Using process-safe random number generation
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
 * @param seed Pointer to the process-specific random seed
 */
int throw_dart(double circle_radius, unsigned int *seed) {
    double x = generate_random_coordinate(circle_radius, seed);
    double y = generate_random_coordinate(circle_radius, seed);
    return is_inside_circle(x, y, circle_radius);
}

/**
 * Process function that performs dart throws and counts hits
 * @param data Pointer to ProcessData structure in shared memory
 */
void process_throw_darts(ProcessData *data) {
    data->hits = 0;
    
    for (long i = 0; i < data->throws_per_process; i++) {
        data->hits += throw_dart(data->circle_radius, &data->seed);
    }
}

/**
 * Estimates PI by throwing n darts using multiple processes and calculating the ratio
 * @param circle_radius double radius of the circle
 * @param n number of throws
 * @param num_processes number of processes to use
 */
double estimate_pi(double circle_radius, long n, int num_processes) {
    // Adjust number of processes if there are more processes than throws
    if (num_processes > n) {
        num_processes = n;
        printf("Warning: Adjusted number of processes to %d (equal to number of throws)\n", num_processes);
    }
    
    // Calculate throws per process
    long throws_per_process = n / num_processes;
    long remaining_throws = n % num_processes;
    
    // Create shared memory for process data
    const char *shm_name = "/dartboard_shm";
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    // Set the size of the shared memory object
    size_t shm_size = num_processes * sizeof(ProcessData);
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        exit(1);
    }
    
    // Map the shared memory object into the address space
    ProcessData *process_data = (ProcessData *)mmap(NULL, shm_size, 
                                                   PROT_READ | PROT_WRITE, 
                                                   MAP_SHARED, shm_fd, 0);
    
    if (process_data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    
    // Initialize process data
    for (int i = 0; i < num_processes; i++) {
        process_data[i].circle_radius = circle_radius;
        process_data[i].throws_per_process = throws_per_process;
        
        // Distribute remaining throws among the first 'remaining_throws' processes
        if (i < remaining_throws) {
            process_data[i].throws_per_process++;
        }
        
        // Initialize process-specific random seed
        process_data[i].seed = time(NULL) ^ (i + 1);
        process_data[i].hits = 0;
    }
    
    // Create child processes
    pid_t *pids = (pid_t *)malloc(num_processes * sizeof(pid_t));
    if (pids == NULL) {
        perror("malloc");
        exit(1);
    }
    
    for (int i = 0; i < num_processes; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            // Fork failed
            perror("fork");
            exit(1);
        } else if (pids[i] == 0) {
            // Child process
            process_throw_darts(&process_data[i]);
            
            // Child process exits after completing its work
            munmap(process_data, shm_size);
            close(shm_fd);
            exit(0);
        }
    }
    
    // Parent process waits for all child processes to complete
    for (int i = 0; i < num_processes; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            printf("Warning: Child process %d did not exit normally\n", i);
        }
    }
    
    // Calculate total hits
    long total_hits = 0;
    for (int i = 0; i < num_processes; i++) {
        total_hits += process_data[i].hits;
    }
    
    // Calculate PI estimate
    double pi_estimate = 4.0 * total_hits / n;
    
    // Clean up
    munmap(process_data, shm_size);
    close(shm_fd);
    shm_unlink(shm_name);
    free(pids);
    
    return pi_estimate;
}

/**
 * Main function that takes the circle radius, number of dart throws, and number of processes
 * as command line arguments
 */
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <circle_radius> <number_of_throws> <number_of_processes>\n", argv[0]);
        return 1;
    }
    
    double circle_radius = atof(argv[1]);
    long total_throws = atol(argv[2]);
    int num_processes = atoi(argv[3]);
    
    if (total_throws <= 0) {
        printf("Error: Number of throws must be positive\n");
        return 1;
    }
    
    if (circle_radius <= 0) {
        printf("Error: Circle radius must be positive\n");
        return 1;
    }
    
    if (num_processes <= 0) {
        printf("Error: Number of processes must be positive\n");
        return 1;
    }
    
    // Start time measurement
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Estimate PI using processes
    double pi_estimate = estimate_pi(circle_radius, total_throws, num_processes);
    
    // End time measurement
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Print results
    printf("Circle radius: %.2f\n", circle_radius);
    printf("Total throws: %ld\n", total_throws);
    printf("Number of processes: %d\n", num_processes);
    printf("Estimated PI: %.10f\n", pi_estimate);
    printf("Actual PI:    %.10f\n", M_PI);
    printf("Error:        %f%%\n", fabs(100.0 * (pi_estimate - M_PI) / M_PI));
    printf("Difference:   %.10f\n", fabs(pi_estimate - M_PI));
    printf("Time:         %.9f seconds\n", execution_time);
    
    return 0;
}