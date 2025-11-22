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
    int32_t *A_local, *B, *C_local, *C_full = NULL;
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

    // All processes open the binary file
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Process %d: Error opening file %s\n", rank, filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Read matrix size n from file
    fread(&n, sizeof(int), 1, f);

    if (n % size != 0) {
        if (rank == 0)
            fprintf(stderr, "Error: n (%d) is not divisible by number of processes (%d)\n", n, size);
        MPI_Finalize();
        return 1;
    }

    int local_rows = n / size;

    // All processes allocate B, A_local, C_local
    A_local = allocate_matrix_1d(local_rows, n);
    B = allocate_matrix_1d(n, n);
    C_local = allocate_matrix_1d(local_rows, n);

    if (!A_local || !B || !C_local) {
        fprintf(stderr, "Error: Memory allocation failed in process %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // read local block of A
    long offset_A = sizeof(int) + (long)rank * (local_rows * n * sizeof(int32_t));
    fseek(f, offset_A, SEEK_SET);
    fread(A_local, sizeof(int32_t), local_rows*n, f);

    // read full matrix B
    long offset_B = sizeof(int) + (long)n * n * sizeof(int32_t);
    fseek(f, offset_B, SEEK_SET);
    fread(B, sizeof(int32_t), n*n, f);

    fclose(f);

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

    if (rank == 0)
        C_full = allocate_matrix_1d(n, n);

    // Gather results from all processes back to rank 0
    MPI_Gather(C_local, local_rows*n, MPI_INT32_T,
               C_full, local_rows*n, MPI_INT32_T,
               0, MPI_COMM_WORLD);

    //  Rank 0 prints results
    if (rank == 0) {
        // Optional: Display results for small matrices (uncomment if needed)
        if (n <= 10) {
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
