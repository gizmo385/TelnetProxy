#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>	//for closing the connection

#include "protocol.h"

void send_message(int socket, message_t *message) {

    if(!message) {
        fprintf(stderr, "Attempting to send NULL on socket %d\n", socket);
        return;
    }

    // Send the message header
    uint32_t n_message_flag = htonl((uint32_t) message->message_flag);
    send(socket, (void *) &n_message_flag, sizeof(n_message_flag), 0);

    switch(message->message_flag) {
        case HEARTBEAT_FLAG: // No body
            break;
        case DATA_FLAG:
            {
                data_message_t *data = message->body->data;

                // Convert the integer types
                uint32_t n_message_size = htonl((uint32_t) data->message_size);
                uint32_t n_seq_num = htonl((uint32_t) data->seq_num);
                uint32_t n_ack_num = htonl((uint32_t) data->ack_num);

                // Send the actual data
                send(socket, (void *) &n_message_size, sizeof(n_message_size), 0);
                send(socket, (void *) &n_seq_num, sizeof(n_seq_num), 0);
                send(socket, (void *) &n_ack_num, sizeof(n_ack_num), 0);
                send(socket, (void *) data->payload, data->message_size, 0);
                break;
            }
        case CONNECTION_FLAG:
            {
                conn_message_t *conn = message->body->conn;

                // Convert the integer types
                uint32_t n_seq_num = htonl((uint32_t) conn->seq_num);
                uint32_t n_ack_num = htonl((uint32_t) conn->ack_num);

                // Send the actual data
                send(socket, (void *) &n_seq_num, sizeof(n_seq_num), 0);
                send(socket, (void *) &n_ack_num, sizeof(n_ack_num), 0);
                send(socket, (void *) conn->old_ip, sizeof(char) * IP_SIZE, 0);
                send(socket, (void *) conn->new_ip, sizeof(char) * IP_SIZE, 0);
                break;
            }
        default: // YOU REAL GOOFED
            fprintf(stderr, "%d is not a valid message flag, ya doofus\n", message->message_flag);
            break;
    }
}

message_t *new_message_t(char flag, message_body_t *body) {
    message_t *message = malloc(sizeof(message_t));
    message->message_flag = flag;
    message->body = body;

    return message;
}

message_t *new_heartbeat_message() {
    return new_message_t(HEARTBEAT_FLAG, NULL);
}

message_t *new_conn_message(int seq_num, int ack_num, char *old_ip, char *new_ip) {
    conn_message_t *conn = malloc(sizeof(conn_message_t));
    conn->seq_num = seq_num;
    conn->ack_num = ack_num;
    conn->old_ip = old_ip;
    conn->new_ip = new_ip;

    message_body_t *body = malloc(sizeof(message_body_t));
    body->conn = conn;

    return new_message_t(CONNECTION_FLAG, body);
}

message_t *new_data_message(int seq_num, int ack_num, int message_size, char *payload) {
    data_message_t *data = malloc(sizeof(data_message_t));
    data->seq_num = seq_num;
    data->ack_num = ack_num;
    data->message_size = message_size;
    data->payload = payload;

    message_body_t *body = malloc(sizeof(message_body_t));
    body->data = data;

    return new_message_t(DATA_FLAG, body);
}