# Implementation du protocole P2P CHORD avec MPI en C.

CHORD est une table de hachage distribuee (DHT).




# Exercice 1 (Recherche d'une clé)

```bash
# Pour compiler, lancer
make DHT

# Pour lancer le programme, lancer
make run
```

# Exercice 2 (Calcul des fingers tables)

## Description

Utilisation de l'algorithme de Hirschberg et Sinclair pour permettre l'élection d'un leader qui va construire en même temps une liste qui contient tous les ID_CHORD.

Puis diffusion de la liste des ID_CHORD, pour que tout le puisse calculer sa finger_table soi même.

```md
#### Variables locales :
idCHORD
rangMPI 
voisin de notre site i : VG, VD
etat : { leader, battu, nsp}
estInitiateur : { oui, non }
nb_in : nombre de IN recu {0, 1, 2}
CHORIDTable : table de (idCHOR, rangMPI)
calculeFinger : { oui, non }
k : numero d'étape (on commence à 0)

Type de messages : {IN, OUT, CALCUL}

##### Si p est Initiateur :
Sp : { Spontanément, une fois }
initier_etape(k) {
    nb_in = 0
    envoyer_VD(<rangMPI,rangMPI,OUT,2^k>)
    envoyer_VG(<rangMPI,rangMPI,OUT,2^k>)
    k++
}

##### Si p est initiateur ou non inititateur : 
Rp : { Un message <emetteur, initiateur, IN, distance, IDCHORD, rangMPI> arrive}
recevoir (<emetteur, initiateur, IN, distance, IDCHORD, rangMPI>) {
    Si (initiateur != myrangMPI){
        Si (emetteur = VD) {
            envoyer_VG(<myrangMPI, initiateur, IN, _, IDCHORD, rangMPI >)
        } Sinon {
            envoyer_VD(<myrangMPI, initiateur, IN, _, IDCHORD, rangMPI >)
        }
    } Sinon {
        nb_in++
        Ajout de (IDCHORD, rangMPI) dans CHORIDTable
        Si (nb_in=2) {
            initier_etape(k)
        }
    }
}


Rp : { Un message <emetteur, initiateur, OUT, distance,  IDCHORD, rangMPI> arrive}
recevoir ( <emetteur, initiateur, OUT, distance,  IDCHORD, rangMPI>) {
    Si (estInitiateur = non ou initiateur > myrangMPI ){
        etat = battu 
        Si (distance > 1) {
            Si (emetteur = VD) {
                envoyer_VG(<myrangMPI, initiateur, OUT, distance - 1,  IDCHORD, rangMPI>)
            } Sinon {
                envoyer_VD(<myrangMPI, initiateur, OUT, distance - 1,  IDCHORD, rangMPI>)
            }
        } Sinon {
            Si (emetteur = VD){
                envoyer_VD(<myrangMPI, initiateur, IN, _, myIDCHORD, myrangMPI>)
            } Sinon {
                envoyer_VG(<myrangMPI, initiateur, IN, _, myIDCHORD, myrangMPI>)
            }
        }
    } Sinon {
        Si (initiateur = myrangMPI) {
            etat = leader
            envoyer_VG(<myrangMPI, CALCUL, CHORIDTable>)
            envoyer_VD(<myrangMPI, CALCUL, CHORIDTable>)
        }
    }
}


Rp : { Un message (<emetteur, CALCUL, CHORIDTable>) arrive}
recevoir (<CALCUL, CHORIDTable>) {
    if (calculeFinger = non ){
        calculeFinger = oui
        Si (emetteur = VD) {
            envoyer_VG(<myrangMPI, CALCUL, CHORIDTable>)
        } Sinon {
           envoyer_VD(<myrangMPI, CALCUL, CHORIDTable>)
        }
        calcul_finger(CHORIDTable)
    }
}
```

Correction de l'algorithme:
- L'algorithme de Hirschberg et Sinclair a été vu en cours et en TD, l'algorithme est correct.
    - Permet de connaitre le leader et la taille de l'anneau
    - ainsi que construire la liste des id_chord
- Diffusion
        - Transmettre aux autres la taille de l'anneau et la liste des id_chord
- Calcul sur chaque processus de sa finger_table

Complexité en messages:
L'algorithme de Hirschberg et Sinclair est en O(nlog(n))
Tour d'anneau O(n) 

Donc on est en O(n + nlog(n))

Code non complet, boucle infini sur l'algorithme Hirschberg et Sinclair.

# Exercice 3

Le processus `newP` ajouté envoie son id CHORD à n'importe quel autre processus déjà présent dans l'anneau puis se met en attente d'un message.

Le processus `p` dans l'anneau qui reçoit le message <newP_id, emetteur, INSERT, initiateur> avec `INSERT` le type du message, avec `initiateur` l'identifiant MPI du processus `p`:
- met à jour sa finger table et sa liste inversep
- regarde si `initiateur` != p.id:
    - cherche où insérer le nouveau processus `newP`, en regardant si `idp` < newP.idp < succ.idp:
        - si la condition est respectée:
            - il envoie le <succNewP, succMPIrank, finger_table, inversep> au nouveau processus `newP`
                - le processus va construire sa finger_table et inversep depuis celui de son prédecesseur et envoie <newP_id, emetteur, INSERT, initiateur> à son successeur avec le même initiateur.
        - sinon:
            - il envoie <newP_id, emetteur, INSERT, initiateur>**TODO** à son successeur `succ`
- sinon s'arrête, l'algorithme est censé être terminée, tout le monde à reçus le nouvelle identifiant CHORD insérée

L'algorithme est censé faire le tour de l'anneau.

Théoriquement, l'algorithme se base sur le fait que seul le prédecesseur et le successeur sont majoritairement impactés dans l'insertion du processus dans l'anneau, les autres processus peuvent en déduire de l'insertion du nouveau processus en connaissant l'id du prédecesseur.

Grâce à la liste inversep, un processus à maintenant la possibilité de voir ce qu'il y'a `derrière` lui.

Le programme ne contient que la liste des inversep en plus, l'implémentation n'a pas été faite
```bash
# Pour compiler, lancer
make DHT3

# Pour lancer le programme, lancer
make run3
```
