#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// Initialize the road with cars based on density
void initialize_road(int *road, int n, double density) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        road[i] = (rand() / (double)RAND_MAX) < density ? 1 : 0;
    }
}

// Apply cellular automaton rules for one time step
int update_step(int *old_road, int *new_road, int n) {
    int cars_moved = 0;
    
    for (int i = 0; i < n; i++) {
        int left = (i - 1 + n) % n;   // Periodic boundary condition
        int right = (i + 1) % n;       // Periodic boundary condition
        
        // Rule: Rt+1(i) = Rt(i-1) AND NOT Rt(i)  OR  Rt(i) AND Rt(i+1)
        int car_arriving = old_road[left] && !old_road[i];
        int car_staying = old_road[i] && old_road[right];
        
        new_road[i] = car_arriving || car_staying;
        
        // Count cars that moved
        if (old_road[i] && !old_road[right]) {
            cars_moved++;
        }
    }
    
    return cars_moved;
}

// Count total number of cars on the road
int count_cars(int *road, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        count += road[i];
    }
    return count;
}

// Print the state of the road
void print_road(int *road, int n) {
    for (int i = 0; i < n; i++) {
        printf("%c", road[i] ? 'X' : '.');
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int n = 1000;              // Length of the road
    int iterations = 1000;     // Number of iterations
    double density = 0.3;      // Initial car density
    int print_every = 1000;     // Print frequency
    
    // Parse command line arguments
    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) iterations = atoi(argv[2]);
    if (argc > 3) density = atof(argv[3]);
    
    printf("=== Traffic Simulation - Serial Version ===\n");
    printf("Length of the road: %d\n", n);
    printf("Number of iterations: %d\n", iterations);
    printf("Initial car density: %.2f\n\n", density);
    
    // Allocate memory
    int *road_current = (int *)malloc(n * sizeof(int));
    int *road_next = (int *)malloc(n * sizeof(int));
    
    if (!road_current || !road_next) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }
    
    // Initialize road
    initialize_road(road_current, n, density);
    int total_cars = count_cars(road_current, n);
    
    printf("Total number of cars: %d\n", total_cars);
    if (n <= 100) {
        printf("Initial state:\n");
        print_road(road_current, n);
        printf("\n");
    }
    
    // Start time measurement
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int t = 0; t < iterations; t++) {
        // Update state
        int cars_moved = update_step(road_current, road_next, n);
        
        // Calculate average velocity
        double velocity = (total_cars > 0) ? 
                         (double)cars_moved / total_cars : 0.0;
        
        // Swap buffers
        int *temp = road_current;
        road_current = road_next;
        road_next = temp;
        
        // Print statistics
        if ((t + 1) % print_every == 0) {
            printf("Iteration %d: velocity = %.4f Cars moved: %d\n", t + 1, velocity, cars_moved);
            if (n <= 100) print_road(road_current, n);
        }
    }
    
    // End time measurement
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calculate execution time
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Resultados finales
    printf("\n=== Results ===\n");
    printf("Total execution time: %.6f seconds\n", execution_time);
    printf("Average time per iteration: %.6f ms\n", 
           (execution_time * 1000.0) / iterations);
    if (n <= 100) {
        printf("Final state:\n");
        print_road(road_current, n);
    }
    
    // Free memory
    free(road_current);
    free(road_next);
    
    return 0;
}

// Compile the program
// gcc cellular_automaton.c -o cellular_automaton.out

// Run with different parameters
// ./cellular_automaton.out [n] [iterations] [density]
// ./cellular_automaton.out 200 500 0.5
// ./cellular_automaton.out 1000 1000 0.3