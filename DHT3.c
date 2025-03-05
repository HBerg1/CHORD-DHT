#include <mpi.h>
#include <stdio.h>
#include <time.h>
// #include <math.h>
#include <unistd.h>

#include "node.h"


/* DEFINIR LES DONNEES POUR UN EXEMPLE*/
#define ID_EXAMPLE 7
#define KEY_EXAMPLE 0

//TAG
#define TAGINIT    0
#define RESPONSABLE 1
#define LOOKUP 2
#define SUCCESS 3
#define FAILURE 4
#define TERMINATED 5


#define NB_SITE 10
#define M 6


MPI_Status status;
int idp;
int fingerp[M];
int peers[NB_SITE];
int succp;
int rang;
node* list = NULL;

/* UTILS */
// Permet de comparer deux int
int compare_function(const void *a,const void *b) {
   int *x = (int *) a;
   int *y = (int *) b;
   return *x - *y;
}

// Calcule a à la puisssance n
int puissance(int a, int n){
   int res = 1;
   for(int i = 0; i < n; i++){
      res*=a;
   }
   return res;
}

// Affiche un tableau d'entiers
void print_array(int* tab, int len){
   printf("[");
   for(int i =0;i<len; i++){
      printf(" %d ", tab[i]);
   }
   printf("]\n");
}

/* UTILS FIN */

// Vérifie si k est appartient à [a,b[ circulairement
int appIncluExclu(int k, int a, int b){
   if(a==b) return 0;
   k = (k%(int)puissance(2, M));
   if(a<b){
      return a<=k && k<b;
   }
   return a<=k || k<b;
}

// Vérifie si k est appartient à ]a,idp]
// à utiliser avec le finger table;
// teste si k appartient à  
int appExcluInclu(int k, int a, int idp){
   if(a==idp) return 0;
   k = (k%(int)puissance(2, M));
   if(a < idp){
      return a<k && k<=idp;
   }
   return a<k || k<=idp;
}

int findnext(int k){
   // printf("P%d cherche le plus grand finger tq %d appartient à ]j, %d]\n", idp, k, idp);
   int i;
   for(i = M-1; i > 0; i--){
      if(appExcluInclu(k,fingerp[i], idp)){
         return fingerp[i];
      }
   }
   return -1;
}


// Renvoie le rang du processus CHORD d'idp
int idpToRank(int idp, int *peers){
   for(int i = 0; i<NB_SITE; i++){
      if(idp == peers[i]){
         return i+1; // +1 car 0 est réservé au simulateur
      }
   }
   return -1;
}

void lookup(int k, int p){
   printf("P%d recherche la clé %d\n", idp, k);
   sleep(1);
   int next = findnext(k);
   if(next == -1){
      // send(<responsable, k>) to fingerp[0] (successeur)

      printf("P%d notifie P%d qu'il est responsable\n", idp, fingerp[0]);
      sleep(1);
      MPI_Send(&k,1,MPI_INT, idpToRank(fingerp[0], peers), RESPONSABLE, MPI_COMM_WORLD);
   } else {
      // send(<lookup,k,p>) to next
      printf("P%d forward la requête à P%d\n", idp, next);
      sleep(1);
      MPI_Send(&k,1,MPI_INT, idpToRank(next, peers), LOOKUP, MPI_COMM_WORLD);

      // Pas nécessaire car le procesus envoie son rang
      // Donc il peut directement accéder à son identifiant CHORD
      // MPI_Send(&p,1,MPI_INT, next, LOOKUP, MPI_COMM_WORLD); 
   }
}

void initiate_lookup(int k){
   lookup(k,idp);
}

void simulateur(void) {
   srand(time(NULL));


   //Création des paires de manière aléatoire
   // TODO: Pour l'instant j'utilise les données du TD pour vérfier plus facilement l'algo
   //    Décommenter les lignes d'en dessous
   

   // Génération aléatoire des identifiants des pairs
   // Avec vérifications des doublons
   // int peers[NB_SITE];
   // for(int i =0; i<NB_SITE; i++){
   //    int id = rand()%(int)puissance(2, M);
   //    for(int j = 0; j<i;j++){
   //       if(id == peers[j]){
   //          j = 0;
   //          id = rand()%(int)puissance(2, M);
   //       }
   //    }
   //    peers[i] = id;
   // }

   int peers[NB_SITE] = {2, 7, 13, 14, 21 ,38, 42, 48, 51, 59};
   printf("peers: ");
   print_array(peers, NB_SITE);

   // Tri du tableau pour l'utiliser plus facilement
   // Ex: pointeur sur un élément, on peut passer au suivant avec pointeur+1
   // Pb: Check quand c'est à la fin pour revenir au début
   qsort(peers, NB_SITE, sizeof(int), compare_function);

   printf("peers triée: ");
   print_array(peers, NB_SITE);

   // Calcul des fingers tables
   // Hypothèse: On utilise une fonction appIncluExclu(int k, int a, int b) qui permet de vérifier si k est entre [a,b[
   // Algo:
   //    Pour chaque pair:
   //       Pour j de 0 à M-1:
   //          var finger: (clé de la pair)
   //          var succ: (successeur de finger)
   //          var pos: (clé+2^k)%2^M
   //          Tant que pos n'appartient pas à [finger, succ[
   //             finger = succ;
   //             succ = succ(finger)
   //          finger_table[i][j] = succ
   //             avec (i) l'indice de la pair dans le tableau de pairs.

   /* liste des inversep */
   node* inversep[NB_SITE];
   for(int i = 0; i< NB_SITE;i++){
      inversep[i] = NULL;
   }

   /* liste des finger_table */
   int fingers_table[NB_SITE][M];
   for(int i = 0;i < NB_SITE; i++){
      int peer = peers[i];
      for(int j=0; j < M; j++){
         int pos = (peer +(int)puissance(2, j))%(int)puissance(2, M);
         int* finger = (peers+i); 
         int *succ = *finger != peers[NB_SITE-1] ? (finger+1): &peers[0];

         while(!appIncluExclu(pos, *finger, *succ)){
            finger = succ;
            succ = *finger != peers[NB_SITE-1] ? (finger+1): &peers[0];
         }
         fingers_table[i][j] = *succ;
         // printf("peer %d\n", peer);
         printf("succ %d et rank%d\n", *succ, idpToRank(*succ, peers)-1);
         
         // On ajoute dans l'inversep pointé par le paire actuelle sans duplicat
         inversep[idpToRank(*succ, peers)-1] = list_add_without_duplicate(peer, inversep[idpToRank(*succ, peers)-1]);
      }
      // printf("--------------------------\n\n");
   }

/******************************************************************************/
   // Affichage des fingers tables
   // for(int i = 0; i<NB_SITE;i++){
   //     printf("Finger %d :[", peers[i]);
   //     for(int j = 0; j< M;j++){

   //       printf("[%d][%d] =",i, j);
   //       printf("%d ", fingers_table[i][j]);
   //     }
   //     printf("]\n");
   // }

   for(int i = 0; i<NB_SITE;i++){
      printf("Finger %d :", peers[i]);
      print_array(fingers_table[i], M);
   }
   
   printf("\nInversep\n");
   for(int i = 0; i<NB_SITE;i++){
      printf("Inversep %d :", peers[i]);
      print_list(inversep[i]);
      // node* tmp=inversep[i];
      // while(tmp!=NULL){
      //    printf("%d, ",tmp->data);
      //    tmp = tmp->next;
      // }
      // printf("\n");
   }
   
/******************************************************************************/


   // Faire attention aux indices, rang et indice sont décalés de 1
   for(int i=1; i<=NB_SITE; i++){
      MPI_Send(peers, NB_SITE, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(fingers_table[i-1], M, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
   }


   // int p = ID_EXAMPLE;
   // int k = KEY_EXAMPLE;
   
   int p = peers[(rand()%NB_SITE)+1];
   int k = rand()%puissance(2,M);

   printf("\nExemple du avec P%d qui recherche %d\n", p, k);
   MPI_Send(&k,1,MPI_INT, idpToRank(p, peers), LOOKUP, MPI_COMM_WORLD);

   // FIN DU PROGRAMME
   int terminated=0;
   while(!terminated){
      MPI_Recv(&terminated, 1, MPI_INT, MPI_ANY_SOURCE, TERMINATED, MPI_COMM_WORLD, &status);
   }   
   printf("Simulateur annoce la fin du programme\n");
   for(int i=1; i<=NB_SITE; i++){
      MPI_Send(&terminated, 1, MPI_INT, i, TERMINATED, MPI_COMM_WORLD);    
   }
   printf("Simulteur terminée\n");
}

/******************************************************************************/

// Cherche si la clé k 
// appartient à la liste des données
int isData(int k){
   return find_data(k, list);
}


// Fonction d'un processus de l'algorithme
// Boucle tant qu'il n'a pas reçu le message de fin
//    Attends de recevoir un messages de n'importe qui et de tout type
//    Regarde le type de message et l'aiguille
//       - LOOKUP
//       - RESPONSABLE
//       - SUCCESS
//       - FAILURE
//       - TERMINATED
// Pour FAILURE et SUCCESS, je sais pas trop 
// TODO
// TODO
// TODO
// TODO
void calcul_min(int rang){
   rang = rang;
   MPI_Recv(peers, NB_SITE, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);    
   MPI_Recv(fingerp, M, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);    
   
   // printf("P%d je suis de rang %d MPI ", idp, rang);
   // print_array(fingerp, M);
   // printf("\n");
   
   // les rang MPI commencent à 1 donc décalé de +1 avec les indices
   idp = peers[rang-1];
   succp= peers[(rang)%NB_SITE]; 


   


   int terminated = 0;
   int k = 0;

   while(!terminated){
      MPI_Recv(&k, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      int p = peers[status.MPI_SOURCE];    
      switch (status.MPI_TAG) {
         case LOOKUP:
            lookup(k,p);
            break;
         case RESPONSABLE:
            if(isData(k)){
               printf("P%d a trouvé  et est responsable de clé %d\n", idp, k);
               // MPI_Send(&k,1,MPI_INT, p, SUCCESS, MPI_COMM_WORLD);
            } else {
               list = list_add(k, list);
               printf("P%d n'a pas trouvé la clé %d et l'ajoute dans sa liste\n", idp, k);
               // MPI_Send(&k,1,MPI_INT, p, FAILURE, MPI_COMM_WORLD);
            }
            printf("P%d demande au simulateur de terminé\n", idp);
            k = 1;
            MPI_Send(&k,1,MPI_INT, 0, TERMINATED, MPI_COMM_WORLD);
            break;
         case SUCCESS:

            break;
         case FAILURE:
            break;
         case TERMINATED:
            terminated = k;
            free_list(list);
            break;
      }
   }
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
      calcul_min(rang);
   }
  
   MPI_Finalize();
   return 0;
}