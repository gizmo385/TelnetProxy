#define HEARTBEAT_FLAG 1
#define DATA_FLAG 2
#define CONNECTION_FLAG 3

#define IP_SIZE 6
#define BUFFER_SIZE 1024

typedef struct {
    int message_size;
    int seq_num;
    int ack_num;
    char *payload;
} data_message_t;

typedef struct {
    int seq_num;
    int ack_num;
    char *old_ip;
    char *new_ip;
} conn_message_t;

typedef union message_body_t {
    conn_message_t *conn;
    data_message_t *data;
} message_body_t;

typedef struct message_t {
    int message_flag;
    message_body_t *body;
} message_t;

extern void send_message(int socket, message_t *message);
extern message_t *read_message(int socket);

extern message_t *new_heartbeat_message();
extern message_t *new_conn_message(int seq_num, int ack_num, char *old_ip, char *new_ip);
extern message_t *new_data_message(int seq_num, int ack_num, int message_size, char *payload);
