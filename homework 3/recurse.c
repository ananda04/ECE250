#include "stdio.h"
#include "stdlib.h"
#include "string.h"
 int recurse(int N){
    if (N == 0) {
        return 2;  // Base case 1
    } 
    else {
        return 2*(N+1)+ 3*recurse(N - 1) - 17; 
    }
 }
 
 int main(int argc, char *argv[]){
    int N = atoi(argv[1]);
    int ans;
    ans = recurse(N);
    printf("%d\n", ans);
 }