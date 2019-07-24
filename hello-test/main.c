#include <stdio.h>
#include <stdlib.h>

int main() {
   char *c;
   c = malloc( 200 * sizeof(char));
   printf("Size of c: %d bytes \n", sizeof(c));
   return 0;
}