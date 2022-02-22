#ifndef LINKED_LIST
#define LINKED_LIST

#include <stdlib.h>

//
// An very easy linked list implementation
// I wanted to experiment with making C feel a little more object
// oriented without giving up any speed (or using C++).
//
// CURRENTLY USING TYPEOF(), SO ARRAYS MUST BE HANDLED BY USER
//
// Kael Johnston, Feb 18th 2022

// define linkedlist type
typedef struct t_ListNode ListNode;

// define linkedlist handle
typedef struct t_LinkedList LinkedList;

// define list returns
typedef enum e_ListReturns ListReturns;

// define function types
typedef void* (*t_get)(LinkedList*, size_t, ListReturns*);
typedef ListNode* (*t_getNode)(LinkedList*, size_t, ListReturns*);
typedef ListReturns (*t_append)(LinkedList*, void*);
typedef ListReturns (*t_prepend)(LinkedList*, void*);
typedef ListReturns (*t_push)(LinkedList*, size_t index, void*);
typedef ListReturns (*t_pop)(LinkedList*, size_t index);


// possible list return info
enum e_ListReturns
{
    LIST_SUCCESS = 0,
    LIST_FAILURE = 1,
    LIST_INDEX_OVERFLOW = 2,
    LIST_INDEX_NULL = 3
};

struct t_LinkedList
{
    ListNode *head;
    ListNode *tail;

    size_t length;
    
    // function pointers
    t_get get;
    t_getNode getNode;
    t_append append;
    t_prepend prepend;
    t_push push;
    t_pop pop;
};

// initialize new linkedlist (LinkedList x = newList ();)
LinkedList newList (void);

// delete a linked list
ListReturns deleteList (LinkedList *list);

// initialize a new list node
ListNode *newNode (void);

// read a value from the list. Returns null on failure
void *list_get (LinkedList *list, size_t index, ListReturns *state);

// get node from list. do not mess with. returns not on failure
ListNode *list_getNode (LinkedList *list, size_t index, ListReturns *state);

// append a value to the end of list
ListReturns list_append (LinkedList *list, void *value);

// push value to the beggining of list
ListReturns list_prepend (LinkedList *list, void *value);

// push value to index of list (value becomes index of index)
ListReturns list_push (LinkedList *list, size_t index, void *value);

// remove node from list at index
ListReturns list_pop (LinkedList *list, size_t index);


#endif // header guard
