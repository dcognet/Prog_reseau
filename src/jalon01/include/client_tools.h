
#ifndef _CLIENT_TOOLS_H_
#define _CLIENT_TOOLS_H_

#define MSG_SIZE 256


//Fonctions---------------------------------------------------------------------

void error(const char *msg);
//------------------------------------------------------------------------------


int do_socket(int domain, int type, int protocol);


//------------------------------------------------------------------------------


void get_addr_info(const char* port, struct sockaddr_in* serv_addr,const char* host) ;

//------------------------------------------------------------------------------


void do_connect(int socket, struct sockaddr_in pointeur_serv_addr);

//------------------------------------------------------------------------------


void handle_client_message(int socket, char *message);

//------------------------------------------------------------------------------

int do_read(int socket, char *buffer);

#endif
