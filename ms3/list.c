#include "list.h"

#define RETRANSMIT_THRESHOLD 5

void list_t_add(list_t *list, message_t *message) {
    node_t *node = malloc(sizeof(node_t));
    node->message = message;
    node->next = NULL;
    gettimeofday(&node->last_sent, NULL);

    if(!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
}

message_t *list_t_pop(list_t *list) {
    if(!list->head) {
        return NULL;
    } else {
        message_t *message = list->head->message;
        list->head = list->head->next;
        return message;
    }
}

message_t* list_t_peek(list_t *list) {
    if(!list->head) {
        return NULL;
    } else {
        return list->head->message;
    }
}

list_t *new_list_t() {
    list_t *list = malloc(sizeof(list_t));
    list->head = NULL;
    list->tail = NULL;

    return list;
}

void remove_messages(list_t *list, int ack_num) {
    while(list->head) {
        data_message_t *data = list->head->message->body->data;

        if(data->ack_num < ack_num) {
            list->head = list->head->next;
        } else {
            break;
        }
    }
}

void retransmit_all_messages(int socket, list_t *list) {
    node_t *current_node = list->head;
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    while(current_node) {
        send_message(socket, current_node->message);
        current_node->last_sent = current_time;
        current_node = current_node->next;
    }
}

void retransmit_messages(int socket, list_t *list) {
    node_t *current_node = list->head;
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    while(current_node) {
        struct timeval last_sent = current_node->last_sent;

        if((current_time.tv_sec - last_sent.tv_sec) > RETRANSMIT_THRESHOLD) {
            send_message(socket, current_node->message);
            current_node->last_sent = current_time;
        }

        current_node = current_node->next;
    }
}
