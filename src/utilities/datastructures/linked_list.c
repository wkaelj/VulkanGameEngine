#include "linked_list.h"

struct t_ListNode
{
    ListNode *next;
    ListNode *back;
    void *value;
    size_t size;
};

// get implementation
void *list_get (LinkedList *list, size_t index, ListReturns *state)
{

    ListReturns myState; // state in case other is null
    // I don't want to have to rewrite both. might be slower. idc
    ListNode *node = list_getNode(list, index, &myState);

    // error stuffs
    if (myState != LIST_SUCCESS)
    {
        if (state != NULL) state = &myState;
        return NULL;
    }
    if (node->value == NULL && state != NULL){
         *state = LIST_INDEX_NULL;
         return NULL;
    }

    return node->value;
}

// get implementation
ListNode *list_getNode (LinkedList *list, size_t index, ListReturns *state)
{
    ListNode *node;

    // access node at index
    node = list->head;
    // start counting at 1 bc head is 0 also dont count longer then list
    for (size_t i = 0; i < index && i < list->length; i++)
    {
        // check if node it tail and index is still true
        if (node == list->tail && i < index)
        {
            if (state != NULL ) *state = LIST_INDEX_OVERFLOW;
            return NULL;
        }

        node = node->next; // set node to the next node
    }

    // return
    if (state != NULL) *state = LIST_SUCCESS;
    return node;

}


// append implementation
ListReturns list_append (LinkedList *list, void *value)
{
    
    // create the new node
    ListNode *node = newNode (); 

    node->value = value;
    node->size = sizeof (typeof (*value)); // experiment

    list->length++; // make list longer

    // assumes head is set to NULL if there is only no value?
    if (list->head == NULL)
    {
        list->head = node; // put new node in list

        list->tail = node; // tail too ig?

        return LIST_SUCCESS;
    }

    // stick new node in at end of list
    list->tail->next = node; // set current tail next to node
    node->back = list->tail; // set node back to current tail
    list->tail = node; // set list tail to node

    return LIST_SUCCESS;
}

// prepend implementation
ListReturns list_prepend (LinkedList *list, void *value)
{
    
    // create new node
    ListNode *node = newNode ();
    node->value = value;
    node->size = sizeof (typeof (*value)); // experiment

    list->length++; // make list longer

    // check if list is empty
    if (list->head == NULL)
    {
        list->head = node;
        list->tail = node;

        return LIST_SUCCESS;
    }

    // insert node into head of list
    node->next = list->head;
    list->head->back = node;
    list->head = node;

    
    return LIST_SUCCESS;
}

ListReturns list_push (LinkedList *list, size_t index, void *value)
{

    // check if index is past end of list (not >= thats handled)
    if (index > list->length) return LIST_INDEX_OVERFLOW;
    
    // create new node
    ListNode *node = newNode ();
    node->value = value;
    node->size = sizeof (typeof (*value));


    // catch start of list and tail of list
    if (index == 0)
    {
        list->head->back = node;
        node->next = list->head;
        list->head = node;
        list->length++;

        return LIST_SUCCESS;
    }
    else if (index == list->length)
    {
        list->tail->next = node;
        node->back = list->tail;
        list->tail = node;

        list->length++;

        return LIST_SUCCESS;
    }

    // check if the list is empty
    if (list->head == NULL)
    {
        list->head = node;
        list->tail = node;
    }
    
    // point node to index
    ListReturns state;
    ListNode *bNode = list_getNode (list, index, &state);
    if (state < LIST_SUCCESS) return state; // catch errors

    // put node in same spot as node
    node->back = bNode->back;
    node->next = bNode;
    bNode->back->next = node; // set node before newNode next to newNode
    bNode->back = node; // set node back to newnode

    list->length++; // make list longer bc list is longer

    return LIST_SUCCESS;
}

// pop node from list
ListReturns list_pop (LinkedList *list, size_t index)
{
    ListReturns state;

    ListNode *node = list_getNode(list, index, &state);

    if (state != LIST_SUCCESS || node == NULL) {
        return state;
    }
    // knit closed space around node
    if (node->back != NULL) node->back->next = node->next;
    if (node->next != NULL) node->next->back = node->back;

    list->length--;

    free (node); // better not copy any of the nodes.
    return LIST_SUCCESS;
}

// newList implementation
LinkedList newList (void)
{
    LinkedList out;

    // set function pointers
    out.get = &list_get;
    out.getNode = &list_getNode;
    out.append = &list_append;
    out.prepend = &list_prepend;
    out.push = &list_push;
    out.pop = &list_pop;

    // set head and tail to NULL
    out.head = NULL;
    out.tail = NULL;

    out.length = 0;

    return out;
}

ListNode *newNode (void)
{
    ListNode *out = malloc (sizeof (ListNode));
    out->next = NULL;
    out->back = NULL;
    out->value = NULL;
    out->size = 0;

    return out;
}
