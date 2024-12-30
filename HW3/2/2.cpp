#include "mpi.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char *argv[]){

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    int n,m,k;
    double creal,cimg;
    if(rank==0){
        freopen(argv[1],"r",stdin);
        cin>>n>>m>>k>>creal>>cimg;

        vector<vector<pair<int,int>>> points;
        
        for(int i=0;i<n;i++){
            vector<pair<int,int>> temp;
            for(int j=0;j<m;j++){
                temp.push_back({i,j});
            }
            points.push_back(temp);
        }
        int totalpoints=n*m;
        int pointsperprocess=totalpoints/size;
        int extrapts=totalpoints%size;
        MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&m,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&k,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&creal,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
        MPI_Bcast(&cimg,1,MPI_DOUBLE,0,MPI_COMM_WORLD);        
        int start=0;
        // solve for process 0 here
        vector<int> finalans;
        int loop=pointsperprocess;
        if(extrapts>0){
            loop++;
        }
        for(int i=0;i<n;i++){
            for(int j=0;j<m;j++){        
                long double real=-1.5+j*3.0/(m-1);
                long double img=1.5+i*3.0/(n-1); ;
                complex<long double> cnum(real,img);
                complex<long double> cc(creal,cimg);
                bool f=0;
                for(int i=0;i<k;i++){
                    if(norm(cnum)>4){
                        f=1;
                        break;
                    }
                    cnum=(cnum*cnum)+cc;
                }
                if(norm(cnum)>4){
                    f=1;
                }
                if(f){
                    finalans.push_back(0);
                }else{
                    finalans.push_back(1);
                }
                loop--;
                if(loop==0){
                    break;
                }
            }
            if(loop==0){
                break;
            }
        }
        int numpts;
        for(int i=1;i<size;i++){
            numpts=pointsperprocess;
            start+=pointsperprocess;
            if(extrapts>0){
                if(extrapts!=1){
                    numpts++;
                }
                start++;
                extrapts--;
            }
            int r=start/m;
            int c=start%m;
            int startpt[2]={r,c};
            MPI_Send(&numpts,1,MPI_INT,i,0,MPI_COMM_WORLD);
            MPI_Send(startpt,2,MPI_INT,i,0,MPI_COMM_WORLD);
        }
        for(int i=1;i<size;i++){
            int numpts;
            MPI_Recv(&numpts,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            int ans[numpts];
            MPI_Recv(&ans,numpts,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            for(int j=0;j<numpts;j++){
                finalans.push_back(ans[j]);
            }
        }
        int idx=0;
        for(int i=0;i<n;i++){
            for(int j=0;j<m;j++){
                cout<<finalans[idx++]<<" ";
            }
            cout<<endl;
        }
    }else{
        MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&m,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&k,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&creal,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
        MPI_Bcast(&cimg,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
        int numpts;
        MPI_Recv(&numpts,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        int startpt[2];
        MPI_Recv(startpt,2,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        vector<int> ans(numpts,1);
        int r=startpt[0];
        int c=startpt[1];
        int copynumpts=numpts;
        for(int i=0;i<numpts;i++){
            if(c==m){
                c=0;
                r++;
            }
            long double real=-1.5+c*3.0/(m-1);
            long double img=1.5-r*3.0/(n-1);
            complex<long double> cnum(real,img);
            complex<long double> cc(creal,cimg);
            bool f=0;
            for(int i=0;i<k;i++){
                if(norm(cnum)>4){
                    f=1;
                    break;
                }
                cnum=(cnum*cnum)+cc;
            }
            if(norm(cnum)>4){
                f=1;
            }
            if(f){
                ans[i]=0;
            }
            c++;
        }
        MPI_Send(&numpts,1,MPI_INT,0,0,MPI_COMM_WORLD);
        MPI_Send(&ans[0],numpts,MPI_INT,0,0,MPI_COMM_WORLD);
    }
    MPI_Finalize();

    return 0;   
}
