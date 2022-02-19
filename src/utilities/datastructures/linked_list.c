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

    // I don't want to have to rewrite both. might be slower. idc
    ListNode *node = list_getNode(list, index, state);

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
    for (size_t i = 1; i < index && i < list->length; i++)
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
    ListNode *newNode = malloc (sizeof (ListNode));

    newNode->value = value;
    newNode->size = sizeof (typeof (*value)); // experiment

    list->length++; // make list longer

    // assumes head is set to NULL if there is only no value?
    if (list->head == NULL)
    {
        list->head = newNode; // put new node in list

        list->tail = newNode; // tail too ig?

        return LIST_SUCCESS;
    }

    // stick new node in at end of list
    list->tail->next = newNode; // set current tail next to node
    newNode->back = list->tail; // set node back to current tail
    list->tail = newNode; // set list tail to node

    return LIST_SUCCESS;
}

// prepend implementation
ListReturns list_prepend (LinkedList *list, void *value)
{
    
    // create new node
    ListNode *newNode = malloc (sizeof (ListNode));
    newNode->value = value;
    newNode->size = sizeof (typeof (*value)); // experiment

    list->length++; // make list longer

    // check if list is empty
    if (list->head == NULL)
    {
        list->head = newNode;
        list->tail = newNode;

        return LIST_SUCCESS;
    }

    // insert newNode into head of list
    newNode->next = list->head;
    list->head->back = newNode;
    list->head = newNode;

    
    return LIST_SUCCESS;
}

ListReturns list_push (LinkedList *list, size_t index, void *value)
{

    // create new node
    ListNode *newNode = malloc (sizeof (ListNode));
    newNode->value = value;
    newNode->size = sizeof (typeof (*value));

    // check if index is past end of list (+1 bc index 0 is valid)
    if (index + 1 > list->length) return LIST_INDEX_OVERFLOW;
    
    // check if the list is empty
    if (list->head == NULL)
    {
        list->head = newNode;
        list->tail = newNode;
    }
    
    // point node to index
    ListReturns state;
    ListNode *node = list_getNode(list, index, &state);
    if (state < LIST_SUCCESS) return state; // catch errors

    // put newNode in same spot as node
    newNode->back = node->back;
    newNode->next = node;
    node->back->next = newNode; // set node before newNode next to newNode
    node->back = newNode; // set node back to newnode

    list->length++; // make list longer bc list is longer

    return LIST_SUCCESS;
}

ListReturns list_pop (LinkedList *list, size_t index)
{
    // fuck you
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
