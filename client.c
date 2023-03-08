#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <assert.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <netdb.h>

#include "job.h"

#define IS_PARAM(str) (str[0] == '-' && str[1] != '\0' && str[2] == '\0')
static char *arg0;

// return == 1 PASS
// return == 0 FAIL
int case_J_FILE(int socket, Job *job)
{
    LOG("%s : %s\n", "Openning", job->str);
    char *path = job->str;
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        PERROR("fopen()");
        return FAIL;
    }

    LOG("%s\n", "Sending the file...");
    if (send_file(socket, file) == FAIL) {
        EPRINTF("%s\n", "send_file()");
        return FAIL;
    }

    return PASS;
}

int case_J_CMD(int socket, Job *job)
{
    if (send_cmd(socket, job->str) == FAIL) {
        EPRINTF("%s\n", "send_cmd");
        return FAIL;
    }
    printf("$ %s\n", job->str);

    Packet packet = {0};
    if ((recv(socket, &packet, sizeof(Packet), NO_FLAG)) < 0) {
        EPRINTF("%s\n", "recv_file");
        return FAIL;
    }

    if ((recv_file(socket, &packet, stdout)) == FAIL) {
        EPRINTF("%s\n", "recv_file");
        return FAIL;
    }

    return PASS;
}

// return > 0 PASS
// return == 0 STOP
// return < 0 FAIL
int send_job(int socket, Job *job)
{
    JobType type = job->type;
    int err = -1;

    switch (type) {
    case J_FILE:
        err = case_J_FILE(socket, job);
        break;
    case J_CMD:
        err = case_J_CMD(socket, job);
        break;
    default:
        return -1;
    }
    return err == FAIL ? -1 : 1;
}
void usage()
{
   fprintf(stderr, "Usage: %s MODE <addr> <port/chan> [-f <file>] [-c <command>] [-l : log enable]\n", arg0);
   fprintf(stderr, "MODE:\n");
   fprintf(stderr, "  -b : bluetooth\n");
   fprintf(stderr, "  -i : ip\n");
   exit(1);
}

void parse_args(unsigned int argc, char **argv, Job jobs[argc], size_t *nb_job)
{
    JobType type = UNKNOWN;
    *nb_job = 0;

    if (argv[1][0] == '-') {
        if (argv[1][1] == 'b') {
            f_mode = BLUETOOTH;
        } else if (argv[1][1] == 'i') {
            f_mode = IP;
        } else {
            fprintf(stderr, "Bad input: <%s> isn't a valid MODE\n", argv[1]);
            usage();
        }
    } else {
        fprintf(stderr, "Bad input: <%s> isn't a valid MODE\n", argv[1]);
        usage();
    }

    for (unsigned int i = 4; i < argc; i++) {
        char *arg = argv[i];
        if (IS_PARAM(arg)) {
            switch (arg[1]) {
            case 'f':
                type = J_FILE;
                break;
            case 'c':
                type = J_CMD;
                break;
            case 'l':
                f_log = 1;
                break;
            default:
                fprintf(stderr, "Unknown parameter: %s\n", arg);
                usage();
                exit(3);
            }
        } else if (type ) {
            if (type == UNKNOWN) {
                fprintf(stderr, "Unexpected argument: %s\n", arg);
                usage();
                exit(4);
            } else {
                jobs[*nb_job] = (Job) {
                    .type = type, .str = arg
                };
                *nb_job += 1;
            }
        }
    }
}


int main(int argc, char **argv)
{
    pid = getpid();
    arg0 = argv[0];

    if (argc < 4) {
        usage();
        exit(1);
    }

    char *addr = argv[2];
    char *stopped;
    uint16_t port = strtol(argv[3], &stopped, 10);

    if (*stopped != '\0') {
        EPRINTF("Bad input: <%s> isn't a valid port\n", argv[3]);
        usage();
        exit(2);
    }

    Job jobs[argc];
    size_t nb_job = 0;

    parse_args(argc, argv, jobs, &nb_job);
    if (nb_job == 0) {
        EPRINTF("%s\n","No jobs found");
        usage();
        exit(77);
    }

    
    sockaddr_in ip_sin = {0};
    sockaddr_rc bt_sin = {0};

    sockaddr *sin = NULL;
    size_t size_sin = {0};
    int sock = -1;

    if (setup(&sin, &sock, &size_sin, &ip_sin, &bt_sin, port) < 0) {
        exit(99);
    }
    
    switch (f_mode) {
        case IP:
            ip_sin.sin_addr.s_addr = inet_addr(addr);
            break;    
        case BLUETOOTH:
            str2ba(argv[2], &bt_sin.rc_bdaddr);
            break;
    }

    LOG("%s : %s:%s\n", "Try to connect to ", argv[2], argv[3]);
    if (connect(sock, sin, size_sin) == SOCKET_ERROR) {
        PERROR("connect()");
        exit(98);
    }
    LOG("%s\n", "The connection has been established");

    for (unsigned int i = 0; i < nb_job; i++) {
        send_job(sock, &jobs[i]);
    }

    // Close
    Packet eot = {.type = J_CLOSE, 0};
    if (send(sock, &eot, sizeof(Packet), NO_FLAG) < 0) {
        PERROR("send()");
        return 1;
    }

    close(sock);
    LOG("%s\n", "The connection has been closed");
    return 0;
}

