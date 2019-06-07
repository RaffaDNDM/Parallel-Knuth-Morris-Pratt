#ifndef KMP

#define KMP

/**
    @details Implementazione dell'algortimo KMP
    @param text testo in cui cercare il pattern
    @param pattern stringa da cercare
    @param m lunghezza del pattern
    @param fail array precalcolato fail
*/
int findKMP(const char *text, const char *pattern, int m, const int *fail);

/**
    @details Funzione che computa l'array fail
    @param pattern stringa da cercare nel testo
    @param m lunghezza del pattern
    @param fail array fail da inizializzare
*/
void computeFailKMP(char * pattern, int m, int *fail);

#endif
