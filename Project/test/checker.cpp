#include<bits/stdc++.h>
using namespace std;
#define int long long int

const int INF = 1e15;

int32_t main(){
    ios::sync_with_stdio(0);
    cin.tie(0);

    freopen("./test.txt","r",stdin);
    freopen("output.txt","w",stdout);

    int n, m; cin>>n>>m; 
    vector<vector<int>> matrix(n, vector<int> (n, INF)); 

    for(int i=0; i<n; i++) matrix[i][i] = 0; 

    for(int i=0; i<m; i++){
        int a, b, w; cin>>a>>b>>w;
        matrix[a][b] = w;
    }

    for(int k=0; k<n; k++){
        for(int i=0; i<n; i++){
            for(int j=0; j<n; j++){
                if(matrix[i][k] != INF && matrix[k][j] != INF && matrix[i][j] > matrix[i][k] + matrix[k][j]){
                    matrix[i][j] = matrix[i][k] + matrix[k][j]; 
                }
            }
        }
    }

    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            cout<<matrix[i][j]<<" ";
        }
        cout<<endl; 
    }
    cout << endl;

    return 0;
}