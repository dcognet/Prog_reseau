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
#include "user_tools.h"

#ifndef _SERVER_CAST_H_
#define _SERVER_CAST_H_



//Fonctions---------------------------------------------------------------------

int broadcast(int sender_fd, const void *buffer, struct user *list_user);

int unicast(int sender_fd, const void *buffer, struct user *list_user, char receiver_pseudo[]);

int multicast(int sender_fd, const void *buffer, struct user *list_user, char *channel_name);

#endif
