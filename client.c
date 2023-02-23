#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <assert.h>

//#include <bluetooth/bluetooth.h>
//#include <bluetooth/rfcomm.h>

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
   fprintf(stderr, "Usage: %s <addr> <port> [-f <file>] [-c <command>] [-l : log enable]\n", arg0);
}

void parse_args(unsigned int argc, char **argv, Job jobs[argc], size_t *nb_job)
{
    JobType type = UNKNOWN;
    *nb_job = 0;

    for (unsigned int i = 3; i < argc; i++) {
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

    if (argc < 3) {
        usage();
        exit(1);
    }

    char *addr = argv[1];
    char *stopped;
    uint16_t port = strtol(argv[2], &stopped, 10);

    if (*stopped != '\0') {
        EPRINTF("Bad input: <%s> isn't a valid port\n", argv[2]);
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

    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock == INVALID_SOCKET) {
        PERROR("socket()");
        exit(99);
    }

    sockaddr_in sin;
    sin.sin_addr.s_addr = inet_addr(addr);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    LOG("%s : %s:%s\n", "Try to connect to ", argv[1], argv[2]);
    if(connect(sock, (sockaddr *) &sin, sizeof(sockaddr)) == SOCKET_ERROR) {
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

