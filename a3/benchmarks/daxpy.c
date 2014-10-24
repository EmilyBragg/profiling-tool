#include <stdio.h>

void main() {
    int l,loop,k,n;
    double q;
    n = 100000;
    q = 10.5;

    double x[n];
    double y[n];
    
    for ( k=0 ; k<n ; k++ ) {
      y[k] = q*x[k] + y[k];
    }

    printf("Done!\n");
}
