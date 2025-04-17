#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>        
#include <time.h>
#include <string.h>           
#include <unistd.h>
#include <math.h>

struct OptionsParams
{
    double S;
    double K;
    double r;
    double sig;
    double T;
};

int main(int argc, char** argv) {
    
    int numprocs, rank;

    struct OptionsParams params;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if( (numprocs&(numprocs-1))!=0 || numprocs>32 )
    {
        if( rank==0 ) printf( "ERROR: Launch with a number of processes that is a power of 2 (i.e. 2, 4, 8, ...) and <=32.\n" );

        MPI_Finalize();
        return 1;
    }

    int simulationN; int localSimulationN; double totalSum; double localSum = 0;

    if (rank == 0) {
        totalSum = 0;

        params.S = 100;
        params.K = 100;
        params.r = 0.05;
        params.sig = 0.2;
        params.T = 1.0;

        simulationN = 100000;
        localSimulationN = simulationN / numprocs;
    }

    MPI_Bcast(&params, 5, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&localSimulationN, 1, MPI_INT, 0, MPI_COMM_WORLD);

    srand(rank*time(NULL));
    
    for (int i=0;i<localSimulationN;i++) {
        double u1 = rand() / (double)RAND_MAX;
        double u2 = rand() / (double)RAND_MAX;
        if (u1 == 0) u1 = 1e-10;

        double z = sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);

        double futureStockPrice = params.S * exp((params.r-0.5*pow(params.sig, 2))*params.T + params.sig*sqrt(params.T)*z);

        double payoff = fmax(futureStockPrice - params.K, 0.0);

        localSum += payoff;
    }

    MPI_Reduce(&localSum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double price = exp(-1*params.r*params.T) * (totalSum/simulationN);
        printf("Price: %f", price);
    }

    MPI_Finalize();

    return 0;
};