#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>	//for closing the connection

#include "protocol.h"
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

void send_message(int socket, message_t *message) {

    if(!message) {
        fprintf(stderr, "Attempting to send NULL on socket %d\n", socket);
        return;
    }

    // Send the message header
    uint32_t n_message_flag = htonl((uint32_t) message->message_flag);
    send(socket, (void *) &n_message_flag, sizeof(n_message_flag), 0);

    // Send the message body
    switch(message->message_flag) {
        case HEARTBEAT_FLAG: // No body
            break;
        case DATA_FLAG:
            {
                data_message_t *data = message->body->data;

                // Convert the integer types
                uint32_t message_size = htonl((uint32_t) data->message_size);
                uint32_t seq_num = htonl((uint32_t) data->seq_num);
                uint32_t ack_num = htonl((uint32_t) data->ack_num);

                // Send the actual data
                send(socket, (void *) &message_size, sizeof(message_size), 0);
                send(socket, (void *) &seq_num, sizeof(seq_num), 0);
                send(socket, (void *) &ack_num, sizeof(ack_num), 0);
                send(socket, (void *) data->payload, data->message_size, 0);
                break;
            }
        case CONNECTION_FLAG:
            {
                conn_message_t *conn = message->body->conn;

                // Convert the integer types
                uint32_t seq_num = htonl((uint32_t) conn->seq_num);
                uint32_t ack_num = htonl((uint32_t) conn->ack_num);

                // Send the actual data
                send(socket, (void *) &seq_num, sizeof(seq_num), 0);
                send(socket, (void *) &ack_num, sizeof(ack_num), 0);
                send(socket, (void *) conn->old_ip, sizeof(char) * IP_SIZE, 0);
                send(socket, (void *) conn->new_ip, sizeof(char) * IP_SIZE, 0);
                break;
            }
        default: // YOU REAL GOOFED
            fprintf(stderr, "%d is not a valid message flag, ya doofus\n", message->message_flag);
            break;
    }
}

void safe_read(int socket, char *buffer, size_t size) {
    // TODO: Ensure complete read
}

message_t *read_message(int socket) {
    uint32_t n_message_flag;
    int result = read(socket, (char *) &n_message_flag, sizeof(uint32_t));
    int message_flag = ntohl(n_message_flag);

    printf("Read message with flag %d\n from socket %d\n", message_flag, socket);

    switch(message_flag) {
        case HEARTBEAT_FLAG:
            return result ? new_heartbeat_message() : NULL;
            break;
        case CONNECTION_FLAG:
            {
                uint32_t seq_num, ack_num;
                char *old_ip = calloc(IP_SIZE, sizeof(char));
                char *new_ip = calloc(IP_SIZE, sizeof(char));

                result = MIN(result, read(socket, (char *) &seq_num, sizeof(uint32_t)));
                result = MIN(result, read(socket, (char *) &ack_num, sizeof(uint32_t)));
                result = MIN(result, read(socket, (char *) old_ip, sizeof(char) * IP_SIZE));
                result = MIN(result, read(socket, (char *) new_ip, sizeof(char) * IP_SIZE));

                seq_num = ntohl(seq_num);
                ack_num = ntohl(ack_num);

                return result ? new_conn_message(seq_num, ack_num, old_ip, new_ip) : NULL;
                break;
            }
        case DATA_FLAG:
            {
                uint32_t message_size, seq_num, ack_num;

                result = MIN(result, read(socket, (char *) &message_size, sizeof(uint32_t)));
                result = MIN(result, read(socket, (char *) &seq_num, sizeof(uint32_t)));
                result = MIN(result, read(socket, (char *) &ack_num, sizeof(uint32_t)));

                message_size = ntohl(message_size);
                seq_num = ntohl(seq_num);
                ack_num = ntohl(ack_num);

                char *payload = calloc(message_size, sizeof(char));
                result = MIN(result, read(socket, payload, message_size));

                return result ? new_data_message(seq_num, ack_num, message_size, payload) : NULL;
                break;
            }
        default:
            fprintf(stderr, "%d is not a readable message flag to read from %d!\n", message_flag,
                    socket);
            return NULL;
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
    data_message_t *data = calloc(1, sizeof(data_message_t));
    data->seq_num = seq_num;
    data->ack_num = ack_num;
    data->message_size = message_size;

    data->payload = calloc(message_size, sizeof(char));
    memcpy(data->payload, payload, message_size);

    message_body_t *body = calloc(1, sizeof(message_body_t));
    body->data = data;

    return new_message_t(DATA_FLAG, body);
}
