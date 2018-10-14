#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>

#ifndef _SERVER_TOOLS_H_
#define _SERVER_TOOLS_H_

#define MSG_SIZE 256


//Fonctions---------------------------------------------------------------------
void error(const char *msg);

int do_socket(int domain, int type, int protocol);

void get_addr_info(const char* port, struct sockaddr_in* serv_addr);

void do_bind(int socket, const struct sockaddr_in pointeur_serv_addr);

void listen_client(int socket, int backlog);

int do_accept(int socket, struct sockaddr_in *pointeur_host_addr);

int do_read(int socket, char *buffer);

void do_write(int fd, const void *buffer);

void close_socket(int socket);

#endif
