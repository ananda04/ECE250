#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct Players{
    int stat;
    char player[64];
    struct Players *next;
};

void printList(struct Players* head){
    struct Players* node = head;
    while(node != NULL){
        if(strcmp(head -> player, "DONE") == 0){
            break;
        }
        if(node -> stat <= 0 ){
            printf("%s %d\n", node->player,node->stat);
        }
        else{
            printf("%s +%d\n", node->player,node->stat);
        }
        node = node -> next;
    }
}

void freeList(struct Players* head){
    struct Players* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

bool compare_stat(struct Players* head, struct Players* toInsert){ // what if i made this into a bool 
    // if(head == NULL){
    //     return true;
    // }
    if(head->stat > toInsert->stat){// if bool then this would be considered true 
        return true; // sets to the left 1
    }
    else if(head->stat < toInsert->stat){ // if bool then this would be considered false 
        return false; // set to the right  -1
    }
    else{
        return strcmp(head->player, toInsert->player) > 0;
    }
}


struct Players* insert(struct Players* head, struct Players* toInsert) {
    if(head == NULL || compare_stat(head,toInsert)){ //==1
        toInsert -> next = head;
        head = toInsert;
        return head;
    }

    struct Players* current = head;
        while (current->next != NULL && !compare_stat(current->next,toInsert)) {//== -1
            current = current->next;
        }
        toInsert -> next = current->next;
        current->next = toInsert;
        return head;

}

int main(int argc, char const *argv[]){
    char player[64];
    int par;
    int stat;
    struct Players *head = NULL;

    FILE *file_ptr = fopen(argv[1],"r");
    if(file_ptr == NULL){
        printf("no such file");
        return EXIT_FAILURE;
    }

    fscanf(file_ptr, "%d", &par);

    while(1){
        struct Players *new_player = (struct Players*)malloc(sizeof(struct Players));
        
        fscanf(file_ptr,"%s", player); // scans for player

        if(strcmp(player, "DONE") == 0){ // checks for done 
            free(new_player);
            break;
        }
        
        fscanf(file_ptr, "%d", &stat); // checks for stat

        strcpy(new_player -> player, player); // stores player in new player
        new_player -> stat = stat -par; // stores stat in new player 
        new_player -> next = NULL; // sets end of new player to null

        head = insert(head, new_player);

    }
    printList(head);
    freeList(head);


    fclose(file_ptr);
    return EXIT_SUCCESS;
}