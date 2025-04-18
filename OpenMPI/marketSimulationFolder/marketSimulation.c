#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>        
#include <time.h>
#include <unistd.h>

#define FINALTIME 20
#define STARTSHARES 100.0

int main(int argc, char** argv) {
    int numprocs, rank;
    const char* tickers[8] = {"AAPL", "MSFT", "NVDA", "AMZN", "GOOG", "YHOO", "ORCL", "TSLA"};

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    srand(time(NULL) + rank);

    int stockIndexOne, stockIndexTwo; 
    const char* portfolio[2];
    double lastKnownPrices[8]; 

    // Assign stocks from rank 0
    if (rank == 0) {
        printf("Starting Market Simulation. \n");
        for (int i = 1; i < numprocs; i++) {
            stockIndexOne = rand() % 8;
            do {
                stockIndexTwo = rand() % 8;
            } while (stockIndexTwo == stockIndexOne);

            MPI_Send(&stockIndexOne, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&stockIndexTwo, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&stockIndexOne, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&stockIndexTwo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        portfolio[0] = tickers[stockIndexOne];
        portfolio[1] = tickers[stockIndexTwo];

        for (int i = 0; i < 8; i++) {
            lastKnownPrices[i] = 100.0;
        }
        
    }

    double stockUpdates[8];
    double receivedPrices[8];
    MPI_Request request;
    int currentTime = 0;



    // Start main loop
    while (currentTime < FINALTIME) {
        if (rank == 0) {
            if (currentTime == 0) {
                for (int i = 0; i < 8; i++) {
                    stockUpdates[i] = 100.0; 
                }
            } else {
                for (int i = 0; i < 8; i++) {
                    double percentChange = ((rand() % 201) - 100) / 10000.0; 
                    stockUpdates[i] *= (1.0 + percentChange);
                }
            }

            for (int i = 1; i < numprocs; i++) {
                MPI_Isend(stockUpdates, 8, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, MPI_STATUS_IGNORE);
            }

        } else {
            MPI_Irecv(receivedPrices, 8, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &request);

            int flag = 0;
            while (!flag) {
                // Risk check while waiting for new prices
                double riskValue1 = lastKnownPrices[stockIndexOne];
                double riskValue2 = lastKnownPrices[stockIndexTwo];

                if (riskValue1 < 95.0 || riskValue2 < 95.0) {
                    printf("Rank %d | Warning: Potential risk detected! Price drop below 5%% on held stock\n", rank);
                }

                usleep(100000); 

                MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
            }


            for (int i = 0; i < 8; i++) {
                lastKnownPrices[i] = receivedPrices[i];
            }

            

            double value = STARTSHARES * receivedPrices[stockIndexOne] + 
                           STARTSHARES * receivedPrices[stockIndexTwo];

            printf("Rank %d | %s: %.2f, %s: %.2f => Portfolio Value: %.2f\n",
                   rank,
                   portfolio[0], receivedPrices[stockIndexOne],
                   portfolio[1], receivedPrices[stockIndexTwo],
                   value);
        }

        sleep(0.25);
        currentTime++;
    };

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < numprocs; i++) {
        if (rank == i) {
            if (rank == 0) {
                printf("\n--- FINAL STOCK PRICES ---\n");
                for (int j = 0; j < 8; j++) {
                    printf("%s: %.2f\n", tickers[j], stockUpdates[j]);
                }
                printf("--------------------------\n");
            } else {
                double finalValue = STARTSHARES * receivedPrices[stockIndexOne] +
                                    STARTSHARES * receivedPrices[stockIndexTwo];

                printf("\n[Rank %d] Final Portfolio:\n", rank);
                printf("  %s: %.2f x %.2f = %.2f\n",
                    portfolio[0], STARTSHARES, receivedPrices[stockIndexOne],
                    STARTSHARES * receivedPrices[stockIndexOne]);
                printf("  %s: %.2f x %.2f = %.2f\n",
                    portfolio[1], STARTSHARES, receivedPrices[stockIndexTwo],
                    STARTSHARES * receivedPrices[stockIndexTwo]);
                printf("  Total Value: %.2f\n", finalValue);
            }
            fflush(stdout);
        }

        // Sync before allowing the next rank to print
        MPI_Barrier(MPI_COMM_WORLD);
    }


    MPI_Finalize();
    return 0;
}
