#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int val;
    struct node *next;
} Node;

Node *head;

void createList1(){
    int i = 1;
    Node n;
    n.val = 1;
    *head = n;
    Node *curr = head;
    for(i = 2; i <= 100; i++){
        Node new;
        new.val = i;
        *(curr->next) = new;
        curr = curr->next;
    }
}
int main(){
    Node *curr = head;
    while(curr != NULL){
        printf("%d\n",