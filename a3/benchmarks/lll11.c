#include <stdio.h>

void main() {
    int l,loop,k,n;
    loop = 100000;
    n = 10000;

    double x[n];
    double y[n];
    
    for ( l=0 ; l<loop ; l++ ) {
        x[0] = y[0];
        for ( k=0 ; k<n ; k++ ) {
            x[k] = x[k-1] + y[k];
        }
    }

    printf("Done!\n");
}
