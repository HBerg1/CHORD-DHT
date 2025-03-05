#include <stdlib.h>

typedef struct dataNode{
   int data;
   struct dataNode *next;
} node;


// initialise un noeud
node* init_node(int data){
    node* new_node = malloc(sizeof(node));
    new_node->data = data;
    new_node->next = NULL;
    return new_node;
}

// ajoute un noeud en avant de la liste et renvoie la liste résultante
node* list_add(int data, node*list){
    node* node = init_node(data);
    node->next = list;
    return node;
}


// cherche si data est dans la liste
int find_data(int data, node* list){
    node* tmp = list;
    while(tmp!=NULL){
        if(tmp->data == data){
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

// renvoie le noeud qui contient data, NULL sinon
node* getNode(int data, node* list){
    node* tmp = list;
    while(tmp!=NULL){
        if(tmp->data == data){
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

// ajoute data dans la liste si data n'est pas présente dans la liste
node* list_add_without_duplicate(int data, node*list){
    if(find_data(data, list)) return list;
    node* node = init_node(data);
    node->next = list;
    return node;
}

// libère la mémoire de la liste
void free_list(node* list){
    while(list!=NULL){
        node* next = list->next;
        free(list);
        list = next;
    }
}

// Affiche une liste d'entiers
void print_list(node* liste){
   printf("[");
   for(;liste!=NULL; liste=liste->next){
      printf(" %d ", liste->data);
   }
   printf("]\n");
}
