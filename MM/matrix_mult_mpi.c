#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <mpi.h>

// Function to allocate contiguous memory for a matrix
int32_t* allocate_matrix_1d(int rows, int cols) {
    return (int32_t*)calloc(rows * cols, sizeof(int32_t));
}

// 1D arrays are required because MPI communication functions need 
// contiguous memory blocks for efficient data transfer between processes. 
// While there are workarounds, using 1D arrays from the start is the most 
// straightforward and efficient approach for MPI applications.

// Function to generate random values for matrix (1D representation)
void generate_matrix_1d(int32_t* matrix, int rows, int cols, unsigned int seed_offset) {
    srand(time(NULL) + seed_offset);
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (rand() % 2000);
    }
}

// Matrix multiplication function using 1D arrays with 2D indexing
void matrix_multiply_1d(int32_t* A_local, int32_t* B, int32_t* C_local,
                             int local_rows, int n) {
    // Initialize result matrix to zero
    for (int i = 0; i < local_rows * n; i++) {
        C_local[i] = 0;
            }

    // Perform matrix multiplication: C = A * B
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                // 2D indexing: matrix[i][j] = matrix[i * cols + j]
                C_local[i * n + j] += A_local[i * n + k] * B[k * n + j];
    }
        }
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
    int rank, size;
    double start_time, end_time, execution_time;

    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank of the current process (process ID)
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check command line arguments (only rank 0 needs to validate)
    if (argc != 2) {
    if (rank == 0) {
            fprintf(stderr, "Usage: %s <matrix_size>\n", argv[0]);
        }
    MPI_Finalize();
        return 1;
}

    // Parse matrix size from command line
    n = atoi(argv[1]);

    if (n <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: Matrix size must be a positive integer\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Check if matrix size is divisible by number of processes
    if (n % size != 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: Matrix size (%d) must be divisible by number of processes (%d)\n", n, size);
        }
        MPI_Finalize();
        return 1;
    }

    // Calculate number of rows each process will handle
    int local_rows = n / size;

    // Allocate memory for matrices using 1D arrays
    int32_t* A_full = NULL;        // Full matrix A (only on rank 0)
    int32_t* A_local = NULL;       // Local portion of matrix A
    int32_t* B = NULL;             // Full matrix B (on all processes)
    int32_t* C_full = NULL;        // Full result matrix C (only on rank 0)
    int32_t* C_local = NULL;       // Local portion of result matrix C

    // Allocate local matrices for all processes
    A_local = allocate_matrix_1d(local_rows, n);
    B = allocate_matrix_1d(n, n);
    C_local = allocate_matrix_1d(local_rows, n);

    if (A_local == NULL || B == NULL || C_local == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in process %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Allocate full matrices only on rank 0
    if (rank == 0) {
        A_full = allocate_matrix_1d(n, n);
        C_full = allocate_matrix_1d(n, n);

        if (A_full == NULL || C_full == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for full matrices on rank 0\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Generate random values for matrices A and B
        generate_matrix_1d(A_full, n, n, 0);
        generate_matrix_1d(B, n, n, 1000);

        printf("Matrices generated successfully on rank 0\n");
    }

    // Broadcast matrix B to all processes
    // All processes need the complete matrix B for computation
    MPI_Bcast(B, n * n, MPI_INT32_T, 0, MPI_COMM_WORLD);

    // Scatter rows of matrix A to all processes
    // Each process receives local_rows consecutive rows
    MPI_Scatter(A_full, local_rows * n, MPI_INT32_T,
               A_local, local_rows * n, MPI_INT32_T, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Data distribution completed\n");
    }

    // Synchronize all processes before starting computation
    MPI_Barrier(MPI_COMM_WORLD);

    // Start global time measurement using MPI timer
    start_time = MPI_Wtime();

    // Perform parallel matrix multiplication: C_local = A_local * B
    matrix_multiply_1d(A_local, B, C_local, local_rows, n);

    // Synchronize all processes after computation
    MPI_Barrier(MPI_COMM_WORLD);

    // End global time measurement using MPI timer
    end_time = MPI_Wtime();

    // Calculate execution time
    execution_time = end_time - start_time;

    if (rank == 0) {
        printf("Computation completed\n");
    }

    // Gather results from all processes back to rank 0
    MPI_Gather(C_local, local_rows * n, MPI_INT32_T,
              C_full, local_rows * n, MPI_INT32_T, 0, MPI_COMM_WORLD);

    // Only rank 0 handles output and file writing
    if (rank == 0) {
        // Optional: Display results for small matrices (uncomment if needed)
        // if (n <= 10) {
        //     printf("Matrix A:\n");
        //     display_matrix_1d(A_full, n, n);
        //     printf("Matrix B:\n");
        //     display_matrix_1d(B, n, n);
        //     printf("Matrix C (A * B):\n");
        //     display_matrix_1d(C_full, n, n);
        // }

        // printf("Parallel matrix multiplication completed successfully.\n");
        // printf("Matrix size: %d x %d\n", n, n);
        // printf("Number of processes: %d\n", size);
        // printf("Execution time: %.9f seconds\n", execution_time);

        // Save results to CSV file
        FILE *csv_file = fopen("results_ce3.csv", "a");
        if (csv_file != NULL) {
            fprintf(csv_file, "mpi,%d,%d,%.9f\n", n, size, execution_time);
            fclose(csv_file);
        } else {
            fprintf(stderr, "Error opening results.csv\n");
        }

        // Free full matrices
        free(A_full);
        free(C_full);
    }

    // Free local matrices
    free(A_local);
    free(B);
    free(C_local);

    // Finalize MPI environment
    MPI_Finalize();

    return 0;
}

/*
Compilation and execution instructions:

# Compile the MPI program
mpicc matrix_mult_mpi.c -o matrix_mult_mpi.out

# Run with different numbers of processes and matrix sizes
mpirun -np 2 ./matrix_mult_mpi.out 100    # 2 processes, 100x100 matrices
mpirun -np 4 ./matrix_mult_mpi.out 200    # 4 processes, 200x200 matrices
mpirun -np 8 ./matrix_mult_mpi.out 400    # 8 processes, 400x400 matrices

Note: Matrix size must be divisible by the number of processes
*/
