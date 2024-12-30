#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MASTER_PROCESS 0
#define TAG 1
#define INF 1e15

int readInputData(char *fileName, int **array, int size, int *input_size)
{
    FILE *fp = freopen(fileName, "r", stdin);
    if (fp == NULL){
        perror("freopen");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }  

    int n;
    scanf("%d", input_size);
    
    n = *input_size;
    while(n%size != 0) n++;
    int *vec = (int*) malloc(sizeof(int) * (n+1));
    *array = vec;

    for(int i=0; i<=n; i++){
        if (i <= *input_size) scanf("%d", &vec[i]);
        else vec[i] = 0;
    }
    fclose(fp);

    // problem reduces to n matrices
    return n;
}

void sendDataToSlave(int n, int array[], int size)
{
    int size_partition = n / size;

    for (int rank = 1; rank < size; rank++) // Note: it is not MPI_scatter
    {
        int startIndex = rank * size_partition;
        int dataSize = n+1 - startIndex; // num of dims = n+1
        MPI_Send(array + startIndex, dataSize, MPI_INT, rank, TAG, MPI_COMM_WORLD);
    }
}

void receiveDataInSlave(int n, int array[], int size, int rank)
{
    int size_partition = n / size;
    int startIndex = rank * size_partition;
    int dataSize = n+1 - startIndex;
    MPI_Recv(array + startIndex, dataSize, MPI_INT, MASTER_PROCESS, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}


int main(int argc, char *argv[]) 
{
    MPI_Init(&argc, &argv);

    int size, rank; // size is the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0 && argc != 2) {
        printf("Error: Add input file absolute path\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }   

    int n, input_size; // number of matrices in multiplication array is of size n+1
    int *array;
    long long **dp;

    // reading input data
    if (rank == MASTER_PROCESS){
        n = readInputData(argv[1], &array, size, &input_size); // memory is allocated here!
    }
    MPI_Bcast(&n, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);

    double start = MPI_Wtime();

    // Initialization
    dp = (long long**) malloc(sizeof(long long*) * n);
    for(int i=0; i<n; i++){
        dp[i] = (long long*) malloc(sizeof(long long) * n);
        dp[i][i] = 0; // base case
    }

    // sending data
    if (rank == MASTER_PROCESS) sendDataToSlave(n, array, size);
    else{
        array = (int*) malloc(sizeof(int) * (n+1));
        receiveDataInSlave(n, array, size, rank);
    }

    // computing dp values...
    int size_partition = n / size;
    for(int len = 2; len <= n; len++) // compute corresponding to a partition
    { 
        int startIndex = rank * size_partition, j;
        for(int i = startIndex; i < startIndex+size_partition; i++){
            int j = i + len - 1;
            if (j >= n) break;

            dp[i][j] = INF;
            for(int k=i; k<j; k++){
                int temp = dp[i][k]+dp[k+1][j]+array[i]*array[k+1]*array[j+1];
                if (temp < dp[i][j]) dp[i][j] = temp;
            }

            for (int k=1; k<=size - rank - 1; k ++){
                int l = i + k * size_partition, r = j + k * size_partition;
                if (r < n) MPI_Recv(&dp[l][r], 1, MPI_LONG_LONG, rank+k, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            for (int k=1; k<=rank; k++) {
                MPI_Send(&dp[i][j], 1, MPI_LONG_LONG, rank-k, TAG, MPI_COMM_WORLD);
            }
        }
        if (j >= n) break;
    }

    double end = MPI_Wtime();
    
    if (rank == MASTER_PROCESS){
        printf("%llu\n",dp[0][input_size-1]); // master process calculates this
        double elapsed = end - start;
        // printf("Took %lf seconds\n", elapsed); // run for large n to compare time
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}