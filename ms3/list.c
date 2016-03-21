#include "list.h"

void list_t_add(list_t *list, message_t *message) {
    node_t *node = calloc(1, sizeof(node_t));
    node->message = message;

    if(!list) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
}

message_t *list_t_pop(list_t *list) {
    if(!list) {
        return NULL;
    } else {
        message_t *message = list->head->message;
        list->head = list->head->next;
    return message;
    }
}

message_t* list_t_peek(list_t *list) {
    if(!list) {
        return NULL;
    } else {
        return list->head->message;
    }
}