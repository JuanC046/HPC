#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

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
            matrix[i][j] = (rand() % 2000);
        }
    }
}

// Sequential matrix multiplication function
void matrix_multiply(int32_t** A, int32_t** B, int32_t** C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
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
    int n;
    
    // Check command line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <matrix_size>\n", argv[0]);
        return 1;
    }
    
    // Parse matrix size from command line
    n = atoi(argv[1]);
    
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
    
    // Start time measurement
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Perform matrix multiplication: C = A * B
    matrix_multiply(A, B, C, n);

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
    // printf("Execution time: %.9f seconds\n", execution_time);
    
    // Save results to CSV file
    FILE *csv_file = fopen("results.csv", "a");
    if (csv_file != NULL) {
        fprintf(csv_file, "sequential,%d,1,%.9f\n", n, execution_time);
        fclose(csv_file);
    } else {
        fprintf(stderr, "Error opening results.csv\n");
    }
    
    // Free allocated memory
    free_matrix(A, n);
    free_matrix(B, n);
    free_matrix(C, n);
    
    return 0;
}

// # Compile the program
// cc matrix_mult.c -o matrix_mult.out


// # Run with different matrix sizes
// ./matrix_mult.out 100    # 100x100 matrices
