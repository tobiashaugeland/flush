#include "LinkedList.h"
#include <stdlib.h>
#include <stdio.h>

node *init_list()
{
    node *head = malloc(sizeof(node));
    head->data = NULL;
    head->next = NULL;
    return head;
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
    if (head->next == NULL)
    {
        head->next = newNode;
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
    node *current = head;
    while (current->next != NULL)
    {
        if (current->next == n)
        {
            current->next = n->next;
            //free(n); Why won't this work?
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