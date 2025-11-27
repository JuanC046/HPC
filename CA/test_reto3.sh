#!/bin/bash

ITERATIONS=(400 800 1600 3200)
NUM_PROCESSES=(4 8 16)
ROAD_SIZE=200000000
DENSITY=0.7
NUM_RUNS=1

# Create CSV file
RESULTS_FILE="results_ca.csv"
echo "implementation,road_size,iterations,parallel_units,execution_time" > $RESULTS_FILE

# Function to run tests
run_test() {
    local program=$1
    local iterations=$2
    local parallel_units=$3

    
    echo "Ejecutando: $program $iterations $parallel_units (ejecución $run/$NUM_RUNS)"
    if [ -z "$parallel_units" ]; then
        ./$program $ROAD_SIZE $iterations $DENSITY
    else
        #  mpiexec -n  $parallel_units -hosts head,wn1,wn2,wn3 ./$program $ROAD_SIZE $iterations $DENSITY
        mpiexec -n  $parallel_units ./$program $ROAD_SIZE $iterations $DENSITY
    fi
   
}

echo "Ejecutando pruebas con procesos por secuencia de numero de iteraciones..."
for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- Secuencial: ejecución $run/$NUM_RUNS ---"
        for iteration in "${ITERATIONS[@]}"; do
            run_test cellular_automaton.out $iteration $processes
        done
done

for ((run=1; run<=$NUM_RUNS; run++)); do
    echo "\n--- MPI: ejecución $run/$NUM_RUNS ---"
    for processes in "${NUM_PROCESSES[@]}"; do
        for iteration in "${ITERATIONS[@]}"; do
            run_test cellular_automaton_mpi.out $iteration $processes
        done
    done
done

echo "Pruebas completadas. Resultados guardados en $RESULTS_FILE"