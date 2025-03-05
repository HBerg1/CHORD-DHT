#include <stdio.h>
#include <stdlib.h>

#include "node.h"
void main(void){

    node* list= NULL;
    list = list_add_without_duplicate(5, list);
    list = list_add_without_duplicate(5, list);
    list = list_add_without_duplicate(5, list);
    print_list(list);
    // list = list_add(5, list);
    // list = list_add(5, list);
    free_list(list);

}