#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

//DoubleLinkedList (DLL) for LRU
typedef struct _DLLNode{
    int index;
    struct _DLLNode *prev;
    struct _DLLNode *next;
} DLLNode;

DLLNode *DLLhead;
DLLNode *DLLtail;

void add(int index){
    DLLNode *node = (DLLNode*) malloc(sizeof(DLLNode));
    node->index = index;
    DLLhead->next->prev = node;
    node->next = DLLhead->next;
    DLLhead->next = node;
    node->prev = DLLhead;
}

void headify(DLLNode *node){
    if(node->prev == DLLhead);
    else{
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev = DLLhead;
        node->next = DLLhead->next;
        DLLhead->next->prev = node;
        DLLhead->next = node;
    }
}

void insert(int index){
    DLLNode *node = DLLtail->prev;
    node->prev->next = DLLtail;
    DLLtail->prev = node->prev;
    free(node);
    add(index);
}


//Queue for FIFO
typedef struct _Node{
    int index;
    struct _Node *next;
} Node;

Node *head;
Node *tail;

void put(int index){
    Node *node = (Node*) malloc(sizeof(Node));
    node->index = index;
    tail->next = node;
    tail = node;
}

int poll(){
    int index = head->index;
    Node *node = head;
    head = head->next;
    free(node);
    return index;
}



typedef struct _PTE {
    int PFN;
    char VPN[8];
    bool valid;
    bool dirty;
    bool use;
    bool present;
    DLLNode *node;
} PTE;

int decimal(char *str){
    int length = strlen(str);
    int mult = 1;
    int num = 0;
    for(int i = length-1; i >= 0; i--){
        if(str[i] >= '0' && str[i] <= '9'){
            num += (mult*(str[i] - '0'));
        }
        else{
            if(str[i] == 'a'){
                num += (mult*10);
            }
            else if(str[i] == 'b'){
                num += (mult*11);
            }
            else if(str[i] == 'c'){
                num += (mult*12);
            }
            else if(str[i] == 'd'){
                num += (mult*13);
            }
            else if(str[i] == 'e'){
                num += (mult*14);
            }
            else{
                num += (mult*15);
            }
        }
        mult *= 16;
    }
    return num;
}

void extract_VPN(char *str, char *VPN, int *index){
    char str2[11];
    int length = strlen(str);
    int i = 0;
    int x_index = 0;
    while(str[i] != ' '){
        str2[i] = str[i];
        if(str2[i] == 'x') x_index = i+1;
        i++;
    }
    if(i-2 < 4){
        str2[i-3] = '0';
        str2[i-2] = '\0';
        strcpy(VPN, str2);
        *index = 0;
    }
    else{
        char str3[6];
        str2[i-3] = '\0';
        strcpy(VPN, str2);
        int j = 0;
        while(j < i - x_index){
            str3[j] = str2[x_index + j];
            j++;
        }
        str3[j] = '\0';
        *index = decimal(str3);
    }
}

int main(int argc, char* argv[]){
    if(argc < 4){
        printf("Missing arguments\n");
        return 0;
    }
    if(argc > 5){
        printf("Argument limit exceeded\n");
        return 0;
    }
    if(argc == 5 && strcmp(argv[4], "-verbose") != 0){
        printf("Invalid 5th argument\n");
        return 0;
    }
    FILE *file = fopen(argv[1], "r");
    if(file == NULL) {
        printf("File not found\n");
        return 0;
    }
    int size = (int) (pow(2,20));
    PTE *page_table = (PTE*) malloc(size*sizeof(PTE));
    

    int frames = atoi(argv[2]);
    int frames_allocated = 0;

    int memacc = 0;
    int miss = 0;
    int write = 0;
    int drop = 0;

    if(argc == 4){
        if(strcmp(argv[3], "OPT") == 0){
            int *frame = (int*) malloc((frames)*sizeof(int));
            int lines = 0;
            char str[16];
            while(fgets(str, 16, file) != NULL) lines++;
            rewind(file);
            char **VPNS = (char**) malloc(lines*sizeof(char*));
            int *indices = (int*) malloc(lines*sizeof(int));
            char *opts = (char*) malloc(lines*sizeof(char));
            for(int i = 0; i < lines; i++){
                fgets(str, 16, file);
                int str_length = strlen(str);
                if(str[str_length-1] == '\n') str[str_length-1] = '\0';
                char *str1 = (char*) malloc(8*sizeof(char));
                extract_VPN(str, str1, indices+i);
                VPNS[i] = str1;
                opts[i] = str[str_length-2];
            }
            int line = 0;
            while(frames_allocated < frames && line < lines){
                int index = indices[line];
                char *VPN = VPNS[line];
                if(page_table[index].present){
                    if(opts[line] == 'W') page_table[index].dirty = true;
                }
                else{
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = frames_allocated;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(opts[line] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    frame[frames_allocated] = index;
                    miss++;
                    frames_allocated++;
                }
                memacc++;
                line++;
            }
            for(int i = line; i < lines; i++){
                int index = indices[i];
                char *VPN = VPNS[i];
                if(page_table[index].present){
                    if(opts[i] == 'W') page_table[index].dirty = true;
                }
                else{
                    int count = frames;
                    for(int j = i+1; j < lines; j++){
                        if(count == 1) break;
                        int temp_index = indices[j];
                        if(page_table[temp_index].present){
                            if(page_table[temp_index].use);
                            else{
                                page_table[temp_index].use = true;
                                count--;
                            }
                        }
                    }
                    int out_index = -1;
                    int out_frame;
                    for(int j = 0; j < frames; j++){
                        if(page_table[frame[j]].use){
                            page_table[frame[j]].use = false;
                        }
                        else if(out_index == -1){
                            out_index = frame[j];
                            out_frame = j;
                        }
                    }
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = out_frame;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    page_table[out_index].PFN = -1;
                    page_table[out_index].present = false;
                    page_table[out_index].valid = false;
                    if(opts[i] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    frame[out_frame] = index;
                    if(page_table[out_index].dirty){
                        write++;
                        page_table[out_index].dirty = false;
                    }
                    else drop++;
                    miss++;
                }
                memacc++;
            }
        }
        else if(strcmp(argv[3], "FIFO") == 0){
            char str[16];
            if(fgets(str, 16, file) != NULL){
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                strcpy(page_table[index].VPN, VPN);
                page_table[index].PFN = frames_allocated;
                page_table[index].present = true;
                page_table[index].valid = true;
                if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                else page_table[index].dirty = false;
                head = (Node*) malloc(sizeof(Node));
                head->index = index;
                tail = head;
                frames_allocated++;
                memacc++;
                miss++;
                while(frames_allocated < frames){
                    if(fgets(str, 16, file) == NULL) break;
                    if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                    int index;
                    char VPN[8];
                    extract_VPN(str, VPN, &index);
                    if(page_table[index].present){
                        if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    }
                    else{
                        strcpy(page_table[index].VPN, VPN);
                        page_table[index].PFN = frames_allocated;
                        page_table[index].present = true;
                        page_table[index].valid = true;
                        if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                        else page_table[index].dirty = false;
                        put(index);
                        miss++;
                        frames_allocated++;
                    }
                    memacc++;
                }
                while(fgets(str, 16, file) != NULL){
                    if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                    int index;
                    char VPN[8];
                    extract_VPN(str, VPN, &index);
                    if(page_table[index].present){
                        if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    }
                    else{
                        put(index);
                        int out_index = poll();
                        int out_frame = page_table[out_index].PFN;
                        strcpy(page_table[index].VPN, VPN);
                        page_table[index].PFN = out_frame;
                        page_table[index].present = true;
                        page_table[index].valid = true;
                        if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                        else page_table[index].dirty = false;
                        page_table[out_index].PFN = -1;
                        page_table[out_index].present = false;
                        page_table[out_index].valid = false;
                        if(page_table[out_index].dirty){
                            write++;
                            page_table[out_index].dirty = false;
                        }
                        else drop++;
                        miss++;
                    }
                    memacc++;
                }
            }
        }
        else if(strcmp(argv[3], "LRU") == 0){
            DLLhead = (DLLNode*) malloc(sizeof(DLLNode));
            DLLtail = (DLLNode*) malloc(sizeof(DLLNode));
            DLLhead->next = DLLtail;
            DLLtail->prev = DLLhead;

            while(frames_allocated < frames){
                char str[16];
                if(fgets(str, 16, file) == NULL) break;
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    headify(page_table[index].node);
                }
                else{
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = frames_allocated;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    add(index);
                    page_table[index].node = DLLhead->next;
                    miss++;
                    frames_allocated++;
                }
                memacc++;
            }
            char str[16];
            while(fgets(str, 16, file) != NULL){
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    headify(page_table[index].node);
                }
                else{
                    int out_index = DLLtail->prev->index;
                    int out_frame = page_table[out_index].PFN;
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = out_frame;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    page_table[out_index].PFN = -1;
                    page_table[out_index].present = false;
                    page_table[out_index].valid = false;
                    page_table[out_index].node = NULL;
                    insert(index);
                    page_table[index].node = DLLhead->next;
                    if(page_table[out_index].dirty){
                        write++;
                        page_table[out_index].dirty = false;
                    }
                    else drop++;
                    miss++;
                }
                memacc++;
            }
        }
        else if(strcmp(argv[3], "CLOCK") == 0){
            int *frame = (int*) malloc((frames)*sizeof(int));
            char str[16];
            int hand = 0;
            while(frames_allocated < frames){
                if(fgets(str, 16, file) == NULL) break;
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    page_table[index].use = true;
                }
                else{
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = frames_allocated;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    page_table[index].use = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    frame[frames_allocated] = index;
                    miss++;
                    frames_allocated++;
                }
                memacc++;
            }
            while(fgets(str, 16, file) != NULL){
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    page_table[index].use = true;
                }
                else{
                    int out_index;
                    int out_frame;
                    while(1){
                        if(page_table[frame[hand]].use){
                            page_table[frame[hand]].use = false;
                        }
                        else{
                            out_index = frame[hand];
                            out_frame = hand;
                            frame[hand] = index;
                            if(hand == frames-1) hand = 0;
                            else hand++;
                            break;
                        }
                        if(hand == frames-1) hand = 0;
                        else hand++;
                    }
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = out_frame;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    page_table[index].use = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    page_table[out_index].PFN = -1;
                    page_table[out_index].present = false;
                    page_table[out_index].valid = false;
                    page_table[out_index].use = false;
                    if(page_table[out_index].dirty){
                        write++;
                        page_table[out_index].dirty = false;
                    }
                    else drop++;
                    miss++;
                }
                memacc++;
            }
        }
        else if(strcmp(argv[3], "RANDOM") == 0){
            int *frame = (int*) malloc((frames)*sizeof(int));
            while(frames_allocated < frames){
                char str[16];
                if(fgets(str, 16, file) == NULL) break;
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                }
                else{
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = frames_allocated;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    frame[frames_allocated] = index;
                    miss++;
                    frames_allocated++;
                }
                memacc++;
            }
            srand(5635);
            char str[16];
            while(fgets(str, 16, file) != NULL){
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                }
                else{
                    int out_frame = rand()%frames;
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = out_frame;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    int out_index = frame[out_frame];
                    page_table[out_index].PFN = -1;
                    page_table[out_index].present = false;
                    page_table[out_index].valid = false;
                    if(page_table[out_index].dirty){
                        write++;
                        page_table[out_index].dirty = false;
                    }
                    else drop++;
                    frame[out_frame] = index;
                    miss++;
                }
                memacc++;
            }
        }
        else{
            printf("Invalid method\n");
            return 0;
        }
    }

    //Verbose part
    else{
        if(strcmp(argv[3], "OPT") == 0){
            int *frame = (int*) malloc((frames)*sizeof(int));
            int lines = 0;
            char str[16];
            while(fgets(str, 16, file) != NULL) lines++;
            rewind(file);
            char **VPNS = (char**) malloc(lines*sizeof(char*));
            int *indices = (int*) malloc(lines*sizeof(int));
            char *opts = (char*) malloc(lines*sizeof(char));
            for(int i = 0; i < lines; i++){
                fgets(str, 16, file);
                int str_length = strlen(str);
                if(str[str_length-1] == '\n') str[str_length-1] = '\0';
                char *str1 = (char*) malloc(8*sizeof(char));
                extract_VPN(str, str1, indices+i);
                VPNS[i] = str1;
                opts[i] = str[str_length-2];
            }
            int line = 0;
            while(frames_allocated < frames && line < lines){
                int index = indices[line];
                char *VPN = VPNS[line];
                if(page_table[index].present){
                    if(opts[line] == 'W') page_table[index].dirty = true;
                }
                else{
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = frames_allocated;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(opts[line] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    frame[frames_allocated] = index;
                    miss++;
                    frames_allocated++;
                }
                memacc++;
                line++;
            }
            for(int i = line; i < lines; i++){
                int index = indices[i];
                char *VPN = VPNS[i];
                if(page_table[index].present){
                    if(opts[i] == 'W') page_table[index].dirty = true;
                }
                else{
                    int count = frames;
                    for(int j = i+1; j < lines; j++){
                        if(count == 1) break;
                        int temp_index = indices[j];
                        if(page_table[temp_index].present){
                            if(page_table[temp_index].use);
                            else{
                                page_table[temp_index].use = true;
                                count--;
                            }
                        }
                    }
                    int out_index = -1;
                    int out_frame;
                    for(int j = 0; j < frames; j++){
                        if(page_table[frame[j]].use){
                            page_table[frame[j]].use = false;
                        }
                        else if(out_index == -1){
                            out_index = frame[j];
                            out_frame = j;
                        }
                    }
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = out_frame;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    page_table[out_index].PFN = -1;
                    page_table[out_index].present = false;
                    page_table[out_index].valid = false;
                    if(opts[i] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    frame[out_frame] = index;
                    if(page_table[out_index].dirty){
                        write++;
                        page_table[out_index].dirty = false;
                        printf("Page %s was read from disk, page %s was written to the disk.\n", VPN, page_table[out_index].VPN);
                    }
                    else {
                        drop++;
                        printf("Page %s was read from disk, page %s was dropped (it was not dirty).\n", VPN, page_table[out_index].VPN);
                    }
                    miss++;
                }
                memacc++;
            }
        }
        else if(strcmp(argv[3], "FIFO") == 0){
            char str[16];
            if(fgets(str, 16, file) != NULL){
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                strcpy(page_table[index].VPN, VPN);
                page_table[index].PFN = frames_allocated;
                page_table[index].present = true;
                page_table[index].valid = true;
                if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                else page_table[index].dirty = false;
                head = (Node*) malloc(sizeof(Node));
                head->index = index;
                tail = head;
                frames_allocated++;
                memacc++;
                miss++;
                while(frames_allocated < frames){
                    if(fgets(str, 16, file) == NULL) break;
                    if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                    int index;
                    char VPN[8];
                    extract_VPN(str, VPN, &index);
                    if(page_table[index].present){
                        if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    }
                    else{
                        strcpy(page_table[index].VPN, VPN);
                        page_table[index].PFN = frames_allocated;
                        page_table[index].present = true;
                        page_table[index].valid = true;
                        if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                        else page_table[index].dirty = false;
                        put(index);
                        miss++;
                        frames_allocated++;
                    }
                    memacc++;
                }
                while(fgets(str, 16, file) != NULL){
                    if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                    int index;
                    char VPN[8];
                    extract_VPN(str, VPN, &index);
                    if(page_table[index].present){
                        if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    }
                    else{
                        put(index);
                        int out_index = poll();
                        int out_frame = page_table[out_index].PFN;
                        strcpy(page_table[index].VPN, VPN);
                        page_table[index].PFN = out_frame;
                        page_table[index].present = true;
                        page_table[index].valid = true;
                        if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                        else page_table[index].dirty = false;
                        page_table[out_index].PFN = -1;
                        page_table[out_index].present = false;
                        page_table[out_index].valid = false;
                        if(page_table[out_index].dirty){
                            write++;
                            page_table[out_index].dirty = false;
                            printf("Page %s was read from disk, page %s was written to the disk.\n", VPN, page_table[out_index].VPN);
                        }
                        else {
                            drop++;
                            printf("Page %s was read from disk, page %s was dropped (it was not dirty).\n", VPN, page_table[out_index].VPN);
                        }
                        miss++;
                    }
                    memacc++;
                }
            }
        }
        else if(strcmp(argv[3], "LRU") == 0){
            DLLhead = (DLLNode*) malloc(sizeof(DLLNode));
            DLLtail = (DLLNode*) malloc(sizeof(DLLNode));
            DLLhead->next = DLLtail;
            DLLtail->prev = DLLhead;

            while(frames_allocated < frames){
                char str[16];
                if(fgets(str, 16, file) == NULL) break;
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    headify(page_table[index].node);
                }
                else{
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = frames_allocated;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    add(index);
                    page_table[index].node = DLLhead->next;
                    miss++;
                    frames_allocated++;
                }
                memacc++;
            }
            char str[16];
            while(fgets(str, 16, file) != NULL){
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    headify(page_table[index].node);
                }
                else{
                    int out_index = DLLtail->prev->index;
                    int out_frame = page_table[out_index].PFN;
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = out_frame;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    page_table[out_index].PFN = -1;
                    page_table[out_index].present = false;
                    page_table[out_index].valid = false;
                    page_table[out_index].node = NULL;
                    insert(index);
                    page_table[index].node = DLLhead->next;
                    if(page_table[out_index].dirty){
                        write++;
                        page_table[out_index].dirty = false;
                        printf("Page %s was read from disk, page %s was written to the disk.\n", VPN, page_table[out_index].VPN);
                    }
                    else {
                        drop++;
                        printf("Page %s was read from disk, page %s was dropped (it was not dirty).\n", VPN, page_table[out_index].VPN);
                    }
                    miss++;
                }
                memacc++;
            }
        }
        else if(strcmp(argv[3], "CLOCK") == 0){
            int *frame = (int*) malloc((frames)*sizeof(int));
            char str[16];
            int hand = 0;
            while(frames_allocated < frames){
                if(fgets(str, 16, file) == NULL) break;
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    page_table[index].use = true;
                }
                else{
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = frames_allocated;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    page_table[index].use = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    frame[frames_allocated] = index;
                    miss++;
                    frames_allocated++;
                }
                memacc++;
            }
            while(fgets(str, 16, file) != NULL){
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    page_table[index].use = true;
                }
                else{
                    int out_index;
                    int out_frame;
                    while(1){
                        if(page_table[frame[hand]].use){
                            page_table[frame[hand]].use = false;
                        }
                        else{
                            out_index = frame[hand];
                            out_frame = hand;
                            frame[hand] = index;
                            if(hand == frames-1) hand = 0;
                            else hand++;
                            break;
                        }
                        if(hand == frames-1) hand = 0;
                        else hand++;
                    }
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = out_frame;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    page_table[index].use = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    page_table[out_index].PFN = -1;
                    page_table[out_index].present = false;
                    page_table[out_index].valid = false;
                    page_table[out_index].use = false;
                    if(page_table[out_index].dirty){
                        write++;
                        page_table[out_index].dirty = false;
                        printf("Page %s was read from disk, page %s was written to the disk.\n", VPN, page_table[out_index].VPN);
                    }
                    else {
                        drop++;
                        printf("Page %s was read from disk, page %s was dropped (it was not dirty).\n", VPN, page_table[out_index].VPN);
                    }
                    miss++;
                }
                memacc++;
            }
        }
        else if(strcmp(argv[3], "RANDOM") == 0){
            int *frame = (int*) malloc((frames)*sizeof(int));
            while(frames_allocated < frames){
                char str[16];
                if(fgets(str, 16, file) == NULL) break;
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                }
                else{
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = frames_allocated;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    frame[frames_allocated] = index;
                    miss++;
                    frames_allocated++;
                }
                memacc++;
            }
            srand(5635);
            char str[16];
            while(fgets(str, 16, file) != NULL){
                if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
                int index;
                char VPN[8];
                extract_VPN(str, VPN, &index);
                if(page_table[index].present){
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                }
                else{
                    int out_frame = rand()%frames;
                    strcpy(page_table[index].VPN, VPN);
                    page_table[index].PFN = out_frame;
                    page_table[index].present = true;
                    page_table[index].valid = true;
                    if(str[strlen(str)-1] == 'W') page_table[index].dirty = true;
                    else page_table[index].dirty = false;
                    int out_index = frame[out_frame];
                    page_table[out_index].PFN = -1;
                    page_table[out_index].present = false;
                    page_table[out_index].valid = false;
                    if(page_table[out_index].dirty){
                        write++;
                        page_table[out_index].dirty = false;
                        printf("Page %s was read from disk, page %s was written to the disk.\n", VPN, page_table[out_index].VPN);
                    }
                    else {
                        drop++;
                        printf("Page %s was read from disk, page %s was dropped (it was not dirty).\n", VPN, page_table[out_index].VPN);
                    }
                    frame[out_frame] = index;
                    miss++;
                }
                memacc++;
            }
        }
        else{
            printf("Invalid method\n");
            return 0;
        }
    }

    fclose(file);
    printf("Number of memory accesses: %d\n", memacc);
    printf("Number of misses: %d\n", miss);
    printf("Number of writes: %d\n", write);
    printf("Number of drops: %d\n", drop);
    return 0;
}