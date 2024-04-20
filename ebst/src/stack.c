

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"



static stack_node * newStackNode(const stack_item * _Item) {
    stack_node * sn = (stack_node *) malloc(sizeof(stack_node));
    if (sn == NULL)
        return NULL;

    sn -> item = * _Item;
    sn -> next = NULL;
    return sn;
}


const Stack newStack(void) {
    const Stack S = { 0, NULL };
    return S;
}


bool stackPush(Stack * _Stack, const stack_item * _Item) {
    stack_node * new_node = newStackNode(_Item);
    if (new_node == NULL)
        return false;

    stack_node * head_next = _Stack -> head;
    _Stack -> head = new_node;
    _Stack -> head -> next = head_next;
    _Stack -> size ++;

    return true;
}


bool stackPop(Stack * _Stack, stack_item * _ReturnItem) {
    if (_Stack -> head == NULL)
        return false;
    
    stack_node * node_buffer = _Stack -> head;
    
    _Stack -> head = _Stack -> head -> next;
    _Stack -> size --;

    * _ReturnItem = node_buffer -> item;
    free(node_buffer);
    
    return true;
}


void printStack(const Stack * _Stack) {
    stack_node * ptr = _Stack -> head;
    printf("S < ");
    while (ptr) {
        printf("(%d, %d) ", ptr -> item.left, ptr -> item.right);
        ptr = ptr -> next;
    }
    printf(">\n");
}