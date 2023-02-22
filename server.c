#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>

#include "job.h"




void timestamp_filename(char filename[64])
{
    time_t t = time(NULL);
    snprintf(filename, 64, "tmp/tmp_%lu.out", t);
}
// return == 1 PASS
// return == 0 FAIL
int case_JFILE(int socket, Packet *packet)
{
    char filename[64];
    timestamp_filename(filename);

    LOG("Create : %s\n", filename);

    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        PERROR("fopen()");
        return FAIL;
    }

    LOG("%s\n", "Receiving file...");
    if (recv_file(socket, packet, file) == FAIL) {
        EPRINTF("%s\n", "recv_file");
        fclose(file);
        return FAIL;
    }

    LOG("%s\n", "File received");
    fclose(file);
    return PASS;
}

// return == 1 PASS
// return == 0 FAIL
int case_JCMD(int socket, Packet *packet)
{
    FILE *console = recv_cmd(packet);

    LOG("$ %s\n", packet->data);
    if (console == NULL) {
        EPRINTF("%s\n", "recv_cmd");
        return FAIL;
    }

    if (send_file(socket, console) == FAIL) {
        EPRINTF("%s\n", "send_file");
        return FAIL;
    }

    pclose(console);
    return PASS;
}

// return > 0 when OK
// return == 0 when J_CLOSE
// return < 0 when error
int recv_job(int socket)
{
    Packet packet;

    // Receive
    if ((recv(socket, &packet, sizeof(Packet), NO_FLAG)) < 0) {
        PERROR("recv()");
        return -1;
    }

    JobType type = packet.type;
    int err = 0;
    switch (type) {
    case J_FILE:
        LOG("%s\n", "Downloading a file...");
        err = case_JFILE(socket, &packet);
        break;
    case J_CMD:
        LOG("%s\n", "Executing a cmd");
        err = case_JCMD(socket, &packet);
        break;
    case J_CLOSE:
        return 0;
    default:
        return -1;
    }

    return err == FAIL ? -1 : 1;
}


void run(int socket)
{
    pid = getpid();
    LOG("%s\n", "New connexion established");
    int has_work = -1;

    do {
        LOG("%s\n", "Waiting for a new job");
        has_work = recv_job(socket);
    } while(has_work > 0);

    if (has_work < 0) {
        EPRINTF("%s\n", "recv_job()");
    }

    LOG("%s\n", "The connection has been closed");
    close(socket);
    exit(0);
}

void parse_arg(int argc, char **argv, uint16_t *port, uint32_t *nb_client)
{
    char *stopped;

    if(argc < 3 || argc > 5) {
        goto usage;
    }

    *port = strtol(argv[1], &stopped, 10);
    if (*stopped != '\0') {
        EPRINTF("Bad input: <%s> isn't a valid port\n", argv[1]);
        goto usage;
    }

    *nb_client = strtol(argv[2], &stopped, 10);
    if (*stopped != '\0') {
        EPRINTF("Bad input: <%s> isn't a number of client\n", argv[2]);
        goto usage;
    }

    if (argc == 4) {
        if (argv[3][0] == '-' && argv[3][1] == 'l') {
            f_log = 1;
        } else {
            EPRINTF("Bad input: <%s> isn't a valid option\n", argv[3]);
            goto usage;
        }
    }

    return;
usage:
    EPRINTF("Usage: %s <port: uint16_t> <nb_client: uint32_t> [-l : server log]\n", argv[0]);
    exit(1);
}

int main(int argc, char **argv)
{
    pid = getpid();
    uint32_t nb_client;
    uint16_t port;
    parse_arg(argc, argv, &port, &nb_client);

    int main_socket;
    sockaddr_in main_sin;
    int client_socket;

    main_socket = socket(AF_INET, SOCK_STREAM, NO_FLAG);
    if (main_socket == INVALID_SOCKET) {
        PERROR("socket()");
        exit(99);
    }

    // Listen
    main_sin.sin_addr.s_addr = htonl(INADDR_ANY); //Accepte toute les ips
    main_sin.sin_family = AF_INET;
    main_sin.sin_port = htons(port);
    if (bind(main_socket, (sockaddr *) &main_sin, sizeof main_sin) == SOCKET_ERROR) {
        PERROR("bind()");
        exit(98);
    }

    // Infinit loop
    for(;;) {
        // Waiting for a connexion


        LOG("%s\n", "Waiting for a new client");
        if (listen(main_socket, nb_client) == SOCKET_ERROR) {
            PERROR("listen()");
            exit(97);
        }

        // Init a connexion with a new client
        unsigned int sin_size = sizeof main_sin;
        if ((client_socket = accept(main_socket, (sockaddr *) &main_sin, &sin_size)) == INVALID_SOCKET) {
            PERROR("accept()");
            exit(96);
        }

        switch (fork()) {
        case -1:
            PERROR("fork()");
            exit(99);
        case 0:
            close(main_socket);
            run(client_socket);
            exit(0);
        default:
            break;
        }
        close(client_socket);
    }
    close(main_socket);
    return 0;
} // end main

