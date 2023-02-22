#ifndef __JOB__H__
#define __JOB__H__


#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define NO_FLAG 0
#define DATA_SIZE 255

#define PASS 1
#define FAIL 0

#define PERROR(str)              \
  fprintf(stderr, "[%d] ", pid); \
  perror(str);

#define PRINTF(fmt, ...)            \
  fprintf(stdout, "[%d] ", pid);    \
  fprintf(stdout, fmt, __VA_ARGS__);

#define EPRINTF(fmt, ...)                       \
  fprintf(stderr, "[%d] ", pid);                \
  fprintf(stderr, "Error at : %d, ", __LINE__); \
  fprintf(stderr, fmt, __VA_ARGS__);

#define LOG(fmt, ...)                  \
  if (f_log) {                         \
    fprintf(stderr, "[%d] ", pid);     \
    fprintf(stderr, fmt, __VA_ARGS__); \
  }

// flags
static int f_log = 0;
static int pid = 0;

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

typedef enum {
    J_CLOSE = 0,

    J_FILE  = 1,
    J_CMD   = 2,
    J_STR   = 3,

    J_ERROR = 0xFE,
    UNKNOWN = 0xFF,
} JobType;

typedef struct {
    int8_t type;
    char *str;
} Job;

typedef struct {
    int8_t type;
    int8_t remaining_packets;

    uint8_t len;
    char data[DATA_SIZE];
} Packet;

int calculate_cmd_packets(ssize_t len)
{
    int num_packets = (len + DATA_SIZE - 1) / DATA_SIZE;
    return num_packets;
}

int send_file(int socket, FILE *file)
{
    Packet packet = {
        .type = J_FILE,
        .remaining_packets = 0,
        .len = 0,
        .data = {0},
    };

    size_t bytes_read = 0;
    ssize_t bytes_sent = 0;

    while ((bytes_read = fread(packet.data, sizeof(char), DATA_SIZE, file)) > 0) {
        packet.remaining_packets = 1;
        packet.len = bytes_read;
        bytes_sent = send(socket, &packet, sizeof(Packet), NO_FLAG);
        if (bytes_sent < 0) {
            PERROR("send()");
            return FAIL;
        }
    }

    packet.remaining_packets = 0;
    packet.len = bytes_read;

    if (send(socket, &packet, sizeof(Packet), NO_FLAG) < 0) {
        PERROR("send()");
        return FAIL;
    }

    if (ferror(file)) {
        PERROR("fread()");
        return FAIL;
    }

    return PASS;
}

// return > 0 PASS
// return <= 0 FAIL
int send_cmd(int socket, char *cmd)
{
    size_t len = strlen(cmd) + 1;

    Packet packet = {
        .type = J_CMD,
        .remaining_packets = 0,
        .len = len,
        .data = {0},
    };

    assert(len < DATA_SIZE);
    memcpy(packet.data, cmd, len);
    ssize_t bytes_sent = send(socket, &packet, sizeof(Packet), NO_FLAG);
    if (bytes_sent < 0) {
        PERROR("send()");
        return FAIL;
    }

    return PASS;
}

int recv_file(int socket, Packet *packet, FILE *file)
{
    ssize_t bytes_recv = -1;

    fwrite(packet->data, sizeof(uint8_t), packet->len, file);
    if (ferror(file)) {
        return FAIL;
    }

    while(packet->remaining_packets != 0) {
        if ((bytes_recv = recv(socket, packet, sizeof(Packet), NO_FLAG)) < 0) {
            PERROR("recv()");
            return FAIL;
        }

        fwrite(packet->data, sizeof(uint8_t), packet->len, file);
        if (ferror(file)) {
            PERROR("fwrite()");
            return FAIL;
        }
    }
    return PASS;
}

FILE *recv_cmd(Packet *packet)
{
    return popen((char *) packet->data, "r");
}

#endif // ! __JOB__H__

