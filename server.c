/* =================================================================== */
// Progrmame Serveur qui calcule le résultat d'un coup joué à partir
// des coordonnées reçues de la part d'un client "joueur".
// Version CONCURRENTE : N clients/joueurs à la fois
/* =================================================================== */

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define MY_PORT 5555
#define NB_CLIENT 5
#define closesocket( socket ) close( socket );

typedef struct Jeu_s{
  int taille;
  int x_tresor;
  int y_tresor;
} Jeu;

/**
  Creation d'un tableau de jeu
*/
void initJeu(Jeu * plateauJeu,int taille){
  plateauJeu->taille=taille;
  plateauJeu->x_tresor=1+rand()%(taille);
  plateauJeu->y_tresor=1+rand()%(taille);
}



/* =================================================================== */
/* FONCTION PRINCIPALE : SERVEUR CONCURRENT                            */
/* =================================================================== */


void run(int csock){
  srand(time(NULL));
  pid_t pid=getpid();
  Jeu plateauJeu;
  initJeu(&plateauJeu,N);
  printf("[%d]taille=%d,x=%d,y=%d\n",pid,plateauJeu.taille,plateauJeu.x_tresor,plateauJeu.y_tresor);
  int resultat=-1;
  do {
    //Dans la boucle
    /* Réception du resultat du coup (recv) */
    char recu[6];
    if((recv(csock,recu,6,0))<0){
      perror("recv()");
      exit(99);
    }
    /* Deserialisation du résultat en deux entiers */
    int xp,yp;
    sscanf(recu,"%d %d",&xp,&yp);
    #if PRINTINFO==1
      printf("[%d]Valeur recu : x{%d} y{%d}\n",pid,xp,yp);
    #endif
    /* Recherche du résultat dans le plateau */
    int resultat = recherche_tresor(plateauJeu.taille,plateauJeu.x_tresor,plateauJeu.y_tresor,xp,yp);

    char message[2]; // Taille max d'un message
    sprintf(message,"%d",resultat);
    #if PRINTINFO==1
      printf("[%d]Valeur envoyé : message{%s}\n",pid,message);
    #endif
    /* Envoi de la requête au serveur (send) */
    if(send(csock,message,2,0)<0){
      perror("send()");
      exit(99);
    }
  } while(resultat);
  closesocket(csock);
  exit(0);
}




int main(int argc, char **argv) {
  //_______________________________
  SOCKET sock;
  SOCKADDR_IN sin;
  SOCKADDR_IN csin;
  SOCKET csock;
  //Création du socket
  sock=socket(AF_INET,SOCK_STREAM,0);
  if(sock==INVALID_SOCKET){
    perror("socket()");
    exit(99);
  }
  //Création de l'interface d'écoute
  sin.sin_addr.s_addr=htonl(INADDR_ANY); //Accepte toute les ips
  sin.sin_family=AF_INET;
  sin.sin_port=htons(MY_PORT);
  if(bind(sock,(SOCKADDR*)&sin,sizeof sin)==SOCKET_ERROR){
    perror("bind()");
    exit(98);
  }
  while(1){
    //Ecoute d'une connexion
    if(listen(sock,NB_CLIENT)==SOCKET_ERROR){
      perror("listen()");
      exit(97);
    }

    //Connexion avec un client
    unsigned int sinsize=sizeof csin;
    if((csock=accept(sock,(SOCKADDR*)&csin,&sinsize))==INVALID_SOCKET){
      perror("accept()");
      exit(96);
    }
    switch (fork()) { // Cree un sous fils pour gerer le client
      case -1: //Si erreur
        perror("fork()");
        exit(99);
      case 0: // Cas fils
        closesocket(sock); // Ferme le socket d'ecoute, il lui est inutile
        run(csock); // Lance la fonction de jeu
        exit(0); // Se tue bien
      default: //Cas pere
        break;
    }
    closesocket(csock); // Ferme le socket du fils car il lui est inutile
  }
    closesocket(sock);
    return 0;
} // end main

