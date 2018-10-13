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

//Struct user-------------------------------------------------------------------

struct user{
  char pseudo[MSG_SIZE];
  int fd;
  struct user *next;
  struct tm *date;
  u_short port;
  char *ip;
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
  char buffer[MSG_SIZE]=" [Server] : Online users are :";
  char pseudo[MSG_SIZE];

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

      sprintf(baffer,"[Server] : %s connected since %d-%02d-%02d@%02d:%02d with IP address %s and port number %d\n",
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




//Corps-------------------------------------------------------------------------

int main(int argc, char** argv){

  if (argc != 2)
  {
    fprintf(stderr, "usage: RE216_SERVER port\n");
    return 1;
  }

  //Variables--------------------------------------------------------------

  int i, valeur,event_fd,new_socket;
  int nb_co = 0;
  int nb_co_max = 3;
  struct pollfd fds[200];
  char buffer[MSG_SIZE];
  char pseudo[MSG_SIZE];
  struct user *user_list = NULL;
  struct sockaddr_in *pointeur_host_addr = malloc(sizeof(struct sockaddr_in));
  char envoie[] = "[Serveur] : Welcome on the chat : ";

  //get the socket--------------------------------------------------------------
  printf("Etape : Création socket\n");
  int socket = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  //init the serv_add structure
  printf("Etape : Informations serveur\n");
  struct sockaddr_in pointeur_serv_addr;
  get_addr_info(argv[1], &pointeur_serv_addr);

  //perform the binding---------------------------------------------------------
  printf("Etape : Bind\n");
  do_bind(socket,pointeur_serv_addr);

  //listen for at most 20 concurrent client-------------------------------------
  printf("Etape : Ecoute\n");
  listen_client(socket,20);

  memset(fds,-1,sizeof(fds));
  fds[0].fd = socket;
  fds[0].events = POLLIN;

  while(1){

    // wait for an activity

    event_fd = poll(fds,nb_co_max+1,-1);

    for (i = 1; i <= 20; i++) {

      if(fds[0].revents == POLLIN){
        if(fds[i].fd == -1){

          //accept connection from client-------------------------------------------

          new_socket = do_accept(socket,pointeur_host_addr);
          user_list = user_add(user_list,new_socket,pointeur_host_addr);

          if(nb_co >= nb_co_max){

            // refuse the connection if there is too much client

            printf("Acceptation d'un nouveau client impossibla car trop de connection\n");
            do_write(new_socket,"Server cannot accept incoming connections anymore. Try again later.");
            close_socket(new_socket);
            break;
          }

          printf("Acceptation d'un nouveau client : client %i\n",i);
          fds[i].fd = new_socket;
          fds[i].events = POLLIN;
          nb_co++;
          printf("Nombre de connection = %i\n",nb_co);
          memset(buffer,'\0',MSG_SIZE);

          strcpy(buffer,"[Server] : please logon with /nick <your pseudo>");
          do_write(fds[i].fd,buffer);
          break;
        }
      }

      else{

        if(fds[i].revents == POLLIN){

          //read what the do_readclient has to say------------------------------
          memset(buffer,'\0',MSG_SIZE);

          valeur = do_read(fds[i].fd,buffer);
          printf("Le message reçu est: %s\n",buffer);

          //command /nick

          if(strncmp(buffer,"/nick",strlen("/nick")) == 0){
            memset(pseudo,'\0',MSG_SIZE);
            strcpy(pseudo,buffer+strlen("/nick "));
            printf("%s\n",pseudo);
            user_list = user_change_pseudo(user_list,pseudo,fds[i].fd);
            strcpy(envoie,"[Serveur] : Welcome on the chat : ");
            do_write(fds[i].fd,strcat(envoie,pseudo));
            break;
          }

          //command /who

          if(strcmp(buffer,"/who") == 0){
            display_user_list(user_list,fds[i].fd);
            break;
          }

          //command /who

          if(strncmp(buffer,"/whois",strlen("/whois")) == 0){
            memset(pseudo,'\0',MSG_SIZE);
            strcpy(pseudo,buffer+strlen("/whois "));
            user_date_connexion(fds[i].fd,user_list,pseudo);
            break;
          }

          //clean up client socket----------------------------------------------

          if(strncmp(buffer, "/quit",strlen("/quit")) == 0 || valeur == 0){
            user_list = delete_user(user_list,fds[i].fd);
            printf("Fermeture socket client\n");
            close_socket(fds[i].fd);
            fds[i].fd = -1;
            fds[i].events = -1;
            nb_co--;
            printf("Nombre de connection = %i\n",nb_co);
            break;
          }

          //we write back to the client-----------------------------------------

          do_write(fds[i].fd,buffer);
        }
      }
    }
  }

  //clean up server socket------------------------------------------------------

  printf("Fermeture socket serveur\n");
  close_socket(socket);
  return 0;

}

//------------------------------------------------------------------------------

void error(const char *msg){
  perror(msg);
  exit(1);
}

//------------------------------------------------------------------------------

int do_socket(int domain, int type, int protocol) {
  int socket1 = socket(domain,type,protocol);
  int yes = 1;

  if(socket1 == -1){
    error("ERROR socket creation");
  }
  if (setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    error("ERROR setting socket options");
  }

  return socket1;
}

//------------------------------------------------------------------------------

void get_addr_info(const char* port, struct sockaddr_in* serv_addr) {

  int portno;

  memset(serv_addr,'\0',sizeof(serv_addr));
  portno = atoi(port);
  serv_addr->sin_family = AF_INET;
  serv_addr->sin_addr.s_addr = INADDR_ANY;
  serv_addr->sin_port = htons(portno);
}

//------------------------------------------------------------------------------

void do_bind(int socket, const struct sockaddr_in pointeur_serv_addr){
  int i = bind(socket, (struct sockaddr*) &pointeur_serv_addr, sizeof(pointeur_serv_addr));
  if(i == -1){
    error("ERROR bind server");
  }
}


//------------------------------------------------------------------------------

void listen_client(int socket, int backlog){
  int i = listen(socket, backlog);
  if(i == -1){
    error("ERROR listen server");
  }
}

//------------------------------------------------------------------------------

int do_accept(int socket, struct sockaddr_in *pointeur_host_addr){
  size_t host_addr_size = sizeof(struct sockaddr_in);
  int i = accept(socket, (struct sockaddr *)pointeur_host_addr,(socklen_t *)&host_addr_size);
  printf("Je me connecte avec l'adresse : %s\n",inet_ntoa(pointeur_host_addr->sin_addr));

  if(i == -1){
    error("ERROR accepte server");
  }
  return i;
}

//------------------------------------------------------------------------------

int do_read(int socket, char *buffer){
  memset (buffer, '\0', MSG_SIZE);
  int i = read(socket,buffer,MSG_SIZE);
  if(i == -1){
    error("ERROR read server");
  }
  return i;
}

//------------------------------------------------------------------------------

void do_write(int fd, const void *buffer){
  size_t i = write(fd,buffer,MSG_SIZE);
  if(i == -1){
    error("ERROR write server");
  }
}

//------------------------------------------------------------------------------

void close_socket(int socket){
  int i = close(socket);
  if(i == -1){
    error("ERROR close socket");
  }
}
