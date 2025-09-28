#!/bin/bash

# Output file
OUTPUT_FILE="test_results.csv"

# Initialize CSV with header
echo "Method,Implementation,Iterations,Units,Parameter1,Parameter2,Estimated_PI,Difference,Error,Time" > $OUTPUT_FILE

# Compile programs
gcc Needle/needle.c -o Needle/needle.out -lm
gcc Needle/needle.c -o Needle/needle_O3.out -lm -O3
gcc Needle/needle_threads.c -o Needle/needle_threads.out -lm -lpthread
gcc Needle/needle_processes.c -o Needle/needle_processes.out -lm
gcc Dartboard/dartboard.c -o Dartboard/dartboard.out -lm
gcc Dartboard/dartboard.c -o Dartboard/dartboard_O3.out -lm -O3
gcc Dartboard/dartboard_threads.c -o Dartboard/dartboard_threads.out -lm -lpthread
gcc Dartboard/dartboard_processes.c -o Dartboard/dartboard_processes.out -lm

# Test configurations
ITERATIONS="2000000 20000000 200000000 2000000000"
PARALLEL_UNITS="2 4 8 16"

# Needles: line_distance y needle_length
LINE_DISTANCES="1600 3200"
NEEDLE_LENGTHS="400 800"

# Dartboard: circle radius
RADII="400 800 1600"

# Number of runs
NUM_RUNS=10

##################################
#   BUFFON'S NEEDLE
##################################
for N in $ITERATIONS; do
  for L in $LINE_DISTANCES; do
    for LEN in $NEEDLE_LENGTHS; do

      # Sequential
      for ((run=1; run<=$NUM_RUNS; run++)); do
        RESULT=$(./Needle/needle.out $L $LEN $N | grep -E "Estimated|Difference|Error|Time")
        EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
        DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
        ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
        TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
        echo "Needle,Secuencial,$N,1,$L,$LEN,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
      done

      # Sequential - O3
      for ((run=1; run<=$NUM_RUNS; run++)); do
        RESULT=$(./Needle/needle_O3.out $L $LEN $N | grep -E "Estimated|Difference|Error|Time")
        EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
        DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
        ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
        TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
        echo "Needle,Secuencial_O3,$N,1,$L,$LEN,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
      done

      # Threads
      for ((run=1; run<=$NUM_RUNS; run++)); do
        for T in $PARALLEL_UNITS; do
          RESULT=$(./Needle/needle_threads.out $L $LEN $N $T | grep -E "Estimated|Difference|Error|Time")
          EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
          DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
          ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
          TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
          echo "Needle,Hilos,$N,$T,$L,$LEN,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
        done
      done

      # Processes
      for ((run=1; run<=$NUM_RUNS; run++)); do
        for P in $PARALLEL_UNITS; do
          RESULT=$(./Needle/needle_processes.out $L $LEN $N $P | grep -E "Estimated|Difference|Error|Time")
          EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
          DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
          ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
          TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
          echo "Needle,Procesos,$N,$P,$L,$LEN,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
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

    # Sequential
    for ((run=1; run<=$NUM_RUNS; run++)); do
      RESULT=$(./Dartboard/dartboard.out $R $N | grep -E "Estimated|Difference|Error|Time")
      EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
      DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
      ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
      TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
      echo "Dartboard,Secuencial,$N,1,$R,-,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
    done

    # Sequential - O3
    for ((run=1; run<=$NUM_RUNS; run++)); do
      RESULT=$(./Dartboard/dartboard_O3.out $R $N | grep -E "Estimated|Difference|Error|Time")
      EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
      DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
      ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
      TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
      echo "Dartboard,Secuencial_O3,$N,1,$R,-,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
    done

    # Threads
    for ((run=1; run<=$NUM_RUNS; run++)); do
      for T in $PARALLEL_UNITS; do
        RESULT=$(./Dartboard/dartboard_threads.out $R $N $T | grep -E "Estimated|Difference|Error|Time")
        EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
        DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
        ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
        TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
        echo "Dartboard,Hilos,$N,$T,$R,-,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
      done
    done

    # Processes
    for ((run=1; run<=$NUM_RUNS; run++)); do
      for P in $PARALLEL_UNITS; do
        RESULT=$(./Dartboard/dartboard_processes.out $R $N $P | grep -E "Estimated|Difference|Error|Time")
        EST_PI=$(echo "$RESULT" | grep "Estimated" | awk '{print $3}')
        DIFF=$(echo "$RESULT" | grep "Difference" | awk '{print $2}')
        ERROR=$(echo "$RESULT" | grep "Error" | awk '{print $2}' | tr -d '%')
        TIME=$(echo "$RESULT" | grep "Time" | awk '{print $2}')
        echo "Dartboard,Procesos,$N,$P,$R,-,$EST_PI,$DIFF,$ERROR,$TIME" >> $OUTPUT_FILE
      done
    done

  done
done

echo "Tests completed. Results saved in $OUTPUT_FILE"
