#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <mpi.h>

// Function to allocate contiguous memory for a matrix
int32_t* allocate_matrix_1d(int rows, int cols) {
    return (int32_t*)calloc(rows * cols, sizeof(int32_t));
}

// Matrix multiplication for local block
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
    int rank, size;
    int n;                      // Matrix size
    int32_t *A_full = NULL;     // Only rank 0 loads this
    int32_t *B = NULL;          // All processes need matrix B
    int32_t *A_local = NULL;
    int32_t *C_local = NULL;
    int32_t *C_full = NULL;
    double start_time, end_time, execution_time;

    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank of the current process (process ID)
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check command line arguments
    if (argc != 2) {
        if (rank == 0)
            fprintf(stderr, "Usage: %s <matrix_file>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    char* filename = argv[1];

    // Only rank 0 loads matrices from file
    if (rank == 0) {
        FILE* f = fopen(filename, "rb");
        if (!f) {
            fprintf(stderr, "Error opening file %s\n", filename);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Load size n
        fread(&n, sizeof(int), 1, f);

        A_full = allocate_matrix_1d(n, n);
        B = allocate_matrix_1d(n, n);

        if (!A_full || !B) {
            fprintf(stderr, "Error: memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Read A and B
        fread(A_full, sizeof(int32_t), n*n, f);
        fread(B, sizeof(int32_t), n*n, f);

        fclose(f);
        printf("Matrices loaded successfully from %s\n", filename);
    }

    // Broadcast matrix size n to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (n % size != 0) {
        if (rank == 0)
            fprintf(stderr, "Error: n (%d) is not divisible by number of processes (%d)\n", n, size);
        MPI_Finalize();
        return 1;
    }

    int local_rows = n / size;

    // All processes allocate B, A_local, C_local
    if (rank != 0)
        B = allocate_matrix_1d(n, n);

    A_local = allocate_matrix_1d(local_rows, n);
    C_local = allocate_matrix_1d(local_rows, n);

    if (!A_local || !B || !C_local) {
        fprintf(stderr, "Error: memoria insuficiente en proceso %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Broadcast matrix B to all processes
    MPI_Bcast(B, n*n, MPI_INT32_T, 0, MPI_COMM_WORLD);

    // Scatter rows of A to all processes
    MPI_Scatter(A_full, local_rows*n, MPI_INT32_T,
                A_local, local_rows*n, MPI_INT32_T,
                0, MPI_COMM_WORLD);

    // Prepare rank 0 for gathering C
    if (rank == 0)
        C_full = allocate_matrix_1d(n, n);

     // Synchronize all processes before starting computation
    MPI_Barrier(MPI_COMM_WORLD);

    // Start global time measurement using MPI timer
    start_time = MPI_Wtime();

    // Local multiplication
    matrix_multiply_1d(A_local, B, C_local, local_rows, n);

    // Synchronize all processes after computation
    MPI_Barrier(MPI_COMM_WORLD);

    // End global time measurement using MPI timer
    end_time = MPI_Wtime();

    // Calculate execution time
    execution_time = end_time - start_time;

    // Gather results from all processes back to rank 0
    MPI_Gather(C_local, local_rows*n, MPI_INT32_T,
               C_full, local_rows*n, MPI_INT32_T,
               0, MPI_COMM_WORLD);

    //  Rank 0 prints results
    if (rank == 0) {
        // Optional: Display results for small matrices (uncomment if needed)
        if (n <= 10) {
            printf("Matrix A:\n");
            display_matrix_1d(A_full, n, n);
            printf("Matrix B:\n");
            display_matrix_1d(B, n, n);
            printf("Matrix C (A * B):\n");
            display_matrix_1d(C_full, n, n);
        }

        printf("Parallel matrix multiplication completed successfully.\n");
        printf("Matrix size: %d x %d\n", n, n);
        printf("Number of processes: %d\n", size);
        printf("Execution time: %.9f seconds\n", execution_time);

        FILE* csv = fopen("results_mpi.csv", "a");
        if (csv != NULL) {
            fprintf(csv, "%d,%d,%.9f\n", n, size, execution_time);
            fclose(csv);
        } else {
            fprintf(stderr, "Error opening results_mpi.csv\n");
        }
    }

    // Free full matrices
    if (rank == 0) {
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
mpicc matrix_mult_mpi_v2.c -o matrix_mult_mpi_v2.out

# Run with different numbers of processes and matrix sizes
mpirun -np 2 ./matrix_mult_mpi_v2.out matrices.bin    # 2 processes, use matrices.bin

*/
