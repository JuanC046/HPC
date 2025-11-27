#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <mpi.h>

// Inicializa solo la porción local del proceso
void initialize_road_local(int8_t *road, int local_n, double density, int rank) {
    srand(time(NULL) + rank * 1000); 
    for (int i = 1; i <= local_n; i++) {
        road[i] = (rand() / (double)RAND_MAX) < density ? 1 : 0;
    }
}

// Intercambio de fronteras (Halo Exchange)
void exchange_borders(int8_t *road, int local_n, int left_neighbor, int right_neighbor, MPI_Comm comm) {
    MPI_Sendrecv(&road[1], 1, MPI_INT8_T, left_neighbor, 0,
                 &road[local_n + 1], 1, MPI_INT8_T, right_neighbor, 0,
                 comm, MPI_STATUS_IGNORE);

    MPI_Sendrecv(&road[local_n], 1, MPI_INT8_T, right_neighbor, 1,
                 &road[0], 1, MPI_INT8_T, left_neighbor, 1,
                 comm, MPI_STATUS_IGNORE);
}

// Actualización optimizada (sin módulo, con punteros restrict)
int update_step(const int8_t *restrict old_road, int8_t *restrict new_road, int local_n) {
    int cars_moved = 0;
    
    // Bucle lineal optimizado
    for (int i = 1; i <= local_n; i++) {
        int8_t left_val = old_road[i-1];
        int8_t curr_val = old_road[i];
        int8_t right_val = old_road[i+1];

        int car_arriving = left_val && !curr_val;
        int car_staying = curr_val && right_val;
        
        new_road[i] = car_arriving || car_staying;
        
        if (curr_val && !right_val) {
            cars_moved++;
        }
    }
    return cars_moved;
}

int count_cars_local(int8_t *road, int local_n) {
    int count = 0;
    for (int i = 1; i <= local_n; i++) {
        count += road[i];
    }
    return count;
}


void gather_and_print(int8_t *local_road, int local_n, int n, int rank, int size) {
    int8_t *full_road = NULL;
    if (rank == 0) {
        full_road = (int8_t *)malloc(n * sizeof(int8_t));
    }
    
    int *recvcounts = NULL;
    int *displs = NULL;
    
    if (rank == 0) {
        recvcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        int offset = 0;
        int remainder = n % size;
        for (int i = 0; i < size; i++) {
            recvcounts[i] = n / size + (i < remainder ? 1 : 0);
            displs[i] = offset;
            offset += recvcounts[i];
        }
    }
    
    MPI_Gatherv(&local_road[1], local_n, MPI_INT8_T,
                full_road, recvcounts, displs, MPI_INT8_T,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            printf("%c", full_road[i] ? 'X' : '.');
        }
        printf("\n");
        free(full_road);
        free(recvcounts);
        free(displs);
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = 1000;              
    int iterations = 1000;     
    double density = 0.3;      
    int print_every = 1000;    
    
    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) iterations = atoi(argv[2]);
    if (argc > 3) density = atof(argv[3]);

    // 1. Balanceo de carga
    int local_n = n / size;
    int remainder = n % size;
    if (rank < remainder) {
        local_n++;
    }

    // 2. Memoria (+2 Ghost Cells)
    int8_t *road_current = (int8_t *)calloc(local_n + 2, sizeof(int8_t));
    int8_t *road_next = (int8_t *)calloc(local_n + 2, sizeof(int8_t));

    if (!road_current || !road_next) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // 3. Vecinos
    int left_neighbor = (rank - 1 + size) % size;
    int right_neighbor = (rank + 1) % size;

    // 4. Inicialización
    initialize_road_local(road_current, local_n, density, rank);

    // Conteo inicial
    int local_cars = count_cars_local(road_current, local_n);
    int total_cars = 0;
    MPI_Allreduce(&local_cars, &total_cars, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("=== Traffic Simulation - MPI Optimized ===\n");
        printf("MPI Size: %d processes\n", size);
        printf("Length: %d, Iterations: %d, Density: %.2f\n", n, iterations, density);
        printf("Total cars: %d\n", total_cars);
    }
    
    // if (n <= 100) gather_and_print(road_current, local_n, n, rank, size); // Comentado

    // === INICIO MEDICIÓN DE TIEMPO ===
    // Sincronizamos a todos los procesos antes de iniciar el cronómetro
    MPI_Barrier(MPI_COMM_WORLD);
    
    double start_time = 0.0;
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    // === BUCLE PRINCIPAL ===
    for (int t = 0; t < iterations; t++) {
        
        exchange_borders(road_current, local_n, left_neighbor, right_neighbor, MPI_COMM_WORLD);
        
        int local_moved = update_step(road_current, road_next, local_n);

        int8_t *temp = road_current;
        road_current = road_next;
        road_next = temp;

        // if ((t + 1) % print_every == 0) {
        //     int global_moved = 0;
        //     // Reducción necesaria solo si vamos a imprimir estadísticas
        //     MPI_Reduce(&local_moved, &global_moved, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            
        //     if (rank == 0) {
        //         double velocity = (total_cars > 0) ? (double)global_moved / total_cars : 0.0;
        //         printf("Iter %d: Vel=%.4f Moved=%d\n", t + 1, velocity, global_moved);
        //     }
            // if (n <= 100) gather_and_print(...); // Comentado
        // }
    }

    // === FIN MEDICIÓN DE TIEMPO ===
    // Sincronizamos para asegurarnos que Rank 0 no mida el tiempo hasta que EL ÚLTIMO termine
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        double end_time = MPI_Wtime();
        double execution_time = end_time - start_time;
        
        printf("\n=== Results (Rank 0 Measurement) ===\n");
        printf("Total execution time: %.6f seconds\n\n", execution_time);
        // Escritura de resultados en CSV 
    FILE* csv = fopen("results_ca.csv", "a");
    if (csv != NULL) {
        fprintf(csv, "mpi,%d,%d,%d,%.9f\n", n, iterations, size, execution_time);
        fclose(csv);
    } else {
        fprintf(stderr, "Error opening results_ca.csv\n");
    }
    }

    free(road_current);
    free(road_next);

    MPI_Finalize();
    return 0;
}