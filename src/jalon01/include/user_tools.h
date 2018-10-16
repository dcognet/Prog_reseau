#include "server_tools.h"


#ifndef _USER_TOOLS_H_
#define _USER_TOOLS_H_

#include "server_tools.h"

//Struct user-------------------------------------------------------------------

struct user;

//Create a new user-------------------------------------------------------------

struct user *create_user(int fd,struct sockaddr_in *pointeur_host_addr);

//Return the size of the user list----------------------------------------------

int user_list_size(struct user *user_list);
//Add une new user--------------------------------------------------------------

struct user *user_add(struct user *user,int fd,struct sockaddr_in *pointeur_host_addr);

//Change the pseudo of an user--------------------------------------------------

struct user *user_change_pseudo(struct user *user,char pseudo[],int fd);

//Display the list of user -----------------------------------------------------

int display_user_list(struct user *list_user,int fd);

//Return the user pseudo--------------------------------------------------------

char *user_pseudo(struct user *user_list,int fd);

char *search_user_fd(struct user *user_list,char pseudo[]);


struct user *delete_user(struct user *user_list,int fd);


//Date afficher-----------------------------------------------------------------

int user_date_connexion(int fd, struct user *user_list,char pseudo[]);

int user_fd(struct user *user_list);

struct user *user_next(struct user *user_list);

struct user *user_change_send_to(struct user *user,char pseudo[],int fd);

struct user *user_change_receive_from(struct user *user,char pseudo[],int fd);


int user_send(struct user *user_list,int fd);

int user_receive_from(struct user *user_list,int fd);





#endif
