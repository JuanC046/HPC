#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <mpi.h>

int32_t* allocate_matrix(int rows, int cols) {
    return (int32_t*)calloc(rows * cols, sizeof(int32_t));
}

void generate_matrix(int32_t* M, int rows, int cols, unsigned int seed) {
    srand(seed);
    for (int i = 0; i < rows * cols; i++)
        M[i] = rand() % 2000;
}

void matrix_mult_block(int32_t* A_local, int32_t* B, int32_t* C_local, int block_rows, int n) {
    for (int i = 0; i < block_rows; i++)
        for (int k = 0; k < n; k++) {
            int32_t aik = A_local[i*n + k];
            for (int j = 0; j < n; j++)
                C_local[i*n + j] += aik * B[k*n + j];
        }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) printf("Usage: %s <n>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]);
    int block_rows = n / size;

    // Each process has full B and full A, but computes only its rows.
    int32_t* A = allocate_matrix(n, n);
    int32_t* B = allocate_matrix(n, n);
    int32_t* C_local = allocate_matrix(block_rows, n);

    // All nodes generate the same matrices locally → no communication.
    generate_matrix(A, n, n, 42);
    generate_matrix(B, n, n, 1337);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    // Local block = rows [rank*block_rows ... (rank+1)*block_rows]
    matrix_mult_block(
        &A[rank * block_rows * n],
        B,
        C_local,
        block_rows,
        n
    );

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    if (rank == 0) {
        double execution_time = end - start;
        FILE *csv_file = fopen("results_ce3.csv", "a");
        if (csv_file != NULL) {
            fprintf(csv_file, "mpi,%d,%d,%.9f\n", n, size, execution_time);
            fclose(csv_file);
        } else {
            fprintf(stderr, "Error opening results.csv\n");
        }
    }

    free(A);
    free(B);
    free(C_local);

    MPI_Finalize();
    return 0;
}