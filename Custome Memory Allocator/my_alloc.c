#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

/*
This Node is for free-list.
It stores the size of the free chunk and a virtual address to the next Node in free-list.
The actual address of the next node is retrieved by using an offset defined in my_init() as 'ptr'.
*/
typedef struct node{
    int size;
    int next;
} Node;

/*
The Header type of data is used to store the size of the allocated space which will be
later used in my_free().
Here, magic number is not defined as my_free() will always be passed a pointer which was defined
in my_alloc() call.
*/
typedef struct header{
    int size;
} Header;

void *ptr0; //Pointer to the beginning of the 4KB space allocated.
void *ptr;  //Pointer from which memory can be allocated.
Node *head; //Head Node of the free-list.

/*
These integer values are placed inside the heap when my_init() is called.
These values are used to quickly display the information in my_heapinfo().
These values are updated during the my_alloc() and my_free() calls.
*/
/*
Before any memory allocation, the free space will be the max size allowed.
*/
int *max_size_allowed;

/*
When my_alloc() successfully allocates some memory on its call, current_size is incremented
by the amount of space allocated.
When my_free() frees some space on its call, current_size is decremented by the amount of freed space.
*/
int *current_size;

/*
This is similar to the current_size except that it is incremented when current_size
is decremented and it is decremented when current size is incremented by the same amount.
*/
int *free_size;

/*
When a chunk is found successfully in free-list during my_alloc() call, it is incremented by 1.
It is decremented by one when my_free() is called.
*/
int *blocks;

/*
We will have to update the value of the smallest available chunk in the following cases only:
1. When some memory from the smallest available chunk is used for allocation
   and the remaining space after adding new free-list node is not 0.
2. When some memory from the smallest available chunk is used for allocation
   and the remaining space after adding new free-list node is 0.
3. When the complete smallest available chunk is used for allocation.
4. When a free space is coalesced with the smallest available chunk.
From the above cases, the whole free-list is traversed in the second, third and fourth cases.
In the first case, since the space which is remaining after allocation will be
smaller than the earlier smallest available chunk, it will be updated after allocation without
traversing the whole free-list.
In the cases apart from the above 4, the smallest available chunk size will not change.
The newSmallestChunk() function is used to find the new smallest available chunk in cases 2, 3 and 4. 
*/
int *smallest_chunk;

/*
We will have to update the value of the largest available chunk in the following cases only:
1. When memory is allocated in the largest available chunk.
2. When a free space is coalesced with the largest available chunk.
From the above cases, the whole free-list is traversed only in the first case.
In the second case, since the coalesced space will have a larger size than the
earlier largest available chunk, it will be updated after coalescing without
traversing the whole free-list.
In the cases apart from the above 2, the largest available chunk size will not change.
The newLargestChunk() function is used to find the new largest available chunk in case 1. 
*/
int *largest_chunk;

/*
Initializes the space where we will be allocating the space as required.
It also initializes the free-list and gives the initial values to the variables
which will be used in my_heapinfo().
*/
int my_init(){
    ptr0 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if(ptr0 == NULL){
        return -1;
    }
    
    ptr = ptr0 + (6*sizeof(int));

    max_size_allowed = (int*) ptr0;
    *max_size_allowed = 4096 - ((6*sizeof(int)) + sizeof(Node));

    current_size = (int*) (max_size_allowed+1);
    *current_size = (6*sizeof(int)) + sizeof(Node);

    free_size = (int*) (current_size+1);
    *free_size = 4096 - ((6*sizeof(int)) + sizeof(Node));

    blocks = (int*) (free_size+1);
    *blocks = 0;

    smallest_chunk = (int*) (blocks+1);
    *smallest_chunk = 4096 - ((6*sizeof(int)) + sizeof(Node));

    largest_chunk = (int*) (smallest_chunk+1);
    *largest_chunk = 4096 - ((6*sizeof(int)) + sizeof(Node));

    Node n;
    n.size = 4096 - ((6*sizeof(int)) + sizeof(Node));
    head = (Node*) ptr;
    *head = n;
    head->next = -1;
    return 0;
}

int newSmallestChunk(){
    if(head == NULL) return 0;
    int size = 4096;
    Node *curr = head;
    while(1){
        if(curr->size < size && curr->size != 0) size = curr->size;
        if(curr->next == -1) break;
        curr = (Node*) (ptr + curr->next);
    }
    if(size == 4096) size = 0;
    return size;
}

int newLargestChunk(){
    if(head == NULL) return 0;
    int size = 0;
    Node *curr = head;
    while(1){
        if(curr->size > size) size = curr->size;
        if(curr->next == -1) break;
        curr = (Node*) (ptr + curr->next);
    }
    return size;
}

/*
my_alloc() uses first-fit strategy. Memory is allocated in the beginning part
of the free space.
Memory is allocated in that free chunk which satisfies the following
(Requested memory space) + (Size of Header) <= (Size of free chunk excluding free-list Node) + (size of free-list Node)
If the space remaining after allocating ((Requested memory space) + (Size of Header)) is less than
the size of free-list Node then remaining space is also given to the currently allocated space.
*/
void* my_alloc(int num){
    if(num == 0) return NULL;
    if(num%8 != 0) {
        return NULL;
    }
    if(ptr0 == NULL){
        if(my_init() == -1){
            return NULL;
        }
    }
    if(head == NULL){
        return NULL;
    }

    int size = num + sizeof(Header);

    if(size > ((*largest_chunk) + sizeof(Node))) return NULL;
    Node *curr = head;
    Node *prev = NULL;
    while(1){
        if(size <= (curr->size + sizeof(Node))) break;
        if(curr->next == -1) return NULL;
        prev = curr;
        curr = (Node*) (ptr + curr->next);
    }

    *blocks = (*blocks) + 1; //The number of blocks is increased by 1 only after enough free space is found.

    /*
    Value used to tell at the end of this function if traversing the whole
    free-list is needed or not for updating the values of largest available
    chunk and smallest available chunk.
    */
    int new_large = 0;
    int new_small = 0;

    void *p1 = (void*) curr;

    if(size == (curr->size + sizeof(Node))){
        if(prev == NULL){
            if(head->next == -1){
                head = NULL;
            }
            else{
                head = (Node*) (ptr + head->next);
            }
        }
        else{
            prev->next = curr->next;
        }
        if(size == *largest_chunk){
            new_large = 1;
        }
        if(size == *smallest_chunk){
            new_small = 1;
        }

        *current_size = (*current_size) + (curr->size);

        *free_size = (*free_size) - (curr->size);
    }
    else if(curr->size - size < 0){
        num = curr->size + sizeof(Node) - sizeof(Header);
        if(prev == NULL){
            if(head->next == -1){
                head = NULL;
            }
            else{
                head = (Node*) (ptr + head->next);
            }
        }
        else{
            prev->next = curr->next;
        }

        if(curr->size == *largest_chunk){
            new_large = 1;
        }
        if(curr->size == *smallest_chunk){
            new_small = 1;
        }

        *current_size = (*current_size) + curr->size;

        *free_size = (*free_size) - curr->size;
    }
    else{
        Node n1 = {curr->size - size, curr->next};

        if(curr->size == *largest_chunk){
            new_large = 1;
        }
        if(curr->size == *smallest_chunk){
            if(n1.size != 0) *smallest_chunk = n1.size;
            else new_small = 1;
        }

        if(prev == NULL){
            head = (Node*) (p1 + size);
            *head = n1;
        }
        else{
            curr = (Node*) (p1 + size);
            *curr = n1;
            prev->next = prev->next + size;
        }

        *current_size = (*current_size) + size;

        *free_size = (*free_size) - size;
    }

    if(new_large == 1) *largest_chunk = newLargestChunk();
    if(new_small == 1) *smallest_chunk = newSmallestChunk();

    Header *h = (Header*) p1;
    Header hd = {num};
    *h = hd;
    void *p2 = (void*) (h+1);
    return p2;
}

/*
The node which is freed is put at the head of the free-list.
The node formed after coalescing is also put at the head with all
cases handled.
*/
void my_free(void *f){
    Header *h = (Header*) (f - sizeof(Header));
    int size = (h->size + sizeof(Header) - sizeof(Node));
    Node *new_node = (Node*) h;
    new_node->size = size;

    int new_small = 0;

    *blocks = (*blocks) - 1;

    if(head == NULL){
        new_node->next = -1;
        head = new_node;

        *current_size = (*current_size) - size;

        *free_size = (*free_size) + size;

        *largest_chunk = head->size;
        *smallest_chunk = head->size;
    }
    else{
        void *begin_new_node = (void*) new_node;
        void *end_new_node = begin_new_node + (new_node->size + sizeof(Node));

        Node *before = NULL;
        Node *before_prev = NULL;

        Node *after = NULL;
        Node *after_prev = NULL;

        Node *curr = head;
        Node *prev = NULL;
        while(1){
            void *begin_curr = (void*) curr;
            void *end_curr = begin_curr + (curr->size + sizeof(Node));

            if(before == NULL && end_curr == begin_new_node){
                before = curr;
                before_prev = prev;
            }
            else if(after == NULL && end_new_node == begin_curr){
                after = curr;
                after_prev = prev;
            }
            else if(before != NULL && after != NULL){
                break;
            }
            if(curr->next == -1){
                break;
            }
            prev = curr;
            curr = (Node*) (ptr + curr->next);
        }

        void *start = (void*) head;
        int address = start - ptr;
        if(before == NULL && after == NULL){
            new_node->next = address;
            head = new_node;

            *current_size = (*current_size) - size;

            *free_size = (*free_size) + size;

            if(new_node->size > *largest_chunk) *largest_chunk = new_node->size;
            if(new_node->size < *smallest_chunk) *smallest_chunk = new_node->size;
        }
        else if(before == NULL){ //free chunk exists only after freed space
            *current_size = (*current_size) - (new_node->size + sizeof(Node));

            *free_size = (*free_size) + (new_node->size + sizeof(Node));

            if(after->size == *smallest_chunk) new_small = 1;

            new_node->size += (after->size + sizeof(Node));

            if(new_node->size > *largest_chunk) *largest_chunk = new_node->size;


            if(after_prev == NULL){
                new_node->next = after->next;
                head = new_node;
            }
            else{
                after_prev->next = after->next;
                new_node->next = address;
                head = new_node;
            }
        }
        else if(after == NULL){ //free chunk exists only before freed space
            *current_size = (*current_size) - (new_node->size + sizeof(Node));

            *free_size = (*free_size) + (new_node->size + sizeof(Node));

            if(before->size == *smallest_chunk) new_small = 1;

            before->size += (new_node->size + sizeof(Node));

            if(before->size > *largest_chunk) *largest_chunk = before->size;

            if(before_prev != NULL){
                before_prev->next = before->next;
                before->next = address;
                head = before;
            }
        }
        else{                   //free chunk exists both before and after freed space
            *current_size = (*current_size) - (new_node->size + 2*sizeof(Node));

            *free_size = (*free_size) + (new_node->size + 2*sizeof(Node));

            if(before->size == *smallest_chunk || after->size == *smallest_chunk) new_small = 1;

            before->size += (new_node->size + after->size + 2*sizeof(Node));

            if(before->size > *largest_chunk) *largest_chunk = before->size;

            if(after_prev == NULL){
                if(before == (Node*) (ptr + after->next)){
                    head = before;
                }
                else{
                    before_prev->next = before->next;
                    before->next = after->next;
                    head = before;
                }
            }
            else if(before_prev == NULL){
                if(after == (Node*) (ptr + before->next)){
                    before->next = after->next;
                }
                else{
                    after_prev->next = after->next;
                }
            }
            else{
                if(before == (Node*) (ptr + after->next)){
                    after_prev->next = before->next;
                }
                else if(after == (Node*) (ptr + before->next)){
                    before_prev->next = after->next;
                }
                else{
                    before_prev->next = before->next;
                    after_prev->next = after->next;
                }
                before->next = address;
                head = before;
            }
        }
    }

    if(new_small == 1) *smallest_chunk = newSmallestChunk();
}

void my_clean(){
    munmap(ptr0, 4096);
}

/*
Method to print the Nodes of free-list with the size of each free chunk and
the address of the next free chunk.
*/
void printList(){
    printf("--------------------------\n");
    if(head == NULL) {

    }
    else{
        Node *curr = head;
        int i = 1;
        while(1){
            printf("%d. Size: %d, Next: %d\n", i, curr->size, curr->next);
            if(curr->next == -1) break;
            i++;
            curr = (Node*) (ptr + curr->next);
        }
    }
    printf("--------------------------\n");
}

/*
When my_heapinfo() is called, we need not traverse the whole free-list to get the information.
The variables stored in the beginning of the initialized address space contain the required
values.
Read lines 29-86 for the complete details of how these variables keep track of the required values.
*/
void my_heapinfo(){
		int a = *max_size_allowed,
        b = *current_size,
        c = *free_size,
        d = *blocks,
        e = *smallest_chunk,
        f = *largest_chunk;

	// Do not edit below output format
	printf("=== Heap Info ================\n");
	printf("Max Size: %d\n", a);
	printf("Current Size: %d\n", b);
	printf("Free Memory: %d\n", c);
	printf("Blocks allocated: %d\n", d);
	printf("Smallest available chunk: %d\n", e);
	printf("Largest available chunk: %d\n", f);
	printf("==============================\n");
	// Do not edit above output format
	return;
}