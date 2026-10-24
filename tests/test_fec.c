#include "mlp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(void) {
    const uint8_t *data[3];
    uint8_t d0[] = {1,2,3};
    uint8_t d1[] = {4,5,6};
    uint8_t d2[] = {7,8,9};
    data[0]=d0; data[1]=d1; data[2]=d2;
    size_t lens[3] = {3,3,3};
    uint8_t parity[3];
    xor_parity(data, lens, 3, parity, 3);
    /* recover d1 */
    const uint8_t *recv[3] = {d0, NULL, d2};
    size_t rlens[3] = {3,0,3};
    (void)recv; (void)rlens;
    /* manual recover */
    uint8_t rec[3];
    memcpy(rec, parity, 3);
    for (int j=0;j<3;j++) rec[j] ^= d0[j];
    for (int j=0;j<3;j++) rec[j] ^= d2[j];
    if (rec[0]!=4 || rec[1]!=5 || rec[2]!=6) { printf("recover\n"); return 1; }
    printf("OK\n");
    return 0;
}
