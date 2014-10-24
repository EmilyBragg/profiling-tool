#include <stdio.h>

void main() {
    int l,loop,k,n;
    float q;
    n = 100000;
    q = 100.5;

    float x[n];
    float y[n];
    
    for ( k=0 ; k<n ; k++ ) {
      y[k] = q*x[k] + y[k];
    }

    printf("Done!\n");
}
