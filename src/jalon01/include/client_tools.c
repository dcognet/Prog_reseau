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

void error(const char *msg)
{
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


void do_connect(int socket, struct sockaddr_in pointeur_serv_addr){
  int i = connect(socket, (struct sockaddr*) &pointeur_serv_addr, sizeof(struct sockaddr));
  if(i == -1){
    error("ERROR connexion client");
  }
}

//------------------------------------------------------------------------------


void handle_client_message(int socket, const void *message){
  int i = send(socket,message,strlen(message),0);
  if(i == -1){
    error("ERROR envoi message server");
  }
}

//------------------------------------------------------------------------------

void do_read(int socket, char *buffer){
  memset (buffer, '\0', MSG_SIZE);
  int i = read(socket,buffer,MSG_SIZE);
  if(i == -1){
    error("ERROR read server");
  }
}
