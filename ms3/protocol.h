#define HEARTBEAT_FLAG 1
#define DATA_FLAG 2
#define CONNECTION_FLAG 3

typedef struct {
    char message_flag;
} heartbeat_message;

typedef struct {
    char message_flag;
    int message_size;
    int seq_num;
    int ack_num;
    char *payload;
} data_message;

typedef struct {
    char message_flag;
    int seq_num;
    int ack_num;
    char old_ip[6];
    char new_ip[6];
} conn_message;
