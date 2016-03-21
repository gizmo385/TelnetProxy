#include "protocol.h"

typedef struct node_t {
    data_message_t *message;
    node_t *next;
} node_t;

typedef struct list_t {
    node_t *head;
    node_t *tail;
}

extern void list_t_add(list_t *list, data_message_t *message);
extern data_message_t* list_t_pop(list_t *list);
extern data_message_t* list_t_peek(list_t *list);
