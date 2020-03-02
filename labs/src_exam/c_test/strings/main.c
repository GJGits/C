
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
     * DEFINISCO UNA STRINGA, UNA STRINGA E UN VETTORE UNIDIMENSIONALE 
     * DI CARATTERI CHE TERMINA CON IL CARATTERE '\0'. IL CARATTERE FINALE
     * DI UNA STRINGA E FONDAMENTALE, SENZA TUTTE LE OPERAZIONI CHE
     * RIGUARDANO UNA STRINGA NON VANNO A BUON FINE. 
     * 
*/

int main(int argc, char const *argv[]) {
    
    /*
        UNA STRINGA VIENE INIZIALIZZATA CON BYTE RANDOM, OGNI POSIZIONE DELL'ARRAY
        E ACCESSIBILE IN QUANTO SI STA SEMPLICEMENTE PUNTANDO AD UN INDIRIZZO IN 
        MEMORIA (PUNTATORE + OFFSET). NON VIENE SEGNALATO ALCUN TIPO DI ERRORE NE
        WORNING.
    */
    char stringa[100];
    char c = stringa[101];
    printf("char 0: %c\n", c);
    printf("Arrivo qui\n");

    /*
        COMPARAZIONE TRA STRINGHE, SE IL BUFFER NON E ABBASTANZA GRANDE SI CREANO
        PROBLEMI INASPETTATI E LA COMPARAZIONE NON FUNZIONA, INOLTRE QUESTO PUO 
        ANCHE CAUSARE UNA TRASFORMAZIONE DELLA STRINGA COMPARATA PORTANDO PROBLEMI
        NELLE FASI SUCCESSIVE DEL CODICE.
    */

   char s1[] = "Ciao";
   char s2[4];
   printf("Inserire la parola: Ciao--->");
   scanf("%s", s2);
   int cmp = strcmp(s1, s2);
   printf("comparison result: %d\n", cmp);
   char s4[] = "Ciao";
   char s3[6];
   printf("Inserire nuovamente la parola: Ciao--->");
   scanf("%s", s3);
   int cmp2 = strcmp(s4, s3);
   printf("comparison result: %d\n", cmp2);
   printf("In questo caso la comparazione va a buon fine grazie alla corretta allocazione del buffer\n");

   /*
        PRELEVARE UNA SOTTO STRINGA DA UNA STRINGA. ANCHE SE E POSSIBILE UTILIZZARE IL METODO
        MEMCPY PER LE STRINGHE CONVIENE UTILIZZARE STRNCPY PERCHE PIU SPECIFICO
   */

  char str1[] = "Ciao mondo come va?";
  char sub[40];
  memcpy(sub, str1 + 5, (strlen(str1) - 5));
  printf("sub: %s\n", sub); 
  
  char str2[] = "Seconda stringa da tagliare";
  char sub2[40];
  strncpy(sub2, str2 + 5, 13);

  /*
        PRELEVARE TOKENS DA UNA STRINGA.
  */

  char original[] = "John,Mayer,25";
  char *tokens[3];
  char *token = strtok(original, ",");
  tokens[0] = token;
  int i = 1;
  while(token != NULL) {
      token = strtok(NULL, ",");
      if (token != NULL) {
          tokens[i] = token;
          i++;
      }
  }

  /* CONVERT A STRING TO INT */

  char str_num[] = "45";
  int num = strtol(str_num, (char **)NULL, 10);

  /* CONVERTIRE UN INTERO A STRINGA */
  int val = 153;
  char str_val[4];
  sprintf(str_val, "%d", val);

  /* VALIDARE UNA REGEX, QUESTO FUNZIONA SOLO SU LINUX E MAC_OS. 
     STATUS DIVENTA 0 SE TROVA UN MATCH, REG_NOMATCH ALTRIMENTI.
  */
  regex_t re;
  const char str[] = "ciao123";
  const char pattern[] = "^[0-9]+$";
  if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) return 0;
  int status = regexec(&re, str, 0, NULL, 0);
  regfree(&re);
  return 0;
}
