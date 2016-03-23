#include <stdlib.h>
#include <stdint.h>
#include <stdint.h>

#define HEARTBEAT_FLAG  1
#define DATA_FLAG       2
#define CONNECTION_FLAG 3
#define ACK_FLAG		4

#define IP_SIZE         6
#define BUFFER_SIZE     1024

#define NEW_SESSION     1
#define OLD_SESSION     0

#define TIMEOUT_THRESH  3

typedef struct {
    int message_size;
    int seq_num;
    int ack_num;
    char *payload;
} data_message_t;

typedef struct {
	int last_mess_recv;
    int new_session;
} conn_message_t;

typedef union message_body_t {
    conn_message_t *conn;
    data_message_t *data;
} message_body_t;

typedef struct message_t {
    uint32_t message_flag;
    message_body_t *body;
} message_t;

extern int safe_read(int socket, char *buffer, int size);
extern void send_message(int socket, message_t *message);
extern message_t *read_message(int socket);

extern message_t *new_heartbeat_message();
extern message_t *new_conn_message(int last_mess_recv, int new_session);
extern message_t *new_data_message(int seq_num, int ack_num, int message_size, char *payload);
extern message_t *new_ack_message(int seq_num, int ack_num);
