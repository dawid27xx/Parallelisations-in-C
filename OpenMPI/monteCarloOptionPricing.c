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
    double *sigmas = malloc(numprocs*sizeof(double));
    
    // generate sigma values
    if (rank == 0) {
        for (int i=0; i<numprocs; i++) {
            sigmas[i] = 0.1 + 0.05*i;
        };
    };

    // distribute parameters
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

    // send parameters
    MPI_Bcast(&params, 5, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&localSimulationN, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // update sigma values
    double localSig;
    MPI_Scatter(sigmas, 1, MPI_DOUBLE, &localSig, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    params.sig = localSig;

    // vary the random seed
    srand(rank*time(NULL));
    
    // compute the option pricing
    double payoff;
    for (int i=0;i<localSimulationN;i++) {
        double u1 = rand() / (double)RAND_MAX;
        double u2 = rand() / (double)RAND_MAX;
        if (u1 == 0) u1 = 1e-10;

        double z = sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);

        double futureStockPrice = params.S * exp((params.r-0.5*pow(params.sig, 2))*params.T + params.sig*sqrt(params.T)*z);

        payoff = fmax(futureStockPrice - params.K, 0.0);

        localSum += payoff;
    }

    // receive the total payoff

    // MPI_Reduce(&localSum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double payoffs[numprocs];
    MPI_Gather(&localSum, 1, MPI_DOUBLE, payoffs, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i=0; i<numprocs; i++) {
            totalSum += payoffs[i];
            printf("Sigma: %.2f → Local Price: %.4f\n", sigmas[i], payoffs[i] / localSimulationN * exp(-params.r * params.T));

        }
    };

    double price;

    // calculate price and output
    if (rank == 0) {
        price = exp(-1*params.r*params.T) * (totalSum/simulationN);
        printf("Estimated European Call Option Price: %.4f\n", price);
        printf("Parameters used:\n");
        printf("  Initial Stock Price (S): %.2f\n", params.S);
        printf("  Strike Price (K): %.2f\n", params.K);
        printf("  Risk-free Rate (r): %.2f\n", params.r);
        printf("  Volatility (sigma): %.2f\n", params.sig);
        printf("  Time to Maturity (T): %.2f years\n", params.T);
        printf("  Simulations per Process: %d\n", localSimulationN);
        printf("  Total Simulations: %d\n", localSimulationN * numprocs);
    };

    // send final price to all processes using point-to-point communication 
    if (rank == 0) {
        for (int i=1; i<numprocs; i++) {
            MPI_Send(&price, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        };
    }

    if (rank != 0) {
        MPI_Recv(&price, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, NULL);
    };


    if (rank == 0) free(sigmas);
    MPI_Finalize();

    return 0;
};