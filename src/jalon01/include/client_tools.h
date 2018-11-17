#include "message.h"


#ifndef _CLIENT_TOOLS_H_
#define _CLIENT_TOOLS_H_

#define MSG_SIZE 256


//Fonctions---------------------------------------------------------------------

void error(const char *msg);
//------------------------------------------------------------------------------


int do_socket(int domain, int type, int protocol);


//------------------------------------------------------------------------------


void get_addr_info(int port, struct sockaddr_in* serv_addr,const char* host) ;

//------------------------------------------------------------------------------


int do_connect(int socket, struct sockaddr_in pointeur_serv_addr);

//------------------------------------------------------------------------------


void handle_client_message(int socket, char *message);

//------------------------------------------------------------------------------

int do_read(int socket, char *buffer);

int do_read2(int socket, struct trame *trame);


void do_bind(int socket, const struct sockaddr_in pointeur_serv_addr);


//------------------------------------------------------------------------------

void listen_client(int socket, int backlog);

int do_accept(int socket, struct sockaddr_in *pointeur_host_addr);



#endif
