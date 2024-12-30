#include<stdio.h>
#include<stdlib.h>

int main(){
    char *fileName = "input.txt";
    FILE *fp = freopen(fileName, "w", stdout);
    if (fp == NULL) perror("freopen");
    
    int n;
    scanf("%d", &n);
    printf("%d\n", n);
    for(int i=0; i<=n; i++) printf("%d ", rand()%1000);
    fclose(fp);
}