#include "server_cast.h"

//------------------------------------------------------------------------------

int broadcast(int sender_fd, const void *buffer, struct user *list_user){
  if (list_user == NULL)
  return 0;

  while (list_user != NULL){
    if(user_fd(list_user) != sender_fd){
      do_write(user_fd(list_user),buffer);

    }
    list_user = user_next(list_user);
  }
  return 1;
}

//------------------------------------------------------------------------------

int unicast(const void *buffer, struct user *list_user, char receiver_pseudo[]){
  if (list_user == NULL)
  return 0;

  while (list_user != NULL){
    if(strcmp(receiver_pseudo,user_pseudo(list_user,user_fd(list_user))) == 0){
      do_write(user_fd(list_user),buffer);
      break;
    }
    list_user = user_next(list_user);
  }
  return 1;
}

//------------------------------------------------------------------------------

int multicast(int sender_fd, char *buffer, struct user *list_user, char *channel_name){
  if (list_user == NULL)
  return 0;

  char envoie[MSG_SIZE];
  sprintf(envoie,"[%s] [%s] %s",channel_name,user_pseudo(list_user,sender_fd),buffer);

  while (list_user != NULL){
    if(user_fd(list_user) != sender_fd && strcmp(user_channel_name(list_user),channel_name) == 0){
      do_write(user_fd(list_user),envoie);
    }
    list_user = user_next(list_user);
  }
  return 1;
}
