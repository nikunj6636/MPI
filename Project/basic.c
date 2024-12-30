/*
Base code parallelizing Floyd Warshall Algorithm using OpenMPI
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MASTER_PROCESS 0
#define INF 1e15

long long readInputData(char *fileName, long long **arr, long long size, long long *vertices)
{
    FILE *fp = freopen(fileName, "r", stdin);
    if (fp == NULL){
        perror("freopen");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }  

    long long edges; // vertices, edges
    scanf("%lld %lld", vertices, &edges);
    
    long long n = *vertices;
    while(n%size) n++; // to round off the rows partition in distributed algorithm
    long long *dist = (long long*) malloc(sizeof(long long)*n*n);
    *arr = dist;

    for(long long i=0; i<n; i++){
        for(long long j=0; j<n; j++){
            if (i == j) dist[i*n+j] = 0;
            else dist[i*n+j] = INF;
        }
    }

    for(long long i=0; i<edges; i++){
        long long u, v, w;
        scanf("%lld %lld %lld", &u, &v, &w);
        dist[u*n+v] = w;
    }
    fclose(fp);
    
    return n; // problem reduces to n matrices
}

// Implementing distributed all pair shortest path algorithm(Floyd Warshall):

int main(int argc, char* argv[]) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    long long vertices, n;
    long long* arr = NULL;

    if (rank == MASTER_PROCESS){
        n = readInputData(argv[1], &arr, size, &vertices);
    }

    MPI_Bcast(&n, 1, MPI_LONG_LONG_INT, MASTER_PROCESS, MPI_COMM_WORLD);

    long long m = n / size;
    long long * matrix = (long long *) malloc(n*m*sizeof(long long));

    MPI_Scatter(arr, n*m, MPI_LONG_LONG_INT, matrix, n*m, MPI_LONG_LONG_INT, MASTER_PROCESS, MPI_COMM_WORLD);

    /*
        dp[i][j][k] = min(dp[i][j][k-1] + dp[i][k-1][k-1]+dp[k-1][j][k-1]) // considering negative cycles are not present

        // this for loop needs to executed step by step
        for (k = 0; k < V; k++) { // dist[i][j]: can be computed in any random order, hence we wll parallelize this, distributing row computation
            for (i = 0; i < V; i++) {
                for (j = 0; j < V; j++) { // Each step requires it's own data segment in form of dist[i][k] and kth row data in form of dist[k][j] which have to be passed.
                    if (dist[i][j] > (dist[i][k] + dist[k][j])
                        && (dist[k][j] != INF
                            && dist[i][k] != INF))
                        dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    */

    long long* row_k = (long long *)malloc(n*sizeof(long long));

    for(long long k=0; k<n; k++) {
        long long process_rank_row_k = k / m;

        if (rank == process_rank_row_k) {
            long long i = k%m; // row num in process_rank_row_k
            for(long long j=0; j<n; j++)
                row_k[j] = matrix[i*n+j];
        }

        MPI_Bcast(row_k, n, MPI_LONG_LONG_INT, process_rank_row_k, MPI_COMM_WORLD);

        for(long long i=0; i<m; i++){
            for(long long j=0; j<n; j++){
                long long temp = matrix[i*n + k] + row_k[j];
                if (temp < matrix[i*n+j]) matrix[i*n+j] = temp;
            }
        }
    }

    long long* result = rank == MASTER_PROCESS ? (long long *)malloc(n*n*sizeof(long long)) : NULL;

    MPI_Gather(matrix, n*m, MPI_LONG_LONG_INT, result, n*m, MPI_LONG_LONG_INT, MASTER_PROCESS, MPI_COMM_WORLD);

    if (rank == MASTER_PROCESS)
    {
        for(long long i=0; i<vertices; i++){
            for(long long j=0; j<vertices; j++){
                if (result[i*n+j] == INF) printf("INF ");
                else printf("%lld ", result[i*n+j]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}