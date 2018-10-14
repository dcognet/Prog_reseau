#include "server_cast.h"


int broadcast(int sender_fd, const void *buffer, struct user *list_user){
  if (list_user==NULL)
  return 0;

  while (list_user!=NULL){
    if(user_fd(list_user)!=sender_fd){
      do_write(user_fd(list_user),buffer);

    }
    list_user=user_next(list_user);
  }
  return 1;
}

int unicast(int sender_fd, const void *buffer, struct user *list_user, char receiver_pseudo[]){
  if (list_user==NULL)
  return 0;

  while (list_user!=NULL){
    if(strcmp(receiver_pseudo,user_pseudo(list_user,user_fd(list_user)))==0){
      printf("ok\n");
      do_write(user_fd(list_user),buffer);
      break;
    }
    list_user=user_next(list_user);
  }
  return 1;
}
