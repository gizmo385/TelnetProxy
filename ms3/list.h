#include "protocol.h"

typedef struct node_t {
    message_t *message;
    struct node_t *next;
} node_t;

typedef struct list_t {
    node_t *head;
    node_t *tail;
} list_t;

extern void list_t_add(list_t *list, message_t *message);
extern message_t* list_t_pop(list_t *list);
extern message_t* list_t_peek(list_t *list);
extern list_t *new_list_t();
