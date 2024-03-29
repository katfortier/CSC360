#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../disk/disk.c"


typedef unsigned char byte;
//#define DELIM "\t\r\n\a /"
#define BUFSIZE 64

struct node {
    int number;
    struct node* next;
    struct node* prev;
};

struct node** blocklist_head;
struct node** inodelist_head;


// Node Funtions 
struct node* create(int elem);
struct node* begin(struct node* ref);
struct node* end(struct node* ref);
struct node* next(struct node* ref);
struct node* prev(struct node* ref);
int getValue(struct node* ref);
void push_back(struct node** listHead, int newElem);
int pop_front(struct node** listHead);
void erase(struct node* ref);
void clear(struct node** listHead);
void print(struct node** listHead);

// FS Functions 
void initLLFS();
void startLLFS();
void closeLLFS();
byte* init_freeblocks();
byte* init_inodes();
void create_free_blocklist();
void create_free_inodelist();
void set_block(byte* block, int block_num);
void unset_block(byte* block, int block_num);
int get_block(byte* block, int block_num);
int create_directory();
void add_file_to_directory(int parent_inode_num, int child_node_num, char* filename);
int delete_file_from_directory(int parent_inode_num, char* filename);
int search_directory(int parent_inode_num, char* filename);
int create_file();
void delete_file(int inode_num);
void reclaim_block(int block_num);
void reclaim_inode(int inode_num);

//User Input
void parse_arguments(char* user_input);
char **split_line(char* line);
void execute_command(char **args);
void read_file(char **args);
void make_dir(char **args);
void remove_file(char **args);
void make_file(char **args);

// To DO (back end)
void add_to_file();
void write_inode();
void write_block();
// add ref to directory
// close llfs and free lists (add clear() from linked list)


//TO do (user)
void parse_arguments();


/* -------------------Node Structure--------------------------*/
// Code was taken from the doubly linked list code in assignment #1
// and slightly modified to suite this assignment.

struct node* create(int elem) {
    struct node* newNode
        = (struct node*)malloc(sizeof(struct node));
    newNode->number = elem;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

struct node* begin(struct node* ref) {
    if (ref == NULL) return NULL;
    while(ref->prev) {
        ref = prev(ref);
    }
    return ref;
}

struct node* end(struct node* ref) {
    if (ref == NULL) return NULL;
    while(ref->next) {
        ref = next(ref);
    }
    return ref;
}

struct node* next(struct node* ref) {
    if (ref == NULL) return NULL;
    return ref->next;
}

struct node* prev(struct node* ref) {
    if (ref == NULL) return NULL;
    return ref->prev;
}

int getValue(struct node* ref) {
    return ref->number;
}

void push_back(struct node** listHead, int newElem) {
    struct node* ref = *listHead;
    struct node* temp = create(newElem);
    
    if (ref == NULL){
        ref = temp;
        *listHead = ref;
    } else {
        ref = end(ref);
        ref->next = temp;
        temp->prev = ref;
    }
}

int pop_front(struct node** listHead) {
    struct node* ref = *listHead;
    int value = -1;
    if (ref == NULL){
        return value;
    }

    ref = begin(ref);
    if (next(ref) == NULL) {
        value = getValue(ref);
        erase(ref);
        *listHead = NULL;
    } else {
        ref = next(ref);
        value = getValue(prev(ref));
        erase(prev(ref));
        *listHead = ref;
    }
    return value;
}

void erase(struct node* ref) {
    struct node* nx = next(ref);
    struct node* px = prev(ref);

    free(ref);

    if(nx) {
        nx->prev = px;
    }

    if(px) {
        px->next = nx;
    }
}

void  clear(struct node** listHead) {
    int value = pop_front(listHead);
    while (value != -1){
        value = pop_front(listHead);
    }
}

void print(struct node** listHead) {
    if (listHead == NULL) return;
    struct node* ref = *listHead;
    ref = begin(ref);
    if (ref == NULL) return;
    while (ref != NULL) {
	printf("%d ",ref->number);
	ref = next(ref);
    }
    printf("\n");
}

/*--------------------------Init and Start LLFS---------------------------------*/

/* Initializes the LLFS with Superblock, free inode and block vectors
 * creates free inode and block lists, formats disk to all 0's,
 * and creates the root directory inode */
void initLLFS(){
    // initialize disk to all 0's
    initDisk(); 

    // Initialization of Superblock.
    byte* superBlock = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    // magic number 0x17 = 23
    superBlock[3] = 0x17;
    // number of blocks on disk 0x1000 = 4096 blocks
    superBlock[6] = 0x10;
    superBlock[7] = 0x00;
    // number of inodes on disk 0x100 = 256 inodes
    superBlock[10] = 0x01;
    superBlock[11] = 0x00;
    writeBlock(0, superBlock);
    free(superBlock);

    // bit vector for determining which blocks are available.
    // Ignores first 259 blocks
    byte* free_block_vector = init_freeblocks();
    writeBlock(1, free_block_vector);
    free(free_block_vector);

    // bit vector for determining which inodes are available.
    // Ignores blocks after 256
    byte* free_inode_vector = init_inodes();
    writeBlock(2, free_inode_vector);
    free(free_inode_vector);

    startLLFS();

    //create root directory inode
    int root_directory = create_directory();
    char *name = "..";
    add_file_to_directory(root_directory, root_directory, name);

}

// opens LLFS
void startLLFS(){
    create_free_blocklist();
    create_free_inodelist();
}

// free's up memory from the free block and free inode lists.
void closeLLFS(){
    clear(blocklist_head);
    clear(inodelist_head);

    free(blocklist_head);
    free(inodelist_head);
    blocklist_head = NULL;
    inodelist_head = NULL;
}

/*-------------------Init Free block and inode bit vectors-----------------------*/

// Initializes free_block vector. blocks 0-258 are set to 1
byte* init_freeblocks(){
    byte* free_block_vector = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    for (int i = 0; i < BLOCK_SIZE; i++){
        if (i < 32){
            free_block_vector[i] = 0xFF;
        } else if (i == 32){
            free_block_vector[i] = 0xE0;
        } else {
            free_block_vector[i] = 0x0;
        }
    }
    return free_block_vector;
}

// Initializes free_inode vector. blocks 256-> are set to 1.
byte* init_inodes(){
    byte* free_inode_vector = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    for (int i = 0; i < BLOCK_SIZE; i++){
        if (i == 0){
            free_inode_vector[i] = 0xFF;
        }
        else if (i < 256){
            free_inode_vector[i] = 0x00;
        } else {
            free_inode_vector[i] = 0xFF;
        }
    }
    return free_inode_vector;
}

/* -------------------Create free block and inode lists-------------------------*/
void create_free_blocklist(){
    blocklist_head = (struct node**)calloc(1, sizeof(struct node*));
    *blocklist_head = NULL;

    byte* block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(1,block);

    for (int i = 0; i < BLOCK_SIZE; i++){
        if (get_block(block, i) == 0){
             push_back(blocklist_head, i);
        }
    }
    free(block);
}

 // creates a list of all free inodes at program start time
void create_free_inodelist(){
    inodelist_head = (struct node**)calloc(1, sizeof(struct node*));
    *inodelist_head = NULL;
    byte* block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(2, block);

    for (int i = 0; i < BLOCK_SIZE; i++){
        if (block[i] == 0){
            push_back(inodelist_head, i);
        }
    }
    free(block);
}

/* -------------------------Bit vector functions ---------------------------------*/
// sets block value to 1 in bit vector
void set_block(byte* block, int block_num){
    int index = block_num / 8;
    int bit_index = block_num % 8;
    bit_index = 7 - bit_index;
    block[index] |= 1UL << bit_index;
}

// unsets value of block in bit vector
void unset_block(byte* block, int block_num){
    int index = block_num / 8;
    int bit_index = block_num % 8;
    bit_index = 7 - bit_index;
    block[index] &= ~(1UL << bit_index);
}

// gets status of block, either 1 or 0
int get_block(byte* block, int block_num){
    int index = block_num / 8;
    int bit_index = block_num % 8;
    bit_index = 7 - bit_index;
    int bit = (block[index] >> bit_index) & 1U;
    return bit;
}

/* --------------------------Directory Functions------------------------------------*/

int create_directory(){
    int block_num = pop_front(blocklist_head);
    int inode_num = pop_front(inodelist_head);

    // Create Inode
    byte* inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    //size of file
    inode[3] = 32;
    //set flag to be directory
    inode[4] = 0xDD;
    inode[9] = block_num & 0xFF;
    inode[8] = (block_num >> 8) & 0xFF;

    writeBlock((inode_num+2), inode);

    byte* free_inode_vector = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(2, free_inode_vector);
    free_inode_vector[inode_num] = 0xFF;
    writeBlock(2, free_inode_vector);
    free(free_inode_vector);

    // Create reference Block
    byte* block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    block[0] = inode_num;
    strncpy((char*)(block + 1),".", 1);
    writeBlock(block_num, block);
    free(block);

    byte* free_block_vector = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(1, free_block_vector);
    set_block(free_block_vector, block_num);
    writeBlock(1, free_block_vector);
    free(free_block_vector);
    free(inode);

    return inode_num;
}

// adds given file to directory
// Used resource below for calculating int to hex for file size
// https://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c
void add_file_to_directory(int parent_inode_num, int child_node_num, char* filename){
    //get parent inode block
    byte* parent_inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock((parent_inode_num + 2), parent_inode);

    int parent_block_num = ((int)parent_inode[8])*256 + (int)parent_inode[9];

    //get parent block
    byte* parent_block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(parent_block_num, parent_block);

    // add file to directory
    for (int i = 0; i < BLOCK_SIZE; i += 32){
        if (parent_block[i] == 0x00){
            parent_block[i] = child_node_num;
            strncpy((char*)(parent_block + (i+1)), filename, strlen(filename));
            break;
        }
    }

    int dir_size = ((int)parent_inode[3]) + 256*((int)parent_inode[2]) + 256*256*((int)parent_inode[1]);
    dir_size += 32;
    parent_inode[3] = dir_size & 0xFF;
    parent_inode[2] = (dir_size >> 8) & 0xFF;
    parent_inode[1] = (dir_size >> 16) & 0xFF;
    parent_inode[0] = (dir_size >> 24) & 0xFF; 

    writeBlock(parent_block_num, parent_block);
    writeBlock((parent_inode_num + 2), parent_inode);
    free(parent_block);
    free(parent_inode);

}

int delete_file_from_directory(int parent_inode_num, char* filename) {
    byte* parent_inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock((parent_inode_num + 2), parent_inode);


    int filname_len = strlen(filename);
    int file_inode_num = 0;

    // The i and j that lead to the reference of the file to be removed
    int persistent_i = 0;
    int persistent_j = 0;

    byte* block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    // Loop through each non-empty reference block # in the inode for filename
    for (int i = 8; i < BLOCK_SIZE; i += 2) {
        int blocknum = (int)parent_inode[i]*256 + (int)parent_inode[i+1];
        if (blocknum != 0){
            readBlock(blocknum, block);
            // Loop through each block to find filename
            for (int j = 1; j < BLOCK_SIZE; j += 32){
                if (strncmp(filename, (char*)block + j, filname_len) == 0){
                    file_inode_num = block[j-1];
                    persistent_j = j-1;
                    persistent_i = i;
                }
            }
        }
    }
    
    if (file_inode_num == 0){
        return -1;
    }

    byte* file_inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));

    readBlock((file_inode_num + 2), file_inode);
    if (file_inode[4] == 0xFF){
        delete_file(file_inode_num);
    } else if (file_inode[4] == 0xDD){
        int size = ((int)file_inode[3]) + 256*((int)file_inode[2]) + 256*256*((int)file_inode[1]);
        if (size != 64){
            printf("Directory is not empty, cannot be deleted.\n");
        } else {
            delete_file(file_inode_num);
        }
    }

    // remove reference from parent directory
    int parent_block_num = (int)parent_inode[persistent_i]*256 + (int)parent_inode[persistent_i+1];
    readBlock(parent_block_num, block);
    byte* blankline = (byte*)calloc(32, sizeof(byte));
    memcpy(block+persistent_j, blankline, 32);
    free(blankline);
    writeBlock(parent_block_num, block);

    // update parent directory size
    int dir_size = ((int)parent_inode[3]) + 256*((int)parent_inode[2]) + 256*256*((int)parent_inode[1]);
    dir_size -= 32;
    parent_inode[3] = dir_size & 0xFF;
    parent_inode[2] = (dir_size >> 8) & 0xFF;
    parent_inode[1] = (dir_size >> 16) & 0xFF;

    writeBlock((parent_inode_num + 2), parent_inode);


    free(parent_inode);
    free(block);
    free(file_inode);
    return parent_inode_num;
}

//searches from directory for a directory/file name 
// if found, returns inode number, else returns -1
int search_directory(int parent_inode_num, char* filename){
    byte* parent_inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock((parent_inode_num + 2), parent_inode);

    int filname_len = strlen(filename);
    int file_inode_num = 0;

    byte* block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    // Loop through each non-empty reference block # in the inode for filename
    for (int i = 8; i < BLOCK_SIZE; i += 2) {
        int blocknum = (int)parent_inode[i]*256 + (int)parent_inode[i+1];
        if (blocknum != 0){
            readBlock(blocknum, block);
            // Loop through each block to find filename
            for (int j = 1; j < BLOCK_SIZE; j += 32){
                if (strncmp(filename, (char*)block + j, filname_len) == 0){
                    file_inode_num = block[j-1];
                }
            }
        }
    }

    free(parent_inode);
    free(block);
    
    if (file_inode_num == 0){
        return -1;
    }
     
    return file_inode_num;
}
/* -------------------------------File Functions------------------------------------*/

int create_file(byte* contents){
    int inode_num = pop_front(inodelist_head);  
    int file_len = strlen((char*)contents);
    int num_blocks = (file_len / 512) + 1;
    if (file_len % 512 == 0) {
        num_blocks--;
    }

    if (num_blocks > 252) {
        printf("File size is too big!\n");
    }

     // Create Inode
    byte* inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    //size of file
    inode[3] = file_len & 0xFF;
    inode[2] = (file_len >> 8) & 0xFF;
    inode[1] = (file_len >> 16) & 0xFF;
    inode[0] = (file_len >> 24) & 0xFF;
    //set flag to be directory
    inode[4] = 0xFF;


    byte* block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    byte* free_block_vector = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(1, free_block_vector);
    for (int i = 0; i < num_blocks; i++) {
        int block_num = pop_front(blocklist_head);
        //write file to block(s)
        if (file_len < 512) {
            memcpy(block, contents + (i*512), file_len);
        } else {
            memcpy(block, contents + (i*512), 512);
            file_len = file_len - 512;
        }

        //write block number to inode
        inode[2*i+9] = block_num & 0xFF;
        inode[2*i+8] = (block_num >> 8) & 0xFF;

        //write block to disk
        writeBlock(block_num, block);
        //set free-block vector
        set_block(free_block_vector, block_num);
        free(block);
        block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    }
    //write inode to disk
    writeBlock((inode_num+2), inode);
    writeBlock(1, free_block_vector);

    // mark inode as unavailable
    byte* free_inode_vector = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(2, free_inode_vector);
    free_inode_vector[inode_num] = 0xFF;
    writeBlock(2, free_inode_vector);

    free(free_inode_vector);
    free(free_block_vector);
    free(block);
    free(inode);

    return inode_num;
}

void delete_file(int inode_num){
    byte* inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock((inode_num + 2), inode);

    for (int i = 8; i < 512; i += 2){
        int block_num = 256 * (int)inode[i] + (int)inode[i+1];
        if (block_num != 0){
            reclaim_block(block_num);
        }
    }
    reclaim_inode(inode_num);
    free(inode);
}

/* -------------------------------Reclaim Functions---------------------------------*/

// clears block and makes it available
void reclaim_block(int block_num){
    //write over block with 0's
    byte* block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    writeBlock(block_num, block);

    //update free block vector
    byte* free_block_vector = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(1, free_block_vector);
    unset_block(free_block_vector, block_num);
    writeBlock(1, free_block_vector);

    //update free block list
    push_back(blocklist_head, block_num);

    free(block);
    free(free_block_vector);

}

// clears inode and makes it available
void reclaim_inode(int inode_num){
    //write over block with 0's
    byte* inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    writeBlock((inode_num + 2), inode);

    //update free inode vector
    byte* free_inode_vector = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock(2, free_inode_vector);
    free_inode_vector[inode_num] = 0x00;
    writeBlock(2, free_inode_vector);

    //update free inode list
    push_back(inodelist_head, inode_num);
    free(inode);
    free(free_inode_vector);
}

/* -------------------------------User Input Functions---------------------------------*/


void parse_arguments(char* user_input){
    char **args = split_line(user_input);
    execute_command(args);
    free(args);
}

//splits line into an array
// This function was taken from my shell assignment
char **split_line(char* line){
    int bufsize = BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens){
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    const char* delim = " /";
    token = strtok(line, delim);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if (position >= bufsize){
            bufsize += BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens){
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, delim);
    } // end of while loop
    tokens[position] = NULL;
    return tokens;
} 

// takes the tokenized commands from the split_line functions and executes the correct 
// file system function
// This function was taken from my shell assignment
void execute_command(char **args){
    if (args == NULL){
        fprintf(stderr, "Error: NULL arguments\n");
        exit(1);
    } else if (args[0] == NULL) {
        fprintf(stderr, "Error: NULL arguments\n");
        exit(1);
    }

    size_t cmd_len = strlen(args[0]);

    if (strncmp(args[0], "read", cmd_len) == 0) read_file(args);
    if (strncmp(args[0], "mkdir", cmd_len) == 0) make_dir(args);
    if (strncmp(args[0], "rm", cmd_len) == 0) remove_file(args);
    if (strncmp(args[0], "addfile", cmd_len) == 0) make_file(args);

}

// opens a file
void read_file(char **args){
    int child_inode_num = 0;
    int parent_inode_num = 1;
    int i = 1;
    while (args[i] != NULL){
        child_inode_num = search_directory(parent_inode_num, args[i]);
        if (args[i+1] != NULL){
            parent_inode_num = child_inode_num;
        }
        i++;
    } 
    byte* inode = (byte*)calloc(BLOCK_SIZE, sizeof(byte));
    readBlock((child_inode_num + 2), inode); 
    byte* block = (byte*)calloc(BLOCK_SIZE, sizeof(byte));

    for (int i = 8; i < BLOCK_SIZE; i = i + 2){
        int blocknum = (int)inode[i]*256 + (int)inode[i+1];
        if (blocknum != 0){
            readBlock(blocknum, block);
            // Loop through each block and print contents to std.out
            printf("%s", (char*)block);
        }
    } 

    free(block);
    free(inode);
}

// makes a directory
void make_dir(char **args){
    int child_inode_num = 0;
    int parent_inode_num = 1;
    for (int i = 1; args[i] != NULL; i++){
        child_inode_num = search_directory(parent_inode_num, args[i]);
        if (child_inode_num == -1){
            child_inode_num = create_directory();
            add_file_to_directory(parent_inode_num, child_inode_num, args[i]);
            add_file_to_directory(child_inode_num, parent_inode_num, "..");
        } 
        parent_inode_num = child_inode_num;
    }


}

// removes a file, or an empty directory
void remove_file(char **args){
    int child_inode_num = 0;
    int parent_inode_num = 1;
    int i = 1;
    while (args[i] != NULL){
        child_inode_num = search_directory(parent_inode_num, args[i]);
        if (args[i+1] != NULL){
            parent_inode_num = child_inode_num;
        }
        i++;
    }
    delete_file_from_directory(parent_inode_num, args[i-1]);

}

// creates a file
void make_file(char **args){
    int child_inode_num = 0;
    int parent_inode_num = 1;
    int i = 1;
    while (args[i+2] != NULL){
        child_inode_num = search_directory(parent_inode_num, args[i]);
        if (child_inode_num == -1){
            child_inode_num = create_directory();
            add_file_to_directory(parent_inode_num, child_inode_num, args[i]);
            add_file_to_directory(child_inode_num, parent_inode_num, "..");
        } 
        parent_inode_num = child_inode_num;
        i++;
    }

    FILE* f;
    f = fopen(args[i+1], "rb");
    if (f == NULL){
        printf("File not found\n");
        exit(1);
    }
    //https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
    fseek(f, 0L, SEEK_END);
    size_t size_of_file = ftell(f);
    size_of_file = size_of_file + 1;
    rewind(f); 
    char* contents = (char*)calloc(size_of_file, sizeof(char));

    fread(contents, 1, size_of_file, f);
    int file_inode_num = create_file((byte*)contents);
    add_file_to_directory(parent_inode_num, file_inode_num, args[i]);
    free(contents);

}
