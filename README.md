# Applying HPC Strategies

This project demonstrates the application of High-Performance Computing (HPC) strategies to improve efficiency in matrix multiplication and related computational algorithms. Matrix multiplication serves as an excellent case study for HPC optimization techniques due to its computational intensity and regular memory access patterns.

## Project Overview

The project explores how various HPC optimization strategies can dramatically improve the performance of matrix operations, which are fundamental to:
- Scientific simulations
- Machine learning algorithms
- Computer graphics
- Signal processing
- Computational physics

Starting with a baseline sequential implementation, the project will progressively incorporate advanced HPC techniques to achieve maximum computational efficiency.

## HPC Strategies Implemented

The project demonstrates several key HPC optimization approaches:

- Parallelization
- Distribute Computing

## System Requirements

This project is specifically designed for **Linux** environments and requires:
- GCC compiler (version 7.0+)
- Standard C libraries
- POSIX threads support


## Compilation

To compile the baseline sequential implementation:

```bash
cc matrix_mult.c -o matrix_mult.out 
```

## Execution

### Sequential Version
```bash
./matrix_mult.out <matrix_size>
```

Examples:
```bash
# Sequential multiplication of 1000x1000 matrices
./matrix_mult.out 1000

## Contributing

Contributions to implement new optimization strategies or extend the project to other algorithms are welcome. Please ensure all contributions maintain compatibility with Linux environments.

## License

This project is open source and available under the MIT License.