/* matr_inv.c : file sorgente di un programma che determina l'inversa
                di una matrice A, utilizzando il metodo di Cramer.
	        La lettura dei valori degli elementi della matrice A
		viene effettuata dal file di input matr_inv.inp
		Inoltre, dopo aver calcolato la matrice inversa (che
		chiamiamo Ainv) i suoi elementi vengono scritti sul
		file di output matr_inv.out
		Infine, come test, si controlla che il prodotto
		matriciale A * Ainv e' uguale all'identita' (a meno
		degli inevitabili errori numerici)
*/

#include<stdio.h>
#include<math.h>
/* E' necessario includere il file stdlib.h, al fine di utilizzare
   la function exit */
#include<stdlib.h>
/* E' necessario includere il file string.h, al fine di utilizzare
   la function strchr */
#include<string.h>

/* La prossima definizione controlla la dimensione massima delle matrici
   che verranno introdotte in seguito */
#define NDIM 20

/* Definizione dei prototipi delle funzioni */
double determinante(double mat[][NDIM], int n);
void inv_mat_cramer(double A[][NDIM], int n, double A_inv[][NDIM]);
void leggi_riga_vet(char riga[], int n, double vet[]);
void prod_matr(double mat1[][NDIM], double mat2[][NDIM], int n,
               double matris[][NDIM]);
void togli_riga0_colonnai(double mat[][NDIM], int n,
                          int i, double matridotta[][NDIM]);


/* ### */
int main()
{
    int n, i, j;
    double A[NDIM][NDIM], Ainv[NDIM][NDIM];
    double A_per_Ainv[NDIM][NDIM], A_per_Ainv_meno_I[NDIM][NDIM];
    double max_A_per_Ainv_meno_I;
    char riga[NDIM*30];
    FILE *ifp, *ofp;

    /* Apro il file di input (che si chiama matr_inv.inp) */
    ifp = fopen("matr_inv.inp" , "r");
    /* Leggo la prima linea del file di input e la assegno alla stringa riga */
    fgets(riga, NDIM*30, ifp);
    /* Dalla stringa riga leggo la dimensione "vera" n dei vettori e delle
       righe e colonne delle matrici */
    sscanf(riga, "%d", &n);
    /* Se la dimensione "vera" n non e' positiva oppure e' maggiore
       di NDIM, arresto immediatamente l'esecuzione del programma,
       utilizzando l'istruzione exit */
    if (n <= 0 || n > NDIM)
        exit(1);
    /* Eseguo un ciclo per i che va da 0 a n-1 */
    for(i=0; i<n; i++) {
        /* Leggo una linea del file di input e la assegno alla stringa riga */
        fgets(riga, NDIM*30, ifp);
        /* Dalla stringa riga leggo e assegno i valori degli elementi
           corrispondenti alla i-esima riga della matrice A, facendo
           ricorso alla function leggi_riga_vet */
        leggi_riga_vet(riga, n, A[i]);
    }
    /* Chiudo il file di input */
    fclose(ifp);

    /* Pongo Ainv uguale alla matrice inversa di A, facendo ricorso alla
       function inv_mat_cramer, che e' basata sul metodo di Cramer */
    inv_mat_cramer(A, n, Ainv);

    /* Apro il file che si chiama "matr_inv.out" in modalita' di
       scrittura (cioe' in output) */
    ofp = fopen("matr_inv.out","w");
    /* Scrivo ordinatamente i valori di tutti gli elementi della matrice
       Ainv sul file di output */
    for(i=0; i<n; i++) {
        fprintf(ofp, "(");
        for(j=0; j<n; j++)
            fprintf(ofp, " %12.9lf", Ainv[i][j]);
        fprintf(ofp, "  )\n");
    }
    /* Chiudo il file di output */
    fclose(ofp);

    /* Calcolo della matrice A_per_Ainv = A * Ainv, facendo ricorso alla
       function prod_matr */
    prod_matr(A, Ainv, n, A_per_Ainv);

    /* Pongo la matrice A_per_Ainv_meno_I uguale alla matrice A_per_Ainv
       meno l'identita' (tale operazione matriciale deve essere svolta,
       SEPARATAMENTE, per ciascun elemento) */
    for(i=0; i<n; i++)
        for(j=0; j<n; j++)
            if(i == j)
                A_per_Ainv_meno_I[i][j] = A_per_Ainv[i][j] - 1;
            else
                A_per_Ainv_meno_I[i][j] = A_per_Ainv[i][j];

    /* Inizialmente, azzero il valore di max_A_per_Ainv_meno_I */
    max_A_per_Ainv_meno_I = 0;
    /* Pongo il valore di max_A_per_Ainv_meno_I uguale al massimo valore
       assoluto di TUTTI gli elementi di A_per_Ainv_meno_I */
    for(i=0; i<n; i++)
        for(j=0; j<n; j++)
            if( max_A_per_Ainv_meno_I < fabs(A_per_Ainv_meno_I[i][j]) )
                max_A_per_Ainv_meno_I = fabs(A_per_Ainv_meno_I[i][j]);

    /* Stampo il massimo valore assoluto di tutti gli elementi di
       A_per_Ainv_meno_I */
    printf("Il prodotto della matrice A per la sua inversa e' uguale\n");
    printf("all'identita' a meno del seguente errore massimo:\n    %le\n",
           max_A_per_Ainv_meno_I);

}


/* ### */
double determinante(double mat[][NDIM], int n)
{
    int i;
    double ris=0., mattmp[NDIM][NDIM];
    /* Questa funzione effettua il calcolo del determinante di una
       matrice nxn-dimensionale utilizzando il metodo (ricorsivo)
       di Laplace */
    /* Se n e' maggiore di 1 ... */
    if(n > 1) {
        /* ... Facciamo un ciclo per i che va da 0 a n-1 */
        for(i=0; i<n; i++) {
            /* Mettiamo nella matrice mattmp una parte della matrice mat,
           che otteniamo da mat stessa togliendo la 0-esima riga e la
           i-esima colonna. Facciamo tutto cio', chiamando la function
           togli_riga0_colonnai */
            togli_riga0_colonnai(mat, n, i, mattmp);
            /* Aggiorniamo il risultato, sommandovi il prodotto tra:
           (1) l'elemento della matrice mat che sta nella 0-esima riga
               e nella i-esima colonna,
           (2) il minore corrispondente, ovvero il determinante della
               matrice (n-1)x(n-1)-dimensionale che e' posta nella matrice
               mattmp,
           (3) il segno che si ottiene elevando -1 alla i    */
            ris += ( 1 - 2 * (i % 2) ) * mat[0][i] * determinante(mattmp, n-1);
        }
    }
        /* Altrimenti ... */
    else {
        /* ... Poniamo il risultato uguale al termine che sta nel vertice
           sinistro in alto della matrice mat */
        ris = mat[0][0];
    }
    /* Restituiamo il valore del risultato alla funzione chiamante */
    return(ris);
}


/* ### */
void inv_mat_cramer(double A[][NDIM], int n, double A_inv[][NDIM])
{
    int i, j, l, m;
    double detA, b[NDIM], mattmp[NDIM][NDIM];
    /* Questa funzione calcola l'inversa della matrice A, utilizzando il
       metodo di Cramer  */
    /* Calcolo del determinante della matrice A, facendo ricorso alla
       function determinante */
    detA = determinante(A, n);
    /* Eseguo un ciclo per i che va da 0 a n-1 */
    for(i=0; i<n; i++) {
        /* Azzero la i-esima componente del "vettore noto" b */
        b[i] = 0.;
    }
    /* Eseguo un ciclo per m che va da 0 a n-1 */
    for(m=0; m<n; m++) {
        /* Pongo la m-esima componente del "vettore noto" b uguale a 1 */
        b[m] = 1.;
        /* Eseguo un ciclo per i che va da 0 a n-1 */
        for(i=0; i<n; i++) {
            /* Eseguo un ciclo per j che va da 0 a n-1 */
            for(j=0; j<n; j++) {
                /* Eseguo un ciclo per l che va da 0 a n-1 */
                for(l=0; l<n; l++) {
                    /* Se i e' uguale a l, poniamo l'elemento che occupa la j-esima
                       riga e la l-esima colonna della matrice mattmp uguale alla
                       j-esima componente del "vettore noto" b ... */
                    if(i == l)
                        mattmp[j][l] = b[j];
                        /* ...Altrimenti, poniamo l'elemento che occupa la j-esima
                           riga e la l-esima colonna della matrice mattmp uguale al
                           corrispondente elemento della matrice A */
                    else
                        mattmp[j][l] = A[j][l];
                }
            }
            /* Poniamo l'elemento che occupa la i-esima riga e la m-esima
           colonna della matrice A_inv uguale al determinante della
           matrice mattmp diviso per il determinante di A */
            A_inv[i][m] = determinante(mattmp, n) / detA;
        }
        /* Azzero la m-esima componente del "vettore noto" b */
        b[m] = 0.;
    }
}


/* ### */
void leggi_riga_vet(riga, n, vet)
        char riga[];
        int n;
        double vet[];
/* Nota: questa function riceve in input una riga (letta in
     precedenza) e l'intero n che e' uguale al numero di
     numeri in doppia precisione che dovrebbero essere
     scritti nella riga, ciascuno separato dall'altro da uno
     o piu' ' '. All'inizio della riga ci sono i caratteri
     '( ' e alla fine ' )'.
     Lo scopo della function e' di leggere tutti i numeri
     della riga e di assegnarli ai corrispondenti elementi del
     vettore.
*/
{
    int i;
    char *puntatore;
    /* Inizialmente, pongo puntatore uguale all'indirizzo del primo
       carattere della stringa che si chiama "riga" */
    puntatore = riga;

    for(i=0; i<n; i++) {
        /* A partire dall'indirizzo contenuto in puntatore vado a cercare
           il primo carattere che e' uguale a uno "spazio". Pongo puntatore
           uguale all'indirizzo del primo "spazio" trovato. */
        puntatore = strchr(puntatore, ' ');
        /* Se puntatore e' uguale a NULL significa che lo "spazio" non e'
           stato trovato, ma cio' non e' ammissibile perche' il vettore
           DEVE contenere N elementi. */
        if(puntatore == NULL) {
            printf("  Gli elementi del vettore su questa riga sono meno di %d\n", n);
            /* Arresto forzato dell'esecuzione del programma */
            exit(1);
        }
        /* A partire da puntatore vado all'indirizzo del primo carattere
           diverso da "spazio" trovato e ridefinisco puntatore uguale
           a questo nuovo indirizzo */
        while(*puntatore == ' ')
            puntatore++;
        /* Leggo un numero double nella stringa che si chiama "riga",
           a partire dall'indirizzo contenuto in puntatore.
           Il valore dell'i-esimo elemento del vettore vet viene posto
           uguale proprio al numero che viene letto dalla stringa */
        sscanf(puntatore, "%lf", &vet[i]);
    }

}


/* ### */
void prod_matr(double mat1[][NDIM],double mat2[][NDIM],
               int n, double matris[][NDIM])
{
    int i, j, l;
    /* Questa funzione effettua il calcolo riga per colonna tra la
       matrice mat1 e la matrice mat2 (che sono entrambe
       nxn-dimensionali), il risultato e' posto nella matrice matris  */
    for(i=0;i<n;i++) {
        for(j=0;j<n;j++) {
            matris[i][j] = 0.;
            for(l=0;l<n;l++) {
                matris[i][j] += mat1[i][l] * mat2[l][j];
            }
        }
    }
}


/* ### */
void togli_riga0_colonnai(mat, n, i, matridotta)
        double mat[][NDIM], matridotta[][NDIM];
        int n, i;
{
    int j, l;
    /* Questa funzione isola una parte della matrice mat, che otteniamo
       da mat stessa togliendo la 0-esima riga e la i-esima colonna, il
       risultato e' posto nella matrice matridotta; mat e' da
       considerarsi nxn-dimensionale e matridotta
       (n-1)x(n-1)-dimensionale */
    for(j=1;j<n;j++) {
        for(l=0;l<n;l++) {
            if(l!=i)
                matridotta[j-1][l-(l>i)]=mat[j][l];
        }
    }
}

