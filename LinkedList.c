#include "LinkedList.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct process_data
{
    pid_t pid;
    char command[128];
} process_data;

typedef struct node {
    process_data *data;
    struct node *next;
} node;

node *init_list()
{
    return NULL;
}

node *createNode(process_data *data)
{
    node *newNode = (node *)malloc(sizeof(node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

int addNode(node *head, process_data *data)
{
    node *newNode = createNode(data);
    if (head == NULL)
    {
        head = newNode;
        return 0;
    }
    node *current = head;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = newNode;
    return 0;
}

int deleteNode(node *head, node *n)
{
    if (head == NULL)
    {
        return -1;
    }
    if (head == n)
    {
        head = n->next;
        free(n);
        return 0;
    }
    node *current = head;
    while (current->next != NULL)
    {
        if (current->next == n)
        {
            current->next = n->next;
            free(n);
            return 0;
        }
        current = current->next;
    }
    return -1;
}

pid_t getPid(node *n)
{
    return n->data->pid;
}


node *next_node(node *n)
{
    return n->next;
}