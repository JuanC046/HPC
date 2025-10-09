#!/bin/bash

MATRIX_SIZES=(800 1600 3200)
NUM_PROCESSES=(8 12 16)
NUM_THREADS=(8 12 16)
NUM_RUNS=10

# Create CSV file
RESULTS_FILE="results.csv"
echo "implementation,matrix_size,parallel_units,execution_time" > $RESULTS_FILE

# Compile programs
echo "Compilando programas..."
gcc matrix_mult.c -o matrix_mult.out -lm
gcc matrix_mult_O3.c -o matrix_mult_O3.out -lm
gcc matrix_mult_transpose.c -o matrix_mult_transpose.out -lm
gcc matrix_mult_threads.c -o matrix_mult_threads.out -lpthread -lm
gcc matrix_mult_processes.c -o matrix_mult_processes.out -lrt -lm
gcc -fopenmp matrix_mult_omp.c -o matrix_mult_omp.out

# Function to run tests
run_test() {
    local program=$1
    local size=$2
    local parallel_units=$3
    
    echo "Ejecutando: $program $size $parallel_units (ejecución $run/$NUM_RUNS)"
    if [ -z "$parallel_units" ]; then
        ./$program $size
    else
        ./$program $size $parallel_units
    fi
}

echo "Ejecutando pruebas secuenciales por secuencia de tamaños..."
for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- Secuencial: ejecución $run/$NUM_RUNS ---"
    for size in "${MATRIX_SIZES[@]}"; do
        run_test matrix_mult.out $size
    done
done
echo "Ejecutando pruebas secuenciales por secuencia de tamaños matriz B transpuesta..."
for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- Secuencial: ejecución $run/$NUM_RUNS ---"
    for size in "${MATRIX_SIZES[@]}"; do
        run_test matrix_mult_transpose.out $size
    done
done
echo "Ejecutando pruebas secuenciales con optimizacion O3 por secuencia de tamaños..."
for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- Secuencial: ejecución $run/$NUM_RUNS ---"
    for size in "${MATRIX_SIZES[@]}"; do
        run_test matrix_mult_O3.out $size
    done
done
echo "Ejecutando pruebas con hilos por secuencia de tamaños..."
for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- Hilos: ejecución $run/$NUM_RUNS ---"
    for threads in "${NUM_THREADS[@]}"; do
        for size in "${MATRIX_SIZES[@]}"; do
            run_test matrix_mult_threads.out $size $threads
        done
    done
done
echo "Ejecutando pruebas con procesos por secuencia de tamaños..."
for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- Procesos: ejecución $run/$NUM_RUNS ---"
    for processes in "${NUM_PROCESSES[@]}"; do
        for size in "${MATRIX_SIZES[@]}"; do
            run_test matrix_mult_processes.out $size $processes
        done
    done
done
echo "Ejecutando pruebas con hilos OMP por secuencia de tamaños..."
for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- Procesos: ejecución $run/$NUM_RUNS ---"
    for threads in "${NUM_THREADS[@]}"; do
        for size in "${MATRIX_SIZES[@]}"; do
            run_test matrix_mult_omp.out $size $threads
        done
    done
done

echo "Pruebas completadas. Resultados guardados en $RESULTS_FILE"