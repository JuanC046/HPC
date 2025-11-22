#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// Function to allocate contiguous memory for a matrix
int32_t* allocate_matrix_1d(int rows, int cols) {
    return (int32_t*)calloc(rows * cols, sizeof(int32_t));
}

// Function to generate random values for matrix (1D representation)
void generate_matrix_1d(int32_t* matrix, int rows, int cols, unsigned int seed_offset) {
    srand(time(NULL) + seed_offset);
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (rand() % 2000);
    }
}

// Function to display matrix (for debugging purposes)
void display_matrix_1d(int32_t* matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%8d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int n;
    char* filename;

    // Check command line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <matrix_size> <output_file>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    filename = argv[2];
    
    if (n <= 0) {
        fprintf(stderr, "Error: Matrix size must be a positive integer\n");
        return 1;
    }

    int32_t* A = allocate_matrix_1d(n, n);
    int32_t* B = allocate_matrix_1d(n, n);

    if (A == NULL || B == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    // Generate matrices
    generate_matrix_1d(A, n, n, 0);
    generate_matrix_1d(B, n, n, 1000);

    // Save matrices in binary form
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    // Save size n first
    fwrite(&n, sizeof(int), 1, f);

    // Write matrix A
    fwrite(A, sizeof(int32_t), n * n, f);

    // Write matrix B
    fwrite(B, sizeof(int32_t), n * n, f);

    fclose(f);

    // Display matrices (optional, for debugging)
    display_matrix_1d(A, n, n);
    display_matrix_1d(B, n, n);

    printf("Matrices A and B generated and stored in %s\n", filename);

    free(A);
    free(B);

    return 0;
}

// # Compile the program
// gcc generate_matrix.c -o generate_matrix.out


// # Run with different matrix sizes
// ./generate_matrix.out 100    # 100x100 matrices
