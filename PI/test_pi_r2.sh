#!/bin/bash

# Output file
OUTPUT_FILE="test_results_r2.csv"

# Initialize CSV with header
echo "Method,Implementation,Iterations,Units,Parameter1,Parameter2,Estimated_PI,Difference,Error,Time" > $OUTPUT_FILE

# Compile programs
gcc Needle/needle_omp.c -o Needle/needle_omp.out -fopenmp -lm
gcc Dartboard/dartboard_omp.c -o Dartboard/dartboard_omp.out -fopenmp -lm


# Test configurations
ITERATIONS="200000000 2000000000"
PARALLEL_UNITS="4 8 16"

# Needles: line_distance y needle_length
LINE_DISTANCES="1600 3200"
NEEDLE_LENGTHS="800"

# Dartboard: circle radius
RADII="800 1600"

# Number of runs
NUM_RUNS=10

##################################
#   BUFFON'S NEEDLE
##################################
for N in $ITERATIONS; do
  for L in $LINE_DISTANCES; do
    for LEN in $NEEDLE_LENGTHS; do

      # OMP
      for ((run=1; run<=$NUM_RUNS; run++)); do
        for T in $PARALLEL_UNITS; do
          RESULT=$(./Needle/needle_omp.out $L $LEN $N $T | grep -E "Estimated|Difference|Error|Time")
          EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
          DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
          ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
          TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
          echo "Needle,OMP,$N,$T,$L,$LEN,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
        done
      done
 
    done
  done
done

##################################
#   DARTBOARD
##################################
for N in $ITERATIONS; do
  for R in $RADII; do

    # OMP
    for ((run=1; run<=$NUM_RUNS; run++)); do
      for T in $PARALLEL_UNITS; do
        RESULT=$(./Dartboard/dartboard_omp.out $R $N $T | grep -E "Estimated|Difference|Error|Time")
        EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
        DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
        ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
        TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
        echo "Dartboard,OMP,$N,$T,$R,-,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
      done
    done

  done
done

echo "Tests completed. Results saved in $OUTPUT_FILE"
