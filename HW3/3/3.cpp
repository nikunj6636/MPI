#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MASTER_PROCESS 0
#define TAG 1

void compute_prefix_sum(float array[], int n){
    for(int i=1; i<n; i++) array[i] += array[i-1];
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

    int n, input_size; // n is the total size of array for prefix computation, input_size is the size of array 
    float *array = NULL;
    float *prefix_sum = NULL;

    if (rank == MASTER_PROCESS) {
        char *filename = argv[1];
        FILE *fp = freopen(filename, "r", stdin); // redirecting stdin to .txt file
        if (fp == NULL){
            perror("freopen");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }   

        scanf("%d", &n);
        input_size = n;

        while(n % size != 0) n++;
        array = (float*)malloc(n * sizeof(float)); // memory allocation
        prefix_sum = (float*)malloc(n * sizeof(float)); // memory allocation

        for(int i=0; i<input_size; i++) scanf("%f", &array[i]);
        fclose(fp); // close the file
    }

    // start time after taking input
    double start = MPI_Wtime();

    MPI_Bcast(&n, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);

    int chunkSize = n / size;
    float recvBuffer[chunkSize];

    MPI_Scatter(array, chunkSize, MPI_FLOAT, recvBuffer, chunkSize, MPI_FLOAT, MASTER_PROCESS, MPI_COMM_WORLD);
    compute_prefix_sum(recvBuffer, chunkSize); // O(chunkSize)

    float val = 0, sendToken;
    if (rank != 0) MPI_Recv(&val, 1, MPI_FLOAT, rank-1, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    sendToken = val + recvBuffer[chunkSize-1];
    if (rank != size-1) MPI_Send(&sendToken, 1, MPI_FLOAT, rank+1, TAG, MPI_COMM_WORLD);

    for(int i=0; i<chunkSize; i++) recvBuffer[i] += val; // O(chunkSize)
    MPI_Gather(recvBuffer, chunkSize, MPI_FLOAT, prefix_sum, chunkSize, MPI_FLOAT, MASTER_PROCESS, MPI_COMM_WORLD);

    // end before printing output
    double end = MPI_Wtime();

    if (rank == MASTER_PROCESS){
        for(int i=0; i<input_size; i++) printf("%.2f ", prefix_sum[i]);
        printf("\n");
        double elapsed = end - start;
        // printf("Took %lf seconds\n", elapsed); // run for large n to compare time
    }

    // now run experimarnts with number of process and large inputs

    MPI_Finalize();
    return EXIT_SUCCESS;
}