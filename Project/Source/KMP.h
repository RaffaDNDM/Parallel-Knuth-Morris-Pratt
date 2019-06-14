/**
  @file KMP.h
  @brief Definizione delle funzioni necessarie a KMP
  @author Di Nardo Di Maio Raffaele 1204879
  @author Fabris Cristina 1205722
*/

#ifndef KMP

#define KMP

/**
    @details Implementazione dell'algortimo KMP
    @param text testo in cui cercare il pattern
    @param pattern stringa da cercare
    @param m lunghezza del pattern
    @param fail array precalcolato fail
*/
int findKMP(const char *text, const char *pattern, const int m, const int *fail);

/**
    @details Funzione che computa l'array fail
    @param pattern stringa da cercare nel testo
    @param m lunghezza del pattern
    @param fail array fail da inizializzare
*/
void computeFailKMP(const char * pattern, const int m, int *fail);

#endif
