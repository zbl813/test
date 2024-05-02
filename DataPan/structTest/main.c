#include <stdio.h>
typedef struct PDU
{
        int a;
        int b;
        int c;
        int d[];
}PDU;

int main(int argc, char *argv[])
{
    printf("%ld\n",sizeof(PDU));
    PDU *pdu = (PDU *)malloc(sizeof(PDU)+100*sizeof(int));

    printf("%ld\n",sizeof(pdu));
    printf("Hello World!\n");
    return 0;
}
