/**
  @file KMP.c
  @Implementazione dell'algoritmo KMP sequenziale
  @author Di Nardo Di Maio Raffaele 1204879
  @author Fabris Cristina 1205722
*/

#include <string.h>
#include "KMP.h"

int findKMP(const char *text, const char *pattern, const int m, const int *fail)
{
  int n = strlen(text); /*lunghezza del testo*/
  int j=0; /*indice che scandisce il testo*/
  int k=0; /*indice che scandisce il pattern*/

  while (j<n)
  {
    if(text[j]==pattern[k])
    {
      if(k==m-1) /*se ho raggiunto la fine del pattern restituisco l'indice di inizio dell'occorrenza*/
      {
        return j-m+1;
      }

      j=j+1;
      k=k+1;
    }
    else if (k>0) /*se non ho corrispondenza ma non sono al primo elemento del pattern*/
    {
      k=fail[k-1];
    }
    else /*se sono all'inizio del pattern e non ho corrispondenza*/
      {
        j=j+1;
      }
  }
  return -1;/*non ha trovato occorrenze*/
}

void computeFailKMP(const char * pattern, const int m, int *fail)
{
   int j=1;/*indice che scandisce fail*/
   int k=0;/*indice che scandisce il pattern*/

   while (j<m)
   {
     if (pattern[j]==pattern[k]) /*se presente più volte lo stesso carattere all'interno del pattern*/
     {
       fail[j]=k+1;
       j=j+1;
       k=k+1;
     }
     else if (k>0) /*se non sono all'inizio della scansione del pattern e non è un carattere già presente nel pattern*/
     {
       k=fail[k-1];
     }
     else /*se sono all'inizio della scansione del pattern*/
     {
       j=j+1;
     }
   }
}
