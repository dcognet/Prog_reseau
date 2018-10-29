#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include "client_tools.h"


#define MSG_SIZE 256


//Fonctions---------------------------------------------------------------------

void error(const char *msg){
  perror(msg);
  exit(1);
}

//------------------------------------------------------------------------------


int do_socket(int domain, int type, int protocol) {
  int socket1;
  int yes = 1;
  socket1 = socket(domain,type,protocol);
  if(socket1 == -1){
    error("ERROR socket1 creation");
  }
  if (setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    error("ERROR setting socket options");
  }
  return socket1;
}


//------------------------------------------------------------------------------


void get_addr_info(const char* port, struct sockaddr_in* serv_addr,const char* host) {
  int portno;
  memset(serv_addr,'\0',sizeof(serv_addr));
  portno = atoi(port);
  serv_addr->sin_family = AF_INET;
  inet_aton(host,&(serv_addr->sin_addr));
  serv_addr->sin_port = htons(portno);
}

//------------------------------------------------------------------------------


int do_connect(int socket, struct sockaddr_in pointeur_serv_addr){
  int i = connect(socket, (struct sockaddr*) &pointeur_serv_addr, sizeof(struct sockaddr));
  if(i == -1){
    error("ERROR connexion client");
  }
  return i;
}

//------------------------------------------------------------------------------


void handle_client_message(int socket, char *message){
  int i = send(socket,message,strlen(message),0);
  if(i == -1){
    error("ERROR envoi message server");
  }
}

//------------------------------------------------------------------------------

int do_read(int socket, char *buffer){
  memset (buffer,'\0', MSG_SIZE);
  int i = read(socket,buffer,MSG_SIZE);
  if(i == -1){
    error("ERROR read server");
  }
  return i;
}

void do_bind(int socket, const struct sockaddr_in pointeur_serv_addr){
  int i = bind(socket, (struct sockaddr*) &pointeur_serv_addr, sizeof(pointeur_serv_addr));
  if(i == -1){
    error("ERROR bind server");
  }
}


//------------------------------------------------------------------------------

void listen_client(int socket, int backlog){
  int i = listen(socket, backlog);
  if(i == -1){
    error("ERROR listen server");
  }
}

int do_accept(int socket, struct sockaddr_in *pointeur_host_addr){
  size_t host_addr_size = sizeof(struct sockaddr_in);
  int i = accept(socket, (struct sockaddr *)pointeur_host_addr,(socklen_t *)&host_addr_size);
  printf("Je me connecte avec l'adresse : %s\n",inet_ntoa(pointeur_host_addr->sin_addr));

  if(i == -1){
    error("ERROR accepte server");
  }
  return i;
}
