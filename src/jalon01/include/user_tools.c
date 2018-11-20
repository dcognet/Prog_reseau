#include "user_tools.h"


//Struct user-------------------------------------------------------------------

struct user{
  char pseudo[MSG_SIZE]; //pseudo du client
  int fd; // fd de la socket corespondante au client
  struct user *next; //pointeur sur le client suivant
  struct tm *date; // date et heure à laquelle le client s'est connecté
  char *ip; //adresse ip du client
  char channel_name[MSG_SIZE]; // nom du channel auquel il appartient
  u_short port;  //port avec lequel du client s'est connecté au serveru
  int send_to; // fd de la socket correspodante au client à qui on envoie le fichier
  int receive_from; // fd de la socket correspodante au client qui nous envoie le fichier
};

//création d'un nouvel utilisateur-------------------------------------------------------------

struct user *user_create(int fd,struct sockaddr_in *pointeur_host_addr){

  struct user *new_user = malloc(sizeof(struct user));
  time_t date = time(NULL);
  struct tm *pointeur_date = localtime(&date);

  new_user->fd = fd;
  new_user->next = NULL;
  new_user->date = pointeur_date;
  new_user->port = pointeur_host_addr->sin_port;
  new_user->ip = inet_ntoa(pointeur_host_addr->sin_addr);
  strcpy(new_user->pseudo,"Unspecified pseudo");
  strcpy(new_user->channel_name,"Unspecified channel");
  new_user->send_to=-1;
  new_user->receive_from=-1;
  return new_user;
}

//retourne la taille de la liste des utilisateur----------------------------------------------

int user_list_size(struct user *user_list){
  int size = 0;
  if (user_list==NULL){
    return 0;
  }
  while (user_list!=NULL){
    size++;
    user_list=user_list->next;
  }
  return size;
}

//ajoute un nouvel utilisateur--------------------------------------------------------------

struct user *user_add(struct user *user,int fd,struct sockaddr_in *pointeur_host_addr){

  struct user *new_user = user_create(fd,pointeur_host_addr);

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

//Change le nom d'un utilisateur--------------------------------------------------

struct user *user_change_pseudo(struct user *user,char pseudo[],int fd){
  struct user *temp = user;

  if(temp->fd == fd){
    strcpy(temp->pseudo,pseudo);
  }
  while(temp != NULL){
    if(temp->fd == fd){
      memset(temp->pseudo,'\0',MSG_SIZE);
      strcpy(temp->pseudo,pseudo);
    }
    temp=temp->next;
  }
  return user;
}

//affiche la liste des utilisateurs -----------------------------------------------------

char *user_display_list(struct user *list_user,int fd){
  char *buffer = malloc(sizeof(char)*MSG_SIZE);
  strcpy(buffer,"Online users are :");
  char pseudo[MSG_SIZE];
  memset(pseudo,'\0',MSG_SIZE);

  while (list_user != NULL){
    strcat(pseudo,"\n -");
    strcat(pseudo,list_user->pseudo);
    list_user = list_user->next;
  }
  strcat(buffer,pseudo);
  return buffer;
}

//retourne le pseudo de l'utilisateur--------------------------------------------------------

char *user_pseudo(struct user *user_list,int fd){

  if (user_list == NULL){
    return NULL;
  }

  while (user_list != NULL){
    if(user_list->fd == fd){
      return user_list->pseudo;
    }
    user_list = user_list->next;
  }
  return NULL;
}


//cherche si un pseudo est deja utilisé----------------------------------------

int user_look_for_pseudo(struct user *user_list,char *pseudo){

  if (user_list == NULL){
    return 1;
  }

  while (user_list != NULL){
    if(strcmp(user_list->pseudo,pseudo) == 0){
      return 1;
    }
    user_list = user_list->next;
  }

  return 0;
}

//retourne les informations concernant l'utilisateur -----------------------------------------------------------------

char *user_connexion_information(int fd, struct user *user_list,char pseudo[]){

  char *buffer = malloc(sizeof(char)*MSG_SIZE);

  while (user_list != NULL){
    if(strcmp(pseudo,user_list->pseudo) == 0){
      struct tm *date = user_list->date;

      sprintf(buffer,"%s connected since %d-%02d-%02d@%02d:%02d with IP address %s and port number %d",
      pseudo,
      date->tm_year + 1900,
      date->tm_mon + 1,
      date->tm_mday,
      date->tm_hour,
      date->tm_min,
      user_list->ip,
      user_list->port);
      return buffer;
    }
    user_list=user_list->next;
  }
  strcpy(buffer,"Aucun utilisateur ne possède cet identifiant");
  return buffer;
}

//supprime un utilisateur----------------------------------------------------------------

struct user *delete_user(struct user *user_list,int fd){

  struct user *temp = user_list;

  if (user_list == NULL){
    return NULL;
  }

  //Supprimer premier maillon

  if(temp->fd == fd){
    free(temp);
    return temp->next;
  }

  while(temp->next != NULL){

    //Supprimer dernier maillon

    if ((temp->next)->next == NULL && (temp->next)->fd == fd) {
      free(temp->next);
      temp->next = NULL;
      break;
    }

    //Supprimer maillon entre le premier et le dernier

    if((temp->next)->fd == fd){
      free(temp->next);
      temp->next=(temp->next)->next;
    }
    temp = temp->next;
  }
  return user_list;
}

//retourne l'utilisateur qui possède le fd passé en argument
struct user *user_look_for_user(struct user *user_list,int fd){
  if (user_list == NULL){
    return NULL;
  }
  while (user_list != NULL){
    if(user_list->fd == fd){
      return user_list;
    }
    user_list = user_list->next;
  }
  return NULL;
}


//retourne 0 si l'utilisateur n'est pas dans un channel
int user_appartient_channel(struct user *user){
  if(strcmp(user->channel_name,"Unspecified channel") == 0){
    return 1;
  }
  return 0;
}

//
struct user *user_change_name_channel(struct user *user,char *channel_name,int fd){
  struct user *temp = user;

  if(temp->fd == fd){
    memset(temp->channel_name,'\0',MSG_SIZE);
    strcpy(temp->channel_name,channel_name);
  }
  while(temp != NULL){
    if(temp->fd == fd){
      memset(temp->channel_name,'\0',MSG_SIZE);
      strcpy(temp->channel_name,channel_name);
    }
    temp = temp->next;
  }
  return user;
}

// retourne le fd d'un utilisateur
int user_fd(struct user *user){
  return user->fd;
}

//return the next user
struct user *user_next(struct user *user){
  return user->next;
}

char *user_channel_name(struct user *user){
  return user->channel_name;
}


//Retourne le fd correspant au l'utilisateur le pseudo passé en argument--------------------------------------------------------

int search_user_fd(struct user *user_list,char pseudo[]){
  if (user_list==NULL){
    return -1;
  }
  while (user_list!=NULL){
    if(strcmp(user_list->pseudo,pseudo)){
      return user_list->fd;
    }
    user_list=user_list->next;
  }

  return -1;
}

//renseigne à l'envoyeur le fd de la socket correspondante au receveur (pour le cas de l'envoie de fichier)

struct user *user_change_send_to(struct user *user,char pseudo[],int fd){

  struct user *temp = user;

  if(temp->fd==fd){
    temp->send_to=search_user_fd(user,pseudo);
  }
  while(temp!=NULL){
    if(temp->fd==fd){
      temp->send_to=search_user_fd(user,pseudo);
    }
    temp=temp->next;
  }

  return user;
}

//renseigne au recpeteur le fd de la socket correspondante à l'envoyeur (pour le cas de l'envoie de fichier)
struct user *user_change_receive_from(struct user *user,char pseudo[],int fd){

  struct user *temp = user;

  if(strcmp(temp->pseudo,pseudo)){
    temp->receive_from=fd;
  }
  while(temp!=NULL){
    if(strcmp(temp->pseudo,pseudo)==0){
      temp->receive_from=fd;
    }
    temp=temp->next;
  }

  return user;
}

// retourne le fd  de la socket du recepteur (pour le cas de l'envoie de fichier)
int user_send(struct user *user_list,int fd){
  if (user_list == NULL){
    return -1;
  }
  while (user_list != NULL){
    if(user_list->fd == fd){
      return user_list->send_to;
    }
    user_list = user_list->next;
  }
  return -1;
}

// retourne le fd  de la socket de l'envoyeur (pour le cas de l'envoie de fichier)
int user_receive_from(struct user *user_list,int fd){
  if (user_list == NULL){
    return -1;
  }
  while (user_list != NULL){
    if(user_list->fd == fd){
      return user_list->receive_from;
    }
    user_list = user_list->next;
  }
  return -1;
}

// retourne le port d'un client
int user_port(struct user *user_list,int fd){
  if (user_list==NULL){
    return -1;
  }
  while (user_list!=NULL){
    if(user_list->fd==fd){
      return user_list->port;
    }
    user_list=user_list->next;
  }
  return -1;
}
