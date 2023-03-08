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
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "job.h"

static char *arg0;


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

void usage() 
{
     fprintf(stderr, "Usage: %s <MODE> <port/chan> <max_client> [-l : log]\n", arg0);
     fprintf(stderr, " MODE:\n");
     fprintf(stderr, "  -b :  bluetooth\n");
     fprintf(stderr, "  -i :  ip\n");
     exit(1);
}

void parse_arg(int argc, char **argv, uint16_t *port, uint32_t *nb_client)
{
    char *stopped;

    if(argc < 4 || argc > 5) {
        usage();
    }

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
    *port = strtol(argv[2], &stopped, 10);
    if (*stopped != '\0') {
        fprintf(stderr, "Bad input: <%s> isn't a valid port\n", argv[2]);
        usage();
    }

    *nb_client = strtol(argv[3], &stopped, 10);
    if (*stopped != '\0') {
        fprintf(stderr, "Bad input: <%s> isn't a number of client\n", argv[3]);
        usage();
    }

    if (argc == 5) {
        if (argv[4][0] == '-' && argv[4][1] == 'l') {
            f_log = 1;
        } else {
            fprintf(stderr, "Bad input: <%s> isn't a valid option\n", argv[3]);
	    usage();
        }
    }

    return;
}



int main(int argc, char **argv)
{
    arg0 = argv[0];
    pid = getpid();
    uint32_t nb_client;
    uint16_t port;
    parse_arg(argc, argv, &port, &nb_client);

    sockaddr_in ip_main_sin = {0};
    sockaddr_rc bt_main_sin = {0};

    sockaddr_in ip_client_sin = {0};
    sockaddr_rc bt_client_sin = {0};

    sockaddr *main_sin = NULL;
    sockaddr *client_sin = NULL;

    size_t size_sin = {0};
    size_t client_size_sin = {0};

    int main_socket;
    int client_socket;

    if (setup(&main_sin, &main_socket, &size_sin, &ip_main_sin, &bt_main_sin, port) < 0) {
        exit(99);
    }

    if ((bind(main_socket, main_sin, size_sin)) == SOCKET_ERROR) {
        PERROR("bind()");
        exit(99);
    }

    if (f_mode == BLUETOOTH) {
        client_sin = (sockaddr*) &bt_client_sin;
    } else {
        client_sin = (sockaddr*) &ip_client_sin;    
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
        if ((client_socket = accept(main_socket, client_sin, &client_size_sin)) == INVALID_SOCKET) {
            PERROR("accept()");
            exit(96);
        }

        if (client_size_sin != size_sin) {
            EPRINTF("%s\n", "Error, not the same protocol");
            exit(99);
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

