#include <stdio.h>
#include <stdlib.h>
#include <rpc/xdr.h>
#include <string.h>

#define BUFSIZE 4000

int main(int argc, char const *argv[])
{
    XDR xdrs;
    /* pointer to XDR stream */
    char buf[BUFSIZE];/* buffer for XDR data */
    /* create stream */
    xdrmem_create(&xdrs, buf, BUFSIZE, XDR_ENCODE);
    /* append data to XDR stream */
    int i=800,j; float a=3.14;
    char *p="ciao mondo";
    j=strlen(p);
    xdr_int(&xdrs, &i); /* convert/append integer i */
    xdr_float(&xdrs,&a);
    xdr_bytes(&xdrs,&p,&j,80); /* convert/append string, max
    len=80 */
    int len = xdr_getpos(&xdrs); /* the amount of buf used*/
   // xdr_destroy(&xdrs); /* destroy (free) stream */
    printf("buf: %s\n", buf);
    return 0;
}
