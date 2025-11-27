#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h> // Necesario para int8_t

// OPTIMIZACIÓN 1: Uso de int8_t para mejorar localidad de caché (1 byte vs 4 bytes)
void initialize_road(int8_t *road, int n, double density) {
    for (int i = 0; i < n; i++) {
        road[i] = (rand() / (double)RAND_MAX) < density ? 1 : 0;
    }
}

// OPTIMIZACIÓN 2: Uso de 'restrict' para permitir vectorización del compilador
int update_step(const int8_t *restrict old_road, int8_t *restrict new_road, int n) {
    int cars_moved = 0;
    
    // OPTIMIZACIÓN 3: Loop Peeling (Sacar fronteras del bucle principal)
    // Esto elimina el costoso operador módulo (%) del bucle crítico.
    
    // --- Tratar frontera izquierda (i = 0) ---
    {
        int i = 0;
        int left = n - 1; // Periodicidad
        int right = 1;
        
        int car_arriving = old_road[left] && !old_road[i];
        int car_staying = old_road[i] && old_road[right];
        new_road[i] = car_arriving || car_staying;
        
        if (old_road[i] && !old_road[right]) cars_moved++;
    }

    // --- Bucle Principal (i = 1 hasta n-2) ---
    // Acceso puramente secuencial, sin modulo, amigable con Prefetcher y SIMD
    for (int i = 1; i < n - 1; i++) {
        // Al no usar %, el compilador puede vectorizar esto fácilmente
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

    // --- Tratar frontera derecha (i = n - 1) ---
    {
        int i = n - 1;
        int left = n - 2;
        int right = 0; // Periodicidad
        
        int car_arriving = old_road[left] && !old_road[i];
        int car_staying = old_road[i] && old_road[right];
        new_road[i] = car_arriving || car_staying;
        
        if (old_road[i] && !old_road[right]) cars_moved++;
    }
    
    return cars_moved;
}

int count_cars(int8_t *road, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        count += road[i];
    }
    return count;
}

void print_road(int8_t *road, int n) {
    for (int i = 0; i < n; i++) {
        printf("%c", road[i] ? 'X' : '.');
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int n = 1000;              
    int iterations = 1000;     
    double density = 0.3;      
    int print_every = 1;    
    
    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) iterations = atoi(argv[2]);
    if (argc > 3) density = atof(argv[3]);
    
    srand(time(NULL)); // Mover srand aquí

    printf("=== Traffic Simulation - Optimized Serial Version ===\n");
    printf("Length of the road: %d\n", n);
    printf("Number of iterations: %d\n", iterations);
    printf("Initial car density: %.2f\n\n", density);
    
    // Cambiado a int8_t
    int8_t *road_current = (int8_t *)malloc(n * sizeof(int8_t));
    int8_t *road_next = (int8_t *)malloc(n * sizeof(int8_t));
    
    if (!road_current || !road_next) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }
    
    initialize_road(road_current, n, density);
    int total_cars = count_cars(road_current, n);
    
    printf("Total number of cars: %d\n", total_cars);
    if (n <= 100) {
        printf("Initial state:\n");
        print_road(road_current, n);
        printf("\n");
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int t = 0; t < iterations; t++) {
        int cars_moved = update_step(road_current, road_next, n);
              
        // Swap pointers
        int8_t *temp = road_current;
        road_current = road_next;
        road_next = temp;
        
        // if ((t + 1) % print_every == 0) {
        //      double velocity = (total_cars > 0) ? (double)cars_moved / total_cars : 0.0;
        //      printf("Iteration %d: velocity = %.4f Cars moved: %d\n", t + 1, velocity, cars_moved);
        //      if (n <= 100) print_road(road_current, n);
        // }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double avg_ms = (execution_time * 1000.0) / iterations;
    
    printf("\n=== Results ===\n");
    printf("Total execution time: %.6f seconds\n\n", execution_time);

    // Escritura de resultados en CSV 
    FILE* csv = fopen("results_ca.csv", "a");
    if (csv != NULL) {
        fprintf(csv, "sequential,%d,%d,1,%.9f\n", n, iterations, execution_time);
        fclose(csv);
    } else {
        fprintf(stderr, "Error opening results_ca.csv\n");
    }

    free(road_current);
    free(road_next);

    return 0;
}
