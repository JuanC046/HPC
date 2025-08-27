# High-Performance Matrix Multiplication

This project focuses on implementing and optimizing matrix multiplication algorithms in a High-Performance Computing (HPC) environment. The goal is to demonstrate various performance optimization strategies for matrix operations, which are fundamental to many scientific and engineering applications.

## Project Overview

The core of this project is a matrix multiplication implementation that can be optimized using different HPC strategies such as:
- Parallelization
- Distribute Computing

The baseline is a sequential matrix multiplication algorithm that will serve as a reference for performance comparisons.

## System Requirements

This project is designed to run on **Linux** systems with:
- GCC compiler
- Standard C libraries

## Compilation

To compile the program, use the following command:

```bash
gcc matrix_mult.c -o matrix_mult.out
```

## Execution

Run the program by specifying the matrix size as a command-line argument:

```bash
./matrix_mult.out <matrix_size>
```

Examples:
```bash
# Multiply 100x100 matrices
./matrix_mult.out 100

# Multiply 1000x1000 matrices
./matrix_mult.out 1000

# Multiply 2000x2000 matrices
./matrix_mult.out 2000
```

## Performance Analysis

For performance analysis, you can use Linux tools such as:
- `time` command to measure execution time
- `perf` for detailed performance metrics
- `valgrind` for memory usage analysis

Example:
```bash
time ./matrix_mult.out 1000
perf stat ./matrix_mult.out 1000
```

## Future Enhancements

Future versions of this project will include:
- POSIX threads
- OpenMP parallelization
- Performance comparison across different optimization strategies

## Contributing

Contributions to improve the performance or add new optimization strategies are welcome.

## License

This project is open source and available under the MIT License.