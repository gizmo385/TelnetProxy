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
#define MAX_WINDOW_SIZE	50

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
                uint32_t new_session = htonl((uint32_t) conn->new_session);
                uint32_t last_mess_recv = htonl((uint32_t) conn->last_mess_recv);

                // Send the actual data
                send(socket, (void *) &new_session, sizeof(new_session), 0);
                send(socket, (void *) &last_mess_recv, sizeof(last_mess_recv), 0);
                break;
            }
		case ACK_FLAG:
			{
				data_message_t *data = message->body->data;
                // Convert the integer types
                uint32_t seq_num = htonl((uint32_t) data->seq_num);
                uint32_t ack_num = htonl((uint32_t) data->ack_num);

				// Send the actual data
                send(socket, (void *) &seq_num, sizeof(seq_num), 0);
                send(socket, (void *) &ack_num, sizeof(ack_num), 0);

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
    if(result == 0) return NULL;
    int message_flag = ntohl(n_message_flag);

    printf("Read message with flag %d\n from socket %d\n", message_flag, socket);

    switch(message_flag) {
        case HEARTBEAT_FLAG:
            return new_heartbeat_message();
            break;
        case CONNECTION_FLAG:
            {
                uint32_t new_session, last_mess_recv;
                result = read(socket, (char *) &new_session, sizeof(uint32_t));
                new_session = ntohl(new_session);
                if(result <= 0) return NULL;

                result = read(socket, (char *) &last_mess_recv, sizeof(uint32_t));
                last_mess_recv = ntohl(last_mess_recv);
                if(result <= 0) return NULL;

                printf("read_message(conn_message) --> %d\n", result);

                return new_conn_message(last_mess_recv, new_session);
                break;
            }
        case DATA_FLAG:
            {
                uint32_t message_size, seq_num, ack_num;

                result = read(socket, (char *) &message_size, sizeof(uint32_t));
                if(result <= 0) return NULL;

                result = read(socket, (char *) &seq_num, sizeof(uint32_t));
                if(result <= 0) return NULL;

                result = read(socket, (char *) &ack_num, sizeof(uint32_t));
                if(result <= 0) return NULL;

                message_size = ntohl(message_size);
                seq_num = ntohl(seq_num);
                ack_num = ntohl(ack_num);

                char *payload = calloc(message_size, sizeof(char));
                result = read(socket, payload, message_size);
                if(result <= 0) return NULL;

                return new_data_message(seq_num, ack_num, message_size, payload);
                break;
            }
		case ACK_FLAG:
			{
                uint32_t seq_num, ack_num;

                result = read(socket, (char *) &seq_num, sizeof(uint32_t));
                if(result <= 0) return NULL;

                result = read(socket, (char *) &ack_num, sizeof(uint32_t));
                if(result <= 0) return NULL;

                seq_num = ntohl(seq_num);
                ack_num = ntohl(ack_num);

                return new_ack_message(seq_num, ack_num);
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

message_t *new_conn_message(int last_mess_recv, int new_session) {
    conn_message_t *conn = malloc(sizeof(conn_message_t));
    conn->new_session = new_session;
	conn->last_mess_recv = last_mess_recv;

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

message_t *new_ack_message(int seq_num, int ack_num){
    data_message_t *data = calloc(1, sizeof(data_message_t));
    data->seq_num = seq_num;
    data->ack_num = ack_num;

	message_body_t *body = calloc(1, sizeof(message_body_t));
    body->data = data;

    return new_message_t(ACK_FLAG, body);
}
