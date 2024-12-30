#include <mpi.h>
#include <bits/stdc++.h>
#define ll long long int
#define double long double

using namespace std;
const ll INF = 1e15;

int main(int argc, char* argv[]) {
    
    MPI_Init(&argc, &argv);

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int n, m, mark = 0; // vertices, edges
    vector<ll> matrix;

    double start_time = MPI_Wtime();
    if(rank == 0){
        freopen(argv[1], "r", stdin); 
        cin>>n>>m;
        matrix.resize(n*n); 

        for(int i=0; i<n*n; i++){
            int row = i/n;
            if(i == row*n + row) matrix[i] = 0;
            else matrix[i] = INF;
        }

        for (int i = 0; i < m; ++i) {
            // assuming directed edges and '0' indexing of nodes
            ll a, b, w; cin>>a>>b>>w;
            matrix[a*n + b] = min(matrix[a*n + b], w); 
        }

        // for(auto x: matrix) cout<<x<<" "; cout<<endl;

    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int rows_per_process = n / p;
    int extra = n%p; 
    int sr = rows_per_process*rank + min(rank, extra); 
    int er = sr + rows_per_process + (rank < extra? 0: -1); 
    int crows = (er-sr+1); 

    vector<int> send_counts(p), displs(p); 
    int sum = 0;
    for(int i=0; i<p; i++){
        int sri = rows_per_process*i + min(i, extra); 
        int eri = sri + rows_per_process + (i < extra? 0: -1);
        send_counts[i] = (eri-sri+1)*n; 
        displs[i] = sum; 
        sum += send_counts[i]; 
    }

    vector<ll> local_matrix(send_counts[rank]);
    MPI_Scatterv(matrix.data(), send_counts.data(), displs.data(), MPI_LONG_LONG_INT, local_matrix.data(), send_counts[rank], MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);

    // Floyd-Warshalls Algorithm
    for(int k=0; k<n; k++){
        // find owner of the process k 
        int owner = (k < extra*(rows_per_process+1)) ? k/(rows_per_process+1): ((k-extra*(rows_per_process+1))/(rows_per_process))+extra;
        vector<ll> row_k(n); 
        if(rank == owner){
            int local_row = k - sr;
            for(int i=0; i<n; i++) {
                row_k[i] = local_matrix[local_row*n + i]; 
            }
        }

        MPI_Barrier(MPI_COMM_WORLD); // blocks all MPI Process until they call this routine 
        MPI_Bcast(row_k.data(), n, MPI_LONG_LONG_INT, owner, MPI_COMM_WORLD); 
        MPI_Barrier(MPI_COMM_WORLD);

        for(int i=0; i<crows; i++){
            for(int j=0; j<n; j++){
                if(local_matrix[i*n+k] != INF && row_k[j] != INF && local_matrix[i*n + j] > local_matrix[i*n+k] + row_k[j]){
                    local_matrix[i*n + j] = local_matrix[i*n + k] + row_k[j];
                }
            }
        }
    }

    // for detecting negative cycles
    for(int k=0; k<n; k++){
        int owner = (k < extra*(rows_per_process+1)) ? k/(rows_per_process+1): ((k-extra*(rows_per_process+1))/(rows_per_process))+extra;
        vector<ll> row_k(n); 
        if(rank == owner){
            int local_row = k - sr;
            for(int i=0; i<n; i++) {
                row_k[i] = local_matrix[local_row*n + i]; 
            }
        }

        MPI_Barrier(MPI_COMM_WORLD); // blocks all MPI Process until they call this routine 
        MPI_Bcast(row_k.data(), n, MPI_LONG_LONG_INT, owner, MPI_COMM_WORLD); 
        MPI_Barrier(MPI_COMM_WORLD);

        for(int i=0; i<crows; i++){
            for(int j=0; j<n; j++){
                if(local_matrix[i*n+k] != INF && row_k[j] != INF && local_matrix[i*n + j] > local_matrix[i*n+k] + row_k[j]){
                    mark = 1;
                    break;
                }
            }
            if(mark == 1) {
                break;
            }
        }
        if(mark == 1){
            MPI_Bcast(&mark, 1, MPI_INT, 0, MPI_COMM_WORLD);
            break;
        }
    }

    MPI_Gatherv(local_matrix.data(), send_counts[rank], MPI_LONG_LONG_INT, matrix.data(), send_counts.data(), displs.data(), MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        if(mark == 0){
            for(int i = 0; i < n; i++) {
                for(int j = 0; j < n; j++) {
                    if(matrix[i*n + j] == INF) cout<<"NP "; // i.e no path
                    else cout<<matrix[i*n + j]<<" "; 
                }
                cout<<endl;
            }
        } else {
            cout<<"Negative Cycle found!"<<endl;
        }

        double end_time = MPI_Wtime();
        double elapsed_time = end_time - start_time;
        cout<<"Time taken: " << elapsed_time << " seconds."<<endl;
    }

    MPI_Finalize();
    return 0;
}