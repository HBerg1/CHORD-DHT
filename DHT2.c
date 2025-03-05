#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h> 
#include "node.h"

#define NB_SITE 10
#define M 8


/* types de message */
#define IN 1
#define OUT 2
#define CALCUL 3
#define ELU 5

#define TAGINIT 20

/* etat */
#define NSP 0
#define LEADER 1
#define BATTU 2

/* TYPE INITIATEUR */
#define INITIATEUR 1
#define NON_INITIATEUR 0


// Variables
// int nb_proc;
int idchord;
int rangMPI;
int VG, VD;
int etat = NSP;
int initiateur = 0;
int nb_in = 0;
node* CHORIDTable = NULL;
int calculeFinger = 0;
int k = 0;
int leader;
int message[4]; // <initiateur, distance, IDCHORD, rangMPI>
MPI_Status status;

// Calcule a à la puisssance n
int puissance(int a, int n){
   int res = 1;
   for(int i = 0; i < n; i++){
      res*=a;
   }
   return res;
}

void initier_etape(int k) {
    nb_in = 0;
    message[0] = rangMPI;
    message[1] = puissance(2,k);
    message[2] = idchord;
    message[3] = rangMPI;
    MPI_Send(message, 4, MPI_INT, VG, OUT, MPI_COMM_WORLD);
    MPI_Send(message, 4, MPI_INT, VD, OUT, MPI_COMM_WORLD);
    k++;
}

// <initiateur, distance, IDCHORD, rangMPI>
void recevoirOUT(int *recu){
    int emetteur = status.MPI_SOURCE;

    if(!initiateur || recu[0] >rangMPI){
        etat = BATTU;
        if(recu[1] > 1){
            recu[1] = recu[1]-1;
            if(emetteur == VD){
                MPI_Send(recu, 4, MPI_INT, VG, OUT, MPI_COMM_WORLD);
            }else{
                MPI_Send(recu, 4, MPI_INT, VD, OUT, MPI_COMM_WORLD);
            }
        } else {
            if(emetteur == VD){
                
                MPI_Send(recu, 4, MPI_INT, VD, IN, MPI_COMM_WORLD);
            }else{
                MPI_Send(recu, 4, MPI_INT, VG, IN, MPI_COMM_WORLD);
            }
        }
    } else {
        if(initiateur==rangMPI){
            etat=LEADER;
            // message[0] = rangMPI;
            // message[1] = puissance(2,k);
            // message[2] = idchord;
            // message[3] = rangMPI;
            // MPI_Send(message, 4, MPI_INT, (rangMPI+1)%NB_SITE, LEADER, MPI_COMM_WORLD);
            printf("JE SUIS LEADER\n");
            printf("%d > JE SUIS LEADER %d est LEADER\n", rangMPI, leader);

        }
    }
}


// <initiateur, distance, IDCHORD, rangMPI>
void recevoirIN(int* recu) {
    if (recu[0] != rangMPI) {
        if (recu[0] != VD) {
            MPI_Send(recu, 4, MPI_INT, VG, IN, MPI_COMM_WORLD);
        } else {
            MPI_Send(recu, 4, MPI_INT, VD, IN, MPI_COMM_WORLD);
        }
    } else {
        nb_in ++;
        // CHORIDTable =  (int *) realloc(CHORIDTable, k * sizeof(int));
        if(nb_in == 2){
            initier_etape(k);
        }
    }
}



void proc(int rang){
    rangMPI = rang;
    VD = rang == 1 ? NB_SITE : rang - 1;
    VG = rang == NB_SITE ? 1 : rang + 1;
    int cont_CS=0;

    MPI_Recv(&idchord, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
    
    printf("%d > Mes voisins VG: %d et VD: %d\n", rangMPI, VG, VD);
    initiateur = rang%2;
    if(initiateur==INITIATEUR) {
        printf("%d > je suis initiateur\n", rangMPI);
        
        // message[4]; // <initiateur, distance, IDCHORD, rangMPI>

        // message[0] = 
        // message[1] = puissance(2,k);
        // message[2] = 
        // message[3] = 

        initier_etape(0);
    }

    do {
        // printf("%d > 1\n", rangMPI);
        MPI_Recv(message, 4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        // printf("%d > 2\n", rangMPI);
        if(status.MPI_TAG==OUT){
            // printf("%d > 2\n", rangMPI);
            recevoirOUT(message);
        } else if(status.MPI_TAG==IN){
            // printf("%d > 3\n", rangMPI);
            recevoirIN(message);
        } else if(status.MPI_TAG==ELU) {
            // printf("%d > 4\n", rangMPI);
            leader = message[0]; 
            printf("%d > %d est LEADER\n", rangMPI, leader);
            MPI_Send(message, 4, MPI_INT, (rangMPI+1)%NB_SITE, ELU, MPI_COMM_WORLD);
        }
        // printf("%d > 5\n", rangMPI);

        
        // if(status.MPI_TAG==ELEC){
        //     if(jeton==rangMPI){
        //         etat = LEADER;
        //         MPI_Send(&leader, 1, MPI_INT, (rangMPI+1)%NB_SITE, LEADER, MPI_COMM_WORLD);
        //         continue;
        //     }
        //     // je suis non initiateur ou jeton > rangMPI
        //     if(initiateur==NON_INITIATEUR){
        //         leader = jeton;
        //         etat = BATTU;
        //         MPI_Send(&jeton, 1, MPI_INT, (rangMPI+1)%NB_SITE, ELEC, MPI_COMM_WORLD);
        //     } else {
        //         if( jeton > rangMPI) {
        //             leader = jeton;
        //             etat = BATTU;
        //             MPI_Send(&jeton, 1, MPI_INT, (rangMPI+1)%NB_SITE, ELEC, MPI_COMM_WORLD);
        //         } else {
        //             printf("%d > message detruit de l'rangMPI %d\n", rangMPI, jeton);
        //         }
        //     }
        // }
        // else if(status.MPI_TAG==LEADER) {
        //     printf("%d > %d est LEADER\n", rangMPI, leader);
        //     MPI_Send(&jeton, 1, MPI_INT, (rangMPI+1)%NB_SITE, LEADER, MPI_COMM_WORLD);
        // }
    } while(status.MPI_TAG!=ELU);

    printf("%d > Je suis fini\n", rangMPI);
}


       
void simulateur(void) {
   srand(time(NULL));


   //Création des paires de manière aléatoire
   // Génération aléatoire des identifiants des pairs
   // Avec vérifications des doublons
   int peers[NB_SITE];
   for(int i =0; i<NB_SITE; i++){
      int id = rand()%(int)puissance(2, M);
      for(int j = 0; j<i;j++){
         if(id == peers[j]){
            j = 0;
            id = rand()%(int)puissance(2, M);
         }
      }
      peers[i] = id;
   }

   // Faire attention aux indices, rang et indice sont décalés de 1
   for(int i=1; i<=NB_SITE; i++){
      MPI_Send(&peers[i-1], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);  
   }


   // int p = ID_EXAMPLE;
   // int k = KEY_EXAMPLE;
   
//    int p = peers[(rand()%NB_SITE)+1];
//    int k = rand()%puissance(2,M);

//    printf("\nExemple du avec P%d qui recherche %d\n", p, k);
//    MPI_Send(&k,1,MPI_INT, idpToRank(p, peers), LOOKUP, MPI_COMM_WORLD);

//    // FIN DU PROGRAMME
//    int terminated=0;
//    while(!terminated){
//       MPI_Recv(&terminated, 1, MPI_INT, MPI_ANY_SOURCE, TERMINATED, MPI_COMM_WORLD, &status);
//    }   
//    printf("Simulateur annoce la fin du programme\n");
//    for(int i=1; i<=NB_SITE; i++){
//       MPI_Send(&terminated, 1, MPI_INT, i, TERMINATED, MPI_COMM_WORLD);    
//    }
   printf("Simulteur terminée\n");
}


int main (int argc, char* argv[]) {
   int nb_proc,rang;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

   if (nb_proc != NB_SITE+1) {
      printf("Nombre de processus incorrect !\n");
      printf("Nécessite %d processus Sinon modifier la constante NB_SITE dans le programme\n", NB_SITE+1);
      MPI_Finalize();
      exit(2);
   }
  
   MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  
   if (rang == 0) {
      simulateur();
   } else {
      proc(rang);
   }
  
   MPI_Finalize();
   return 0;
}