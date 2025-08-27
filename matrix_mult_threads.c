#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

// Function to allocate memory for a square matrix
int32_t** allocate_matrix(int n) {
    int32_t** matrix = (int32_t**)malloc(n * sizeof(int32_t*));
    for (int i = 0; i < n; i++) {
        matrix[i] = (int32_t*)malloc(n * sizeof(int32_t));
    }
    return matrix;
}

// Function to free memory allocated for a matrix
void free_matrix(int32_t** matrix, int n) {
    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Function to generate random values for matrix
void generate_matrix(int32_t** matrix, int n, unsigned int seed_offset) {
    srand(time(NULL) + seed_offset);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            // Generate random int32 values between 0 and 2000
            matrix[i][j] = (rand() % 20001);
        }
    }
}


// Structure to pass data to threads
/*What the pointers are doing here is:

-   Allowing efficient passing of the large matrices to threads 
    without copying the data
-   Enabling each thread to access the same shared memory space
These above statements represent one of the adantages of shared memory systems in HPC
*/
typedef struct {
    int32_t** A; // Pointer to matrix A
    int32_t** B; // Pointer to matrix B
    int32_t** C; // Pointer to matrix C
    int n;
    int start_row;
    int end_row;
} ThreadData;

// Thread function for matrix multiplication
void* thread_matrix_multiply(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int32_t** A = data->A;
    int32_t** B = data->B;
    int32_t** C = data->C;
    int n = data->n;
    int start_row = data->start_row;
    int end_row = data->end_row;

    // Each thread computes its assigned rows
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    pthread_exit(NULL);
}

// Parallel matrix multiplication function using N threads
void matrix_multiply(int32_t** A, int32_t** B, int32_t** C, int n, int num_threads) {
    
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    // Calculate rows per thread (at least 1)
    int rows_per_thread = n / num_threads;
    if (rows_per_thread == 0) {
        rows_per_thread = 1;
        num_threads = n; // Adjust number of threads if n is small
    }

    // Create and launch threads
    for (int t = 0; t < num_threads; t++) {
        thread_data[t].A = A;
        thread_data[t].B = B;
        thread_data[t].C = C;
        thread_data[t].n = n;
        thread_data[t].start_row = t * rows_per_thread;

        // Handle the last thread (may have more rows)
        if (t == num_threads - 1) {
            thread_data[t].end_row = n;
        } else {
            thread_data[t].end_row = (t + 1) * rows_per_thread;
        }

        // Create the thread
        if (pthread_create(&threads[t], NULL, thread_matrix_multiply, &thread_data[t]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", t);
            exit(1);
        }
    }

    // Wait for all threads to complete
    for (int t = 0; t < num_threads; t++) {
        if (pthread_join(threads[t], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", t);
            exit(1);
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
    int n, num_threads;
    
    // Check command line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <matrix_size> <num_threads>\n", argv[0]);
        return 1;
    }
    
    // Parse matrix size from command line
    n = atoi(argv[1]);

    // Parse num_threads from command line
    num_threads = atoi(argv[2]);
    
    if (n <= 0) {
        fprintf(stderr, "Error: Matrix size must be a positive integer\n");
        return 1;
    }
    
    // Allocate memory for matrices A, B, and C
    int32_t** A = allocate_matrix(n);
    int32_t** B = allocate_matrix(n);
    int32_t** C = allocate_matrix(n);
    
    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }
    
    // Generate random values for matrices A and B
    generate_matrix(A, n, 0);
    generate_matrix(B, n, 1000);
    
    // Perform matrix multiplication: C = A * B
    // clock_t start_time = clock();
    matrix_multiply(A, B, C, n, num_threads);
    // clock_t end_time = clock();
    
    // Calculate execution time
    // double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
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
    // printf("Execution time: %.6f seconds\n", execution_time);
    
    // Free allocated memory
    free_matrix(A, n);
    free_matrix(B, n);
    free_matrix(C, n);
    
    return 0;
}

// # Compile the program
// cc matrix_mult_threads.c -o matrix_mult_threads.out


// # Run with different matrix sizes and threads number
// ./matrix_mult_threads.out 100  4   # 100x100 matrices
