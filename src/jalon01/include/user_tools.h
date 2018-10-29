#include "server_tools.h"

#ifndef _USER_TOOLS_H_
#define _USER_TOOLS_H_


struct user;

struct user *user_create(int fd,struct sockaddr_in *pointeur_host_addr);

int user_list_size(struct user *user_list);

struct user *user_add(struct user *user,int fd,struct sockaddr_in *pointeur_host_addr);

struct user *user_change_pseudo(struct user *user,char pseudo[],int fd);

char *user_display_list(struct user *list_user,int fd);

char *user_pseudo(struct user *user_list,int fd);

struct user *delete_user(struct user *user_list,int fd);

char *user_connexion_information(int fd, struct user *user_list,char pseudo[]);

int user_fd(struct user *user);

struct user *user_next(struct user *user);

int user_look_for_pseudo(struct user *user_list,char *pseudo);

struct user *user_look_for_user(struct user *user_list,int fd);

int user_appartient_channel(struct user *user);

struct user *user_change_name_channel(struct user *user,char *channel_name,int fd);

char * user_channel_name(struct user *user);

#endif
