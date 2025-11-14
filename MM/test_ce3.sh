#!/bin/bash

MATRIX_SIZES=(400 800 1600 3200)
NUM_PROCESSES=(4 8 16)
NUM_RUNS=10

# Create CSV file
RESULTS_FILE="results_ce3.csv"
echo "implementation,matrix_size,parallel_units,execution_time" > $RESULTS_FILE

# Function to run tests
run_test() {
    local program=$1
    local size=$2
    local parallel_units=$3
    
    echo "Ejecutando: $program $size $parallel_units (ejecución $run/$NUM_RUNS)"
    mpirun -np  $parallel_units -hosts head,wn1,wn2,wn3 ./$program $size
}

echo "Ejecutando pruebas con procesos por secuencia de tamaños..."
for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- Procesos: ejecución $run/$NUM_RUNS ---"
    for processes in "${NUM_PROCESSES[@]}"; do
        for size in "${MATRIX_SIZES[@]}"; do
            run_test matrix_mult_mpi.out $size $processes
        done
    done
done

echo "Pruebas completadas. Resultados guardados en $RESULTS_FILE"