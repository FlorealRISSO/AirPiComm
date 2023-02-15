#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define IPDEFAULT 130.120.8.80
#define PORTDEFAULT 5555
#define closesocket( socket ) close( socket );
#define DATA_SIZE 256

#define IS_PARAM(str) (str[0] == '-' && str[1] != '\0' && str[2] == '\0')

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

typedef enum {
	EOT	 = 0, // End Of Transmission
	PART = 1,
	DATA = 2,
	CMD	 = 3,
	UNKNOWN = 0xFFFFF,
} Type;

typedef struct {
	uint32_t type;
	uint32_t len;
	unsigned char data[DATA_SIZE];
} Packet;

typedef struct {
	uint32_t type;
	char *str;
} Job;

static char* arg0;

void usage(){
	printf("Usage: %s <addr> <port> [-f <file>] [-c <command>]\n", arg0);
}

void parse_args(unsigned int argc, char **argv, Job jobs[argc], size_t *nb_job) {
	Type type = UNKNOWN;
	*nb_job = 0;

	for (unsigned int i = 3; i < argc; i++) {
		char* arg = argv[i];
		if (IS_PARAM(arg)) {
			switch (arg[1]) {
				case 'f':
					type = DATA;
					break;
				case 'c':
					type = CMD;
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
				jobs[*nb_job] = (Job) {.type = type, .str = arg};
				*nb_job += 1;
			}
		}
	}
}


int main(int argc, char **argv) {
	arg0 = argv[0];

	if (argc < 5) {
		usage();
		exit(1);
	}

    char *addr = argv[1];
	char *stopped;
	uint16_t port = strtol(argv[2], &stopped, 10);

	if (*stopped != '\0') {
		fprintf(stderr, "Bad input: <%s> isn't a valid port\n", argv[2]);
		usage();
		exit(2);
	}

	Job jobs[argc];
	size_t nb_job = 0;
	parse_args(argc, argv, jobs, &nb_job);

#ifdef DEBUG
	printf("addr: %s, port: %u\n", addr, port);
	printf("jobs[%ld] {\n", nb_job);
	for(size_t i = 0; i < nb_job; i++) {
		printf("\t {.type = %d, .str = %s}\n", jobs[i].type, jobs[i].str);
	}
	printf("};\n");
#endif
#ifdef ___NOP___

    /* Creation socket TCP */
    SOCKET sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock==INVALID_SOCKET){
      perror("socket()");
      exit(99);
    }

    /* Create the socket */
    SOCKADDR_IN sin;
    sin.sin_addr.s_addr=inet_addr(ip); // ou htonl(ip);
    sin.sin_family=AF_INET;
    sin.sin_port = htons(port);

    /* Create the connexion */
    printf("Connexion...\n");
    if(connect(sock,(SOCKADDR*)&sin,sizeof(SOCKADDR))==SOCKET_ERROR){
      perror("connect()");
      exit(99);
    }
    printf("Connected\n");

    do {

		// Send
        if(send(sock,message,6,0)<0){
          perror("send()");
          exit(99);
        }

		// Receve
        char recu[2];
        if((recv(sock,recu,2,0))<0){
          perror("recv()");
          exit(99);
        }

    } while (res);

    closesocket(sock);
	printf("End...\n");
#endif
    return 0;
}

