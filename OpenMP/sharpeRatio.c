#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>        
#include <time.h>
#include <string.h>           
#include <unistd.h>
#include <omp.h>
#include <math.h>

double getMean(double *values, int len);
double getStd(double *values, int len, double mean);

int main( int argc, char **argv )
{
    printf("Program Starting\n");
    int threadsN = omp_get_max_threads();
    printf("Number of threads available: %d\n", threadsN);

    // omp_get_thread_num is only threads used in scope
    // int threadsUsedN = omp_get_thread_num();
    // printf("Number of threads used: %d\n", threadsUsedN);

    int n = 1260;
    double *returns = malloc(n*sizeof(double));

    FILE *fp = fopen("sample_stock_returns.csv", "r");

    if (fp == NULL) {
        perror("Failed to open file");
        free(returns);
        return 1;
    };
    
    char line[128];
    int i = 0;
    fgets(line, sizeof(line), fp);    

    while (fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",");
        token = strtok(NULL, ",");

        if (token) {
            returns[i] = atof(token);
            i++;
        }
    }
    printf("Loaded %d return values.\n", i);

    int windowSize = 30;
    double* sharpeRatios = malloc((n-windowSize)*sizeof(double));

    double start = omp_get_wtime();

    double sharpeSum = 0;

    omp_lock_t myLock;
    omp_init_lock(&myLock);

    #pragma omp parallel for schedule(static) 
    for (int i = 0; i < (n - windowSize); i++) {
        double mean = getMean(&returns[i], windowSize);
        double std = getStd(&returns[i], windowSize, mean);
        double sharpeRatio = (mean - 0.0001) / std;
        sharpeRatios[i] = sharpeRatio;

        // #pragma omp atomic 

        // #pragma omp critical 
        // {
        // sharpeSum += sharpeRatio;
        // }

        // omp_set_lock(&myLock);
        // sharpeSum += sharpeRatio;
        // omp_unset_lock(&myLock);

        sharpeSum += sharpeRatio;

    };

    omp_destroy_lock(&myLock);


    double end = omp_get_wtime();
    printf("Parallel computation time: %.6f seconds\n", end - start);


    int sharpeCount = n - windowSize;
    double meanSharpe = getMean(sharpeRatios, sharpeCount);
    printf("Average daily Sharpe ratio over time: %.4f\n", meanSharpe);
    double annualisedSharpe = meanSharpe * sqrt(252);
    printf("Estimated annualised Sharpe ratio: %.4f\n", annualisedSharpe);



    fclose(fp);
    free(returns);
    return 0;
};

double getMean(double *values, int len) {
    double sum = 0;
    for (int i = 0; i < len; i++) {
        sum += values[i];
    }
    return sum / len;
}

double getStd(double *values, int len, double mean) {
    double temp = 0;
    for (int i = 0; i < len; i++) {
        temp += pow(values[i] - mean, 2);
    }
    return sqrt(temp / len);
}