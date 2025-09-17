#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// Function to allocate memory for a square matrix using shared memory
int32_t** allocate_shared_matrix(int n, char* shm_name, int* shm_fd, size_t* shm_size) {
    // Calculate the total size needed for the matrix
    *shm_size = n * sizeof(int32_t*) + n * n * sizeof(int32_t);
    
    // Create a shared memory object
    *shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (*shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }
    
    // Set the size of the shared memory object
    if (ftruncate(*shm_fd, *shm_size) == -1) {
        perror("ftruncate");
        return NULL;
    }
    
    // Map the shared memory object into this process's memory space
    void* ptr = mmap(NULL, *shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, *shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    
    // Set up the matrix structure in shared memory
    int32_t** matrix = (int32_t**)ptr;
    int32_t* data = (int32_t*)((char*)ptr + n * sizeof(int32_t*));
    
    // Set up the row pointers
    for (int i = 0; i < n; i++) {
        matrix[i] = data + i * n;
    }
    
    return matrix;
}

// Function to free shared memory allocated for a matrix
void free_shared_matrix(int32_t** matrix, char* shm_name, int shm_fd, size_t shm_size) {
    // Unmap the shared memory
    if (munmap(matrix, shm_size) == -1) {
        perror("munmap");
    }
    
    // Close the shared memory file descriptor
    if (close(shm_fd) == -1) {
        perror("close");
    }
    
    // Unlink (remove) the shared memory object
    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
    }
}

// Function to generate random values for matrix
void generate_matrix(int32_t** matrix, int n, unsigned int seed_offset) {
    srand(time(NULL) + seed_offset);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            // Generate random int32 values between 0 and 2000
            matrix[i][j] = (rand() % 2000);
        }
    }
}

// Process function for matrix multiplication
// Each process computes a portion of the result matrix
void process_matrix_multiply(int32_t** A, int32_t** B, int32_t** C, int n, int start_row, int end_row) {
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Parallel matrix multiplication function using N processes
void matrix_multiply(int32_t** A, int32_t** B, int32_t** C, int n, int num_processes) {
    pid_t pids[num_processes];
    
    // Calculate rows per process (at least 1)
    int rows_per_process = n / num_processes;
    if (rows_per_process == 0) {
        rows_per_process = 1;
        num_processes = n; // Adjust number of processes if n is small
    }
    
    // Create child processes
    for (int p = 0; p < num_processes; p++) {
        pids[p] = fork();
        
        if (pids[p] < 0) {
            // Fork failed
            fprintf(stderr, "Error forking process %d\n", p);
            exit(1);
        } else if (pids[p] == 0) {
            // Child process
            int start_row = p * rows_per_process;
            int end_row;
            
            // Handle the last process (may have more rows)
            if (p == num_processes - 1) {
                end_row = n;
            } else {
                end_row = (p + 1) * rows_per_process;
            }
            
            // Perform matrix multiplication for the assigned rows
            process_matrix_multiply(A, B, C, n, start_row, end_row);
            
            // Child process exits after completing its work
            exit(0);
        }
        // Parent process continues to create more children
    }
    
    // Parent process waits for all child processes to complete
    for (int p = 0; p < num_processes; p++) {
        int status;
        waitpid(pids[p], &status, 0);
        
        if (WIFEXITED(status)) {
            // Process terminated normally
            // printf("Process %d exited with status %d\n", p, WEXITSTATUS(status));
        } else {
            fprintf(stderr, "Process %d terminated abnormally\n", p);
        }
    }
}

// Function to display matrix (for debugging purposes, can be removed if needed)
void display_matrix(int32_t** matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%8d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int n, num_processes;
    
    // Check command line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <matrix_size> <num_processes>\n", argv[0]);
        return 1;
    }
    
    // Parse matrix size from command line
    n = atoi(argv[1]);
    
    // Parse num_processes from command line
    num_processes = atoi(argv[2]);
    
    if (n <= 0) {
        fprintf(stderr, "Error: Matrix size must be a positive integer\n");
        return 1;
    }
    
    // Shared memory file descriptors and sizes
    int shm_fd_A, shm_fd_B, shm_fd_C;
    size_t shm_size_A, shm_size_B, shm_size_C;
    
    // Allocate shared memory for matrices A, B, and C
    int32_t** A = allocate_shared_matrix(n, "/matrix_A", &shm_fd_A, &shm_size_A);
    int32_t** B = allocate_shared_matrix(n, "/matrix_B", &shm_fd_B, &shm_size_B);
    int32_t** C = allocate_shared_matrix(n, "/matrix_C", &shm_fd_C, &shm_size_C);
    
    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Error: Shared memory allocation failed\n");
        return 1;
    }
    
    // Generate random values for matrices A and B
    generate_matrix(A, n, 0);
    generate_matrix(B, n, 1000);
    
    // Start time measurement
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Perform matrix multiplication: C = A * B
    matrix_multiply(A, B, C, n, num_processes);

    // End time measurement
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Optional: Display results for small matrices (uncomment if needed)
    // if (n <= 10) {
    //     printf("Matrix A:\n");
    //     display_matrix(A, n);
    //     printf("Matrix B:\n");
    //     display_matrix(B, n);
    //     printf("Matrix C (A * B):\n");
    //     display_matrix(C, n);
    // }
    
    // printf("Matrix multiplication completed successfully.\n");
    // printf("Matrix size: %d x %d\n", n, n);
    // printf("Number of processes: %d\n", num_processes);
    // printf("Execution time: %.9f seconds\n", execution_time);

    // Save results to CSV file
    FILE *csv_file = fopen("results.csv", "a");
    if (csv_file != NULL) {
        fprintf(csv_file, "processes,%d,%d,%.9f\n", n, num_processes, execution_time);
        fclose(csv_file);
    } else {
        fprintf(stderr, "Error opening results.csv\n");
    }
    
    // Free allocated shared memory
    free_shared_matrix(A, "/matrix_A", shm_fd_A, shm_size_A);
    free_shared_matrix(B, "/matrix_B", shm_fd_B, shm_size_B);
    free_shared_matrix(C, "/matrix_C", shm_fd_C, shm_size_C);
    
    return 0;
}

// # Compile the program
// cc matrix_mult_processes.c -o matrix_mult_processes.out -lrt

// # Run with different matrix sizes and process numbers
// ./matrix_mult_processes.out 100 4   # 100x100 matrices with 4 processes