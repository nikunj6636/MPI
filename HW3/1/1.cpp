#include "mpi.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char *argv[]){

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Comm_size(comm,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if(rank == 0){
        freopen(argv[1],"r",stdin);
        int n,m,k;
        cin>>n>>m>>k;
        vector<float> p(2*n);
        vector<float> q(2*m);
        for(int i=0;i<n;i++){
            cin>>p[2*i]>>p[2*i+1];
        }
        for(int i=0;i<m;i++){
            cin>>q[2*i]>>q[2*i+1];
        }

        // if only one process is spawned
        if(size==1){
            for(int i=0;i<m;i++){
                float x=q[2*i];
                float y=q[2*i+1];
                vector<pair<float,float>> dist(n);
                for(int j=0;j<n;j++){
                    if(p[2*j]==INT_MAX){
                        dist[j].first=INT_MAX;
                        dist[j].second=j;
                        continue;
                    }
                    dist[j].first=sqrt((p[2*j]-x)*(p[2*j]-x)+(p[2*j+1]-y)*(p[2*j+1]-y));
                    dist[j].second=j;
                }
                sort(dist.begin(),dist.end());
                for(int j=0;j<k;j++){
                    cout<<p[2*dist[j].second]<<" "<<p[2*dist[j].second+1]<<endl;
                }
            }
        }else{
            if(n%(size-1)!=0){
                int val=(size-1)-(n%(size-1));
                for(int i=0;i<2*val;i++){
                    p.push_back(INT_MAX);
                }
            }
            int numprocesses=size-1;
            int perprocess=p.size()/(2*numprocesses);
            MPI_Bcast(&perprocess,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(&k,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(&m,1,MPI_INT,0,MPI_COMM_WORLD);
            for(int i=0;i<numprocesses;i++){
                MPI_Send(&p[2*i*perprocess],perprocess*2,MPI_FLOAT,i+1,0,MPI_COMM_WORLD);
            }
            // loop for queries
            for(int i=0;i<m;i++){
                MPI_Bcast(&q[2*i],1,MPI_FLOAT,0,MPI_COMM_WORLD);
                MPI_Bcast(&q[2*i+1],1,MPI_FLOAT,0,MPI_COMM_WORLD);
                vector<float> p_into_k_nn;
                // loop for recieving from each process
                for(int i=0;i<numprocesses;i++){
                    vector<float> knn(2*k);
                    MPI_Recv(&knn[0],2*k,MPI_FLOAT,i+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    for(int j=0;j<2*k;j++){
                        if(knn[j]==INT_MAX){
                            break;
                        }
                        p_into_k_nn.push_back(knn[j]);
                    }
                }
                vector<pair<float,float>> dist(p_into_k_nn.size()/2);
                for(int j=0;j<p_into_k_nn.size()/2;j++){
                    dist[j].first=sqrt((p_into_k_nn[2*j]-q[2*i])*(p_into_k_nn[2*j]-q[2*i])+(p_into_k_nn[2*j+1]-q[2*i+1])*(p_into_k_nn[2*j+1]-q[2*i+1]));
                    dist[j].second=j;
                }
                sort(dist.begin(),dist.end());
                for(int j=0;j<k;j++){
                    cout<<p_into_k_nn[2*dist[j].second]<<" "<<p_into_k_nn[2*dist[j].second+1]<<endl;
                }
            }
        }
    }else{
        int numelems,k,m;
        MPI_Bcast(&numelems,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&k,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&m,1,MPI_INT,0,MPI_COMM_WORLD);
        vector<float> p1(numelems*2);
        MPI_Recv(&p1[0],numelems*2,MPI_FLOAT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        // loop for queries
        for(int i=0;i<m;i++){
            float x,y;
            MPI_Bcast(&x,1,MPI_FLOAT,0,MPI_COMM_WORLD);
            MPI_Bcast(&y,1,MPI_FLOAT,0,MPI_COMM_WORLD);
            // cout<<x<<" "<<y<<endl;
            if(k>=numelems){
                vector<float> knn(2*k);
                for(int i=0;i<numelems;i++){
                    knn[2*i]=p1[2*i];
                    knn[2*i+1]=p1[2*i+1];
                }
                for(int i=numelems;i<k;i++){
                    knn[2*i]=INT_MAX;
                    knn[2*i+1]=INT_MAX;
                }
                MPI_Send(&knn[0],2*k,MPI_FLOAT,0,0,MPI_COMM_WORLD);
            }else{
                vector<pair<float,float>> dist(numelems);
                for(int i=0;i<numelems;i++){
                    if(p1[2*i]==INT_MAX){
                        dist[i].first=INT_MAX;
                        dist[i].second=i;
                        continue;
                    }
                    dist[i].first=sqrt((p1[2*i]-x)*(p1[2*i]-x)+(p1[2*i+1]-y)*(p1[2*i+1]-y));
                    dist[i].second=i;
                }
                sort(dist.begin(),dist.end());
                vector<float> knn(2*k);
                for(int i=0;i<k;i++){
                    knn[2*i]=p1[2*dist[i].second];
                    knn[2*i+1]=p1[2*dist[i].second+1];
                }
                MPI_Send(&knn[0],2*k,MPI_FLOAT,0,0,MPI_COMM_WORLD);
            }
        }
    }
    // printf("I am %d of %d \n", rank, size);
    MPI_Finalize();

    return 0;   
}
