/* Katherine Jacobs
 * V00783178
 * Last Modified: January 15th 2020
 * CSC 360 A1
 *
 *
 * Partial Double linked-list implementation in C11
 * Course CSC 360, University of Vicoria (thank you Amir!)
 *
 * Assignment number 01, Part 1, Jan 2020
 *
 * Use this command to compile the code:
 * gcc -std=c11 -pedantic-errors doubletroublelist.c -o dList
 *
 * Execute the program with command:
 * ./dList
 *
 * Please note: This code is not complete, and the current tests will seg fault until concat is completed!
 *
 * Your task is to complete the swap and shift_left functions
 *
 * Expected output:
 * Intial list: 2 3 4 5 8 11 14 14
 * list size: 8
 * Delete element with value 8: 2 3 4 5 11 14 14
 * Insert 12 after 11 by reverse search: 2 3 4 5 11 12 14 14
 * Swap 3 and 12: 2 12 4 5 11 3 14 14
 * Swap 4 and 5: 2 12 5 4 11 3 14 14
 * Swap again 4 and 5: 2 12 4 5 11 3 14 14
 * Reverse the list: 14 14 3 11 5 4 12 2
 * Push 55 and then reverse the list again: 55 2 12 4 5 11 3 14 14
 * Create another list: 7 15 30 60 120
 * Concatenate two lists: 55 2 12 4 5 11 3 14 14 7 15 30 60 120
 * Revese the new list: 120 60 30 15 7 14 14 3 11 5 4 12 2 55
 * Insert one 3 and two 7 after 15: 120 60 30 15 3 7 7 7 14 14 3 11 5 4 12 2 55
 * Apply unique function to the list: 120 60 30 15 3 7 14 3 11 5 4 12 2 55
 * Shift left 3 times: 15 3 7 14 3 11 5 4 12 2 55
 * Rotate left 2 times: 7 14 3 11 5 4 12 2 55 15 3
 * Distance between 14 and 15: 8
 * Distance between 15 and 14: -8
 * Distance between 5 and 4: 1
 * Distance between 5 and 5: 0
 * Distance between begin and end: 10
 * list size: 11
 * Create new list from array: 11 5 4 12 2
 * New list is included in the other one: Yes
 * Free up the memory!
 *
 */

#include<stdbool.h> // bool
#include<stdio.h> // printf()
#include<stdlib.h> // malloc()


struct node  {
    int data;
    struct node* next;
    struct node* prev;
};

/*!
 * \brief create Allocate memory and create new single node
 * \param elem The value of node
 * \return Pointer to the new node
 */
struct node* create(int elem) {
    struct node* newNode
        = (struct node*)malloc(sizeof(struct node));
    newNode->data = elem;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

int value(struct node* ref) {
    return ref->data;
}

struct node* next(struct node* ref) {
    return ref->next;
}

struct node* prev(struct node* ref) {
    return ref->prev;
}

/*!
 * \brief begin Traverse the linked-list to the head of it
 * \param ref A node from list
 * \return head of list
 */
struct node* begin(struct node* ref) {
    while(ref->prev) {
        ref = prev(ref);
    }
    return ref;
}

/*!
 * \brief end Traverse the linked-list to the tail of it
 * \param ref A node from list
 * \return tail of list
 */
struct node* end(struct node* ref) {
    while(ref->next) {
        ref = next(ref);
    }
    return ref;
}

/*!
 * \brief erase Remove single given node and free allocated memory
 * \param ref Node to remove
 * \return Next node the newly removed node
 */
struct node* erase(struct node* ref) {
    struct node* nx = next(ref);
    struct node* px = prev(ref);

    free(ref);

    if(nx) {
        nx->prev = px;
    }

    if(px) {
        px->next = nx;
    }

    return nx;
}

/*!
 * \brief clear Removes all the nodes of list and free all allocated memory
 * \param ref A node from list
 */
void  clear(struct node* ref) {
    ref = begin(ref);
    while((ref = erase(ref)) != NULL);
}

/*!
 * \brief push_back Append new node to the end of list
 * \param ref A node from list
 * \param newElem Value of new element
 * \return The tail of list
 */
struct node* push_back(struct node* ref, int newElem) {

    struct node* tail = end(ref);

    struct node* newNode = create(newElem);

    tail->next = newNode;
    newNode->prev = tail;

    return newNode;
}

/*!
 * \brief push_front Insert a new node to the front of list
 * \param ref A node from list
 * \param newElem Value of new element
 * \return The head of list
 */
struct node* push_front(struct node* ref, int newElem) {

    struct node* head = begin(ref);

    struct node* newNode = create(newElem);

    head->prev = newNode;
    newNode->next = head;

    return newNode;
}

/*!
 * \brief pop_back Removes a node from tail of list
 * \param ref A node from list
 * \return New tail of list
 */
struct node* pop_back(struct node* ref) {

    struct node* tail = end(ref);

    struct node* newTail = prev(tail);

    erase(tail);

    return newTail;
}

/*!
 * \brief pop_front Removes a node from head of list
 * \param ref A node from list
 * \return New head of list
 */
struct node* pop_front(struct node* ref) {

    struct node* head = begin(ref);

    return erase(head);
}

unsigned int size(struct node* ref) {
    unsigned int sz = 1;
    struct node* head = begin(ref);
    while((head = next(head))!=NULL) {
        sz++;
    }
    return sz;
}

/*!
 * \brief createFromArray create a list from an array
 * \param arr pointer to the array
 * \param size size of the array
 * \return
 */
struct node* createFromArray(const int arr[], unsigned int size) {
    struct node* list;
    for(unsigned int i = 0; i < size; i++) {
        if(i==0) {
            list = create(arr[i]);
            continue;
        }
        list = push_back(list, arr[i]);
    }
    return list;
}

/*!
 * \brief insert Insert a node after given node
 * \param ref Given node
 * \param newElem Value of new node
 * \return Pointer to the newly inserted node
 */
struct node* insert(struct node* ref, int newElem) {

    struct node* nx = next(ref);

    struct node* newNode = create(newElem);

    if(nx) {
        nx->prev = newNode;
    }

    newNode->next = nx;
    newNode->prev = ref;

    ref->next = newNode;

    return newNode;
}

/*!
 * \brief ffind Froward search and find an element in the list
 * \param ref Given node to start the search
 * \param elem Value to be searched in the list
 * \return First node containing the element if not found will return NULL
 */
struct node* ffind(struct node* ref, int elem) {
    while(ref != NULL && ref->data != elem) {
        ref = next(ref);
    }
    return ref;
}

/*!
 * \brief ffind Reverse (backward) search and find an element in the list
 * \param ref Given node to start the search
 * \param elem Value to be searched in the list
 * \return First node containing the element if not found will return NULL
 */
struct node* rfind(struct node* ref, int elem) {
    while(ref != NULL && ref->data != elem) {
        ref = prev(ref);
    }
    return ref;
}

/*!
 * \brief swap Swap position of two nodes with each other within one list
 * \param ref1 A node from list
 * \param ref2 A node from list
 */
void swap(struct node* ref1, struct node* ref2) {

    /*
     * Nodes could be far from each other
     * Nodes could be next to each other ref2 after ref1
     * Nodes could be next to each other ref1 after ref2
     */
    /* Your code here */

    if (ref1->next == ref2) {
        struct node* px = prev(ref1);
        struct node* nx = next(ref2);

        if (px) px->next = ref2;
        if (nx) nx->prev = ref1;

        ref1->next = nx;
        ref1->prev = ref2;

        ref2->next = ref1;
        ref2->prev = px;

    } else if (ref1->prev == ref2) {
        struct node* px = prev(ref2);
        struct node* nx = next(ref1);

        if (px) px->next = ref1;
        if (nx) nx->prev = ref2;

        ref2->next = nx;
        ref2->prev = ref1;

        ref1->next = ref2;
        ref1->prev = px;

    } else {
        struct node* nx1 = next(ref1);
        struct node* px1 = prev(ref1);

        struct node* nx2 = next(ref2);
        struct node* px2 = prev(ref2);

        if (nx1) nx1->prev = ref2;
        if (px1) px1->next = ref2;

        if (nx2) nx2->prev = ref1;
        if (px2) px2->next = ref1;

        ref1->next = nx2;
        ref1->prev = px2;

        ref2->next = nx1;
        ref2->prev = px1;
    }
}


/*!
 * \brief reverse Reverse the elements order of list
 * \param ref A node from list
 */
void reverse(struct node* ref) {
    struct node* head = begin(ref);
    struct node* tail = end(ref);

    while(head != tail) { // Stop condition for list with odd elements
        struct node* nx = next(head);
        struct node* px = prev(tail);

        swap(head, tail);

        if(tail == nx) { // Stop condition for list with even elements
            break;
        }

        head = nx;
        tail = px;

    }
}

/*!
 * \brief concat Concatenate two list with each other
 * \param ref1 A node from list 1
 * \param ref2 A node from list 2
 * \return Pointer to the head of concatenated list
 */
struct node* concat(struct node* ref1, struct node* ref2) {
    /* Your code here */
    struct node* head = begin(ref2);
    struct node* tail = end(ref1);

    head->prev = tail;
    tail->next = head;

    return begin(ref1);
}


/*!
 * \brief distance Returns the number of hops from ref1 to ref2.
 * \param ref1
 * \param ref2
 * \return distance is positive if ref1 appears before ref2 and negative if ref2 appears before ref1
 */
int distance(struct node* ref1, struct node* ref2) {

    int dist = 0;
    int count1 = 0;
    int count2 = 0;
    /* Your code here */
    while (ref1->prev) {
        ref1 = prev(ref1);
        count1++;
    }

    while (ref2->prev){
        ref2 = prev(ref2);
        count2++;
    }

    dist = (count2-count1);

    return dist;
}

/*!
 * \brief unique Eliminates all but the first element from every consecutive group of equivalent elements from the list
 * \param ref A node form list
 * \return The head of the list
 */
struct node* unique(struct node* ref) {
    struct node* head = begin(ref);
    struct node* temp = head;

    /* Your code here */
    while (temp->next){
        struct node* nx = next(temp);
        if (temp->data == nx->data){
            erase(nx);
        } else {
            temp = next(temp);
        }
    }
    return head;
}


/*!
 * \brief rotate_left Performs a left rotation on list.
 * Swaps the elements in the list in such a way that the
 * element n_first becomes the first element of the
 * new list and n_first - 1 becomes the last element.
 * \param ref A node from list
 * \param n The number of positions to rotate
 * \return The head (begin) of the list
 */

 /*
 * My understanding is that this function rotates n nodes from the front of the
 * list to the back.
 */
struct node* rotate_left(struct node* ref, int n) {
    struct node* head = begin(ref);
    /* Your code here */

    for (int i = 0; i < n; i++){
        struct node* tail = push_back(head, (head->data));
        head = pop_front(tail);

    }

    return head;
}

/*!
 * \brief shift_left Shifts the elements towards the beginning of the range.
 * \param ref A node from list
 * \param n The number of positions to shift
 * \return
 */

 /*
 * My understanding of this function is that it removes n items from the font
 * of the list, which looks like the list has been "shifted" to the left n times.
 */
struct node* shift_left(struct node* ref, int n) {
    /* Your code here */
    struct node* tail = end(ref);

    for (int i = 0; i < n; i++) {
        if (tail) pop_front(tail);
    }

    if (tail) return begin(tail);

    return tail;
}

/*!
 * \brief minmax Returns the lowest and the greatest of the given list.
 * \param ref A node from list
 * \param min Minimum returned value
 * \param max Maximum returned value
 */
void minmax(struct node* ref, int* min, int* max) {
    /* Your code here */
    struct node* head = begin(ref);
    *min = head->data;
    *max = head ->data;

    while (head->next) {
        if (head->data < *min){
            *min = head->data;
        }
        if (head->data > *max){
            *max = head->data;
        }
        head = next(head);
    }
}

/*!
 * \brief includes Returns true if the list starting from ref2 node is a subset of the list starting from ref1.
 * \param ref1
 * \param ref2
 * \return
 */

bool includes(struct node* ref1, struct node* ref2) {
    /* Your code here */
    int sz1 = size(ref1);
    int sz2 = size(ref2);
    int count = 0;

    //copies of starting nodes
    struct node* cr1;
    struct node* cr2;

    for (int i = 0; i < sz1; i++) {
        count = 0;
        cr1 = ref1;
        cr2 = ref2;
        for (int j = 0; j < sz2; j++) {
            if (cr2->data == cr1->data){
                cr2 = cr2->next;
                cr1 = cr1->next;
                count++;
            } else {
                break;
            }

            if (count == sz2){
                return true;
            }
        }
        ref1 = next(ref1);
    }

    return false;
}



/*!
 * \brief print Print all elements of list following a new line
 * \param ref A node from list
 */
void print(struct node* ref) {
    struct node* n = begin(ref);

    do {
        printf("%d ",n->data);
    } while((n = next(n)) != NULL);
    printf("\n");
}


int main() {

    struct node* list = create(3);
    //int* min = (int*)malloc(sizeof(int));
    //int* max = (int*)malloc(sizeof(int));

    push_back(list, 4);

    push_front(list, 2);

    for(int i = 5; i < 15; i+=3) {
        list = push_back(list, i);
    }

    push_back(list, 14);

    printf("Intial list: ");
    print(list);

    printf("list size: %d\n", size(list));


    printf("Delete element with value 8: ");
    erase(ffind(begin(list), 8));
    print(list);

    printf("Insert 12 after 11 by reverse search: ");
    insert(rfind(end(list), 11), 12);
    print(list);

    printf("Swap 3 and 12: ");
    swap(ffind(begin(list), 3), rfind(end(list), 12));
    print(list);

    printf("Swap 4 and 5: ");
    swap(ffind(begin(list), 4), ffind(begin(list), 5));
    print(list);

    printf("Swap again 4 and 5: ");
    swap(ffind(begin(list), 4), ffind(begin(list), 5));
    print(list);


    printf("Reverse the list: ");
    reverse(list);
    print(list);

    printf("Push 55 and then reverse the list again: ");
    push_back(list, 55);
    reverse(list);
    print(list);


    printf("Create another list: ");
    struct node* list2 = create(120);
    for(int i = 60; i > 5; i/=2) {
        list2 = push_front(list2, i);
    }
    print(list2);


    printf("Concatenate two lists: ");
    list = concat(list, list2);
    print(list);

    printf("Revese the new list: ");
    reverse(list);
    print(list);


    printf("Insert one 3 and two 7 after 15: ");
    struct node* newNode = insert(ffind(begin(list), 15), 3);
    newNode = insert(newNode, 7);
    insert(newNode, 7);
    print(list);

    printf("Apply unique function to the list: ");
    unique(list);
    print(list);


    printf("Shift left 3 times: ");
    shift_left(list, 3);
    print(list);


    printf("Rotate left 2 times: ");
    rotate_left(list, 2);
    print(list);

    list = begin(list);
    printf("Distance between 14 and 15: %d\n", distance(ffind(list, 14), ffind(list, 15)));
    printf("Distance between 15 and 14: %d\n", distance(ffind(list, 15), ffind(list, 14)));
    printf("Distance between 5 and 4: %d\n", distance(ffind(list, 5), ffind(list, 4)));
    printf("Distance between 5 and 5: %d\n", distance(ffind(list, 5), ffind(list, 5)));
    printf("Distance between begin and end: %d\n", distance(begin(list), end(list)));
    printf("list size: %d\n", size(list));

    printf("Create new list from array: ");
    int arr[] = {11, 5, 4, 12, 2};
    struct node* list3 = createFromArray(arr, sizeof arr/sizeof(int));
    print(list3);

    printf("New list is included in the other one: %s\n", includes(begin(list), begin(list3)) ? "Yes" : "No");
//    minmax(list, min, max);
//    printf("Minimum: %d, Maximum: %d \n", *min, *max);

    printf("Free up the memory!\n");
    clear(list);
    clear(list3);
//    free(min);
//    free(max);

    return 0;
}
