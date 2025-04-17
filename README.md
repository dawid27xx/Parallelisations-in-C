# Parallelisations Projects in C

## Summary

This repo contains small C projects exploring parallelisation in finance. Each project uses a different model: OpenMP for multithreading, MPI for distributed processes, and OpenCL for GPU acceleration. The focus is on key concepts like performance, communication, and applying parallel methods to real-world finance problems.

## Contents

### OpenMP

**Parallelised Sharpe Ratio Calculator (OpenMP C Project)**

A simple C program that uses OpenMP to speed up the calculation of rolling Sharpe ratios on sample stock return data. It reads returns from a CSV file, processes them in parallel, and prints out the average and annualised Sharpe ratio. Multiple methods for synchronisation were also considered (critical regions, atomic operations and mutexes).

### OpenMPI

**Monte Carlo Parallel Option Pricing (OpenMPI C Project)**

A parallel C program that estimates the price of a European call option using Monte Carlo simulation and MPI. Each process receives a unique volatility value, performs local simulations, and contributes its result using MPI_Scatter, MPI_Gather, and MPI_Bcast. The project explores how volatility affects option pricing while demonstrating core MPI communication patterns.


**Real-Time Portfolio Monitor with Risk Alerts**
Plan:
Rank 0 simulates a "market feed", sending price updates (random walk)
Other ranks:
Maintain a fake portfolio value based on those prices
Use MPI_Irecv to listen for updates
Use MPI_Test to poll if new data is available
Recompute portfolio value asynchronously
Optionally, use MPI_Win_lock (RMA) to write a risk status flag to a shared buffer on rank 0

### OpenCL
...
