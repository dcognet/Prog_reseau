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

#ifndef _SERVER_CHANNEL_H_
#define _SERVER_CHANNEL_H_

#define nb_co_max 20

struct channel;

struct channel *channel_create(char *channel_name);

int channel_list_size(struct channel *channel_list);

struct channel *channel_add(struct channel * channel_list, char *channel_name);

int channel_look_for_name(struct channel *channel_list,char *name_channel);

struct channel *channel_up_number_member(struct channel * channel_list, char *channel_name);

struct channel *channel_down_number_member(struct channel * channel_list, char *name_channel);

struct channel *channel_delete(struct channel *channel_list,char *channel_name);

struct channel *channel_look_for_channel(struct channel *channel_list,char *channel_name);

int channel_nombre_membre(struct channel* channel);

char *channel_display_list(struct channel *channel_list);

#endif
