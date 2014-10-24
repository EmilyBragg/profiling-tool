#include <stdio.h>

void main() {
    int l,loop,k,n;
    loop = 100000;
    n = 10000;
    double q;

    double z[n];
    double x[n];
    
    for ( l=0 ; l<loop ; l++ ) {
        q = 0.0;
        for ( k=0 ; k<n ; k++ ) {
            q += z[k]*x[k];
        }
    }

   printf("Done! %f\n",q);
}
