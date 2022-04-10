#include <sys/types.h>

typedef struct process_data
{
    pid_t pid;
    char command[128];
} process_data;

typedef struct node
{
    process_data *data;
    struct node *next;
} node;

node *init_list();

node *createNode(process_data *data);

char *getData(node *n);

pid_t getPid(node *n);

node *next_node(node *n);

int deleteNode(node *head, node *n);

int addNode(node *head, process_data *data);