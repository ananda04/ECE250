#include "stdio.h"
#include "stdlib.h"
#include "string.h"
int main(int argc, char const *argv[]){
    int N = atoi(argv[1]);
    int base = 3;
    int seq = 1;
    int ans;
    for(int i = 0; i< N; i++){
        seq = seq*base;
    }
    ans = seq -3;
    printf("%d\n", ans);
    return EXIT_SUCCESS;
}