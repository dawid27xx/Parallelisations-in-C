# Parallelisations Projects in C

## Summary

This repo contains C projects exploring parallelisation in finance. Each project uses a different model: OpenMP for multithreading, MPI for distributed processes, and OpenCL for GPU acceleration. The focus is on key concepts like performance, communication, and applying parallel methods to real-world finance problems.

## Contents

### OpenMP

**Parallelised Sharpe Ratio Calculator**

A simple C program that uses OpenMP to speed up the calculation of rolling Sharpe ratios on sample stock return data. It reads returns from a CSV file, processes them in parallel, and prints out the average and annualised Sharpe ratio. Multiple methods for synchronisation were also considered (critical regions, atomic operations and mutexes).

### OpenMPI

**Monte Carlo Parallel Option Pricing**

A parallel C program that estimates the price of a European call option using Monte Carlo simulation and MPI. Each process receives a unique volatility value, performs local simulations, and contributes its result using MPI_Scatter, MPI_Gather, and MPI_Bcast. The project explores how volatility affects option pricing while demonstrating core MPI communication patterns.


**Distributed Market Simulation with MPI**

A real-time market simulation using MPI, where rank 0 generates stock price updates and broadcasts them to trader ranks. Each trader manages a portfolio, receives prices asynchronously with MPI_Irecv, checks for risk conditions, and recalculates portfolio value. The project demonstrates non-blocking communication and synchronised reporting across distributed processes.



### OpenCL
...
