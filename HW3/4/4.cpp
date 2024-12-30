#include <mpi.h>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char *argv[]){
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int n;
    // string filename = argv[1];
    // ifstream input_file(filename);
    if (rank == 0){
        freopen(argv[1], "r", stdin);
        cin >> n;
    }

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int perprocess = n / size;
    vector<int> row_counts(size,perprocess * n);
    vector<int> startpt(size,0);
    int prev=0;
    for (int i = 0; i < size; i++){
        if(i<n%size){
            row_counts[i]+=n;
        }
        startpt[i] = prev +row_counts[i-1];
        prev=startpt[i];
    }

    vector<double> local_arr(row_counts[rank], 0);
    vector<double> local_identity(row_counts[rank], 0);

    if (rank != 0){
        auto ptr=nullptr;
        MPI_Scatterv(ptr, row_counts.data(), startpt.data(), MPI_DOUBLE, local_identity.data(), row_counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Scatterv(ptr, row_counts.data(), startpt.data(), MPI_DOUBLE, local_arr.data(), row_counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    else{
        vector<double> arr(n * n);
        for (int i = 0; i < n * n; i++){
            cin >> arr[i];
        }
        vector<double> identity(n*n,0);
        for(int i=0;i<n;i++){
            identity[i*n+i]=1;
        }
        MPI_Scatterv(identity.data(), row_counts.data(), startpt.data(), MPI_DOUBLE, local_identity.data(), row_counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Scatterv(arr.data(), row_counts.data(), startpt.data(), MPI_DOUBLE, local_arr.data(), row_counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    for (int i = 0; i < n; i++){
        vector<double> pivot_row_identity(n);
        vector<double> pivot_row_arr(n);

        if (rank == i % size){
            double pivot = local_arr[(i / size) * n + i];
            int cnt=0;
            while(cnt<n){
                int idx=(i / size) * n + cnt;
                local_arr[idx] /= pivot;
                local_identity[idx] /= pivot;
                pivot_row_arr[cnt] = local_arr[idx];
                pivot_row_identity[cnt] = local_identity[idx];
                cnt++;
            }
        }

        MPI_Bcast(pivot_row_identity.data(), n, MPI_DOUBLE, i%size, MPI_COMM_WORLD);
        MPI_Bcast(pivot_row_arr.data(), n, MPI_DOUBLE, i%size, MPI_COMM_WORLD);

        int k=0;
        while(k<n+size-1){
            if (k * size + rank != i && k * size + rank < n){
                double factor = local_arr[k * n + i];
                for (int j = 0; j < n; j++){
                    int idx=k * n + j;
                    local_arr[idx] -= factor * pivot_row_arr[j];
                    local_identity[idx] -= factor * pivot_row_identity[j];
                }
            }
            k++;
        }
    }

    vector<double> identity;
    if (rank == 0){
        identity.resize(n * n);
    }

    int val=0;
    MPI_Gatherv(local_identity.data(), row_counts[rank], MPI_DOUBLE, identity.data(), row_counts.data(), startpt.data(), MPI_DOUBLE, val, MPI_COMM_WORLD);

    if (rank == 0){
        // Reorder the rows to the correct global order
        vector<double> ordered_identity(n * n);
        int i=0;
        while(i<size){
            int j=0;
            while(j<row_counts[i]/n){
                for (int k = 0; k < n; k++){
                    ordered_identity[(j * size + i) * n + k] = identity[(startpt[i] / n + j) * n + k];
                }
                j++;
            }
            i++;
        }

        for (int i = 0; i < n; i++){
            for (int j = 0; j < n; j++){
                cout << fixed << setprecision(2) << ordered_identity[i * n + j] << " ";
            }
            cout << endl;
        }
    }

    MPI_Finalize();
    return 0;
}
