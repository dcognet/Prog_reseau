#include "user_tools.h"


//Struct user-------------------------------------------------------------------

struct user{
  char pseudo[MSG_SIZE];
  int fd;
  struct user *next;
  struct tm *date;
  u_short port;
  char *ip;
  int send_to;
  int receive_from;
};

//Create a new user-------------------------------------------------------------

struct user *create_user(int fd,struct sockaddr_in *pointeur_host_addr){

  struct user *new_user = malloc(sizeof(struct user));
  time_t date = time(NULL);
  struct tm *pointeur_date = localtime(&date);

  new_user->fd = fd;
  new_user->next = NULL;
  new_user->date = pointeur_date;
  new_user->port = pointeur_host_addr->sin_port;
  new_user->ip = inet_ntoa(pointeur_host_addr->sin_addr);
  new_user->send_to=0;
  new_user->receive_from=0;

  return new_user;
}

//Return the size of the user list----------------------------------------------

int user_list_size(struct user *user_list){
  int size=0;
  if (user_list==NULL){
    return 0;
  }
  while (user_list!=NULL){
    size++;
    user_list=user_list->next;
  }
  return size;
}

//Add une new user--------------------------------------------------------------

struct user *user_add(struct user *user,int fd,struct sockaddr_in *pointeur_host_addr){

  struct user *new_user = create_user(fd,pointeur_host_addr);

  if (user_list_size(user) == 0){
    new_user->next = user;
    return new_user;
  }

  else {
    struct user *temp = user;
    while(temp->next != NULL){
      temp = temp->next;
    }
    temp->next = new_user;
    return user;
  }
}

//Change the pseudo of an user--------------------------------------------------

struct user *user_change_pseudo(struct user *user,char pseudo[],int fd){

  struct user *temp = user;

  if(temp->fd==fd){
    strcpy(temp->pseudo,pseudo);
  }
  while(temp!=NULL){
    if(temp->fd==fd){
      memset(temp->pseudo,'\0',MSG_SIZE);
      strcpy(temp->pseudo,pseudo);
    }
    temp=temp->next;
  }

  return user;
}

//Display the list of user -----------------------------------------------------

int display_user_list(struct user *list_user,int fd){
  char buffer[MSG_SIZE]="[Server] Online users are :";
  char pseudo[MSG_SIZE]="";

  if (list_user==NULL){
    return 0;
  }
  memset(pseudo,'\0',MSG_SIZE);
  while (list_user!=NULL){
    strcat(pseudo,"\n -");
    strcat(pseudo,list_user->pseudo);
    list_user = list_user->next;
  }

  do_write(fd,strcat(buffer,pseudo));
  return 1;
}

//Return the user pseudo--------------------------------------------------------

char *user_pseudo(struct user *user_list,int fd){

  if (user_list==NULL){
    return NULL;
  }

  while (user_list!=NULL){
    if(user_list->fd==fd){
      return user_list->pseudo;
    }
    user_list=user_list->next;
  }

  return NULL;
}

//Return the user fd thank to the pseudo--------------------------------------------------------

char *search_user_fd(struct user *user_list,char pseudo[]){

  if (user_list==NULL){
    return NULL;
  }

  while (user_list!=NULL){
    if(strcmp(user_list->pseudo,pseudo)){
      return user_list->fd;
    }
    user_list=user_list->next;
  }

  return NULL;
}

//Delete an user----------------------------------------------------------------

struct user *delete_user(struct user *user_list,int fd){

  struct user *temp = user_list;

  if (user_list==NULL){
    return NULL;
  }

  //Supprimer premier maillon

  if(temp->fd==fd){
    return temp->next;
  }

  while(temp->next!=NULL){

    //Supprimer dernier maillon

    if ((temp->next)->next==NULL && (temp->next)->fd==fd) {
      temp->next=NULL;
      break;
    }

    //Supprimer maillon entre le premier et le dernier

    if((temp->next)->fd==fd){
      temp->next=(temp->next)->next;
    }
    temp=temp->next;
  }
  return user_list;
}

//Date afficher-----------------------------------------------------------------

int user_date_connexion(int fd, struct user *user_list,char pseudo[]){

  while (user_list!=NULL){
    if(strcmp(pseudo,user_list->pseudo)==0){
      char baffer[MSG_SIZE];
      struct tm *date = user_list->date;

      sprintf(baffer,"[Server] %s connected since %d-%02d-%02d@%02d:%02d with IP address %s and port number %d",
      pseudo,
      date->tm_year + 1900,
      date->tm_mon + 1,
      date->tm_mday,
      date->tm_hour,
      date->tm_min,
      user_list->ip,
      user_list->port);

      do_write(fd,baffer);
      return 0;
    }
    user_list=user_list->next;
  }
  do_write(fd,"Aucun utilisateur ne possède cet identifiant");
  return 1;
}

// return the fd of the user
int user_fd(struct user *user_list){
  return user_list->fd;
}

//return the next user
struct user *user_next(struct user *user_list){
  return user_list->next;
}


struct user *user_change_send_to(struct user *user,char pseudo[],int fd){

  struct user *temp = user;

  if(temp->fd==fd){
    temp->send_to=search_user_fd(user,pseudo);
  }
  while(temp!=NULL){
    if(temp->fd==fd){
printf("send ps\n" );
      temp->send_to=search_user_fd(user,pseudo);
    }
    temp=temp->next;
  }

  return user;
}

struct user *user_change_receive_from(struct user *user,char pseudo[],int fd){

  struct user *temp = user;

  if(strcmp(temp->pseudo,pseudo)){
    temp->receive_from=fd;
  }
  while(temp!=NULL){
    if(strcmp(temp->pseudo,pseudo)==0){
printf("send ps\n" );
      temp->receive_from=fd;
    }
    temp=temp->next;
  }

  return user;
}

int user_send(struct user *user_list,int fd){
  if (user_list==NULL){
    return NULL;
  }
  while (user_list!=NULL){
    if(user_list->fd==fd){
      return user_list->send_to;
    }
    user_list=user_list->next;
  }
  return NULL;
}

int user_receive_from(struct user *user_list,int fd){
  if (user_list==NULL){
    return NULL;
  }
  while (user_list!=NULL){
    if(user_list->fd==fd){
      return user_list->receive_from;
    }
    user_list=user_list->next;
  }
  return NULL;
}
