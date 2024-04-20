

#ifndef _STACK_HEADER_
#define _STACK_HEADER_


#include "common.h"


typedef struct {
    int left, right;
} stack_item;

typedef struct stack_node_t {
    stack_item item;
    struct stack_node_t * next;
} stack_node;



typedef struct {
    size_t size;
    stack_node * head;
} Stack;


const Stack newStack(void);
bool stackPush(Stack * _Stack, const stack_item * _Item);
bool stackPop(Stack * _Stack, stack_item * _ReturnItem);

void printStack(const Stack* _);

#endif // _STACK_HEADER_

