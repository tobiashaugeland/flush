#include <sys/types.h>

typedef struct node node;

typedef struct process_data process_data;

node *init_list();

node *createNode(process_data *data);

process_data getData(node *n);

pid_t getPid(node *n);

node *next_node(node *n);

int deleteNode(node *head, node *n);

int addNode(node *head, process_data *data);