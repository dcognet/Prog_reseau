#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>

//Fonctions---------------------------------------------------------------------




//---------------------------------------
void error(const char *msg)
{
  perror(msg);
  exit(1);
}

//------------------------------------------------------------------------------

int do_socket(int domain, int type, int protocol) {
  int socket1;
  int yes = 1;

  //create the socket
  socket1 = socket(domain,type,protocol);

  //check for socket validity
  if(socket1 == -1){
    error("ERROR socket creation");
  }

  // set socket option, to prevent "already in use" issue when rebooting the server right on
  if (setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    error("ERROR setting socket options");
  }

  return socket1;
}

//------------------------------------------------------------------------------

void get_addr_info(const char* port, struct sockaddr_in* serv_addr) {

  int portno;

  //clean the serv_add structure
  memset(serv_addr,'\0',sizeof(serv_addr));

  //cast the port from a string to an int
  portno = atoi(port);

  //internet family protocol
  serv_addr->sin_family = AF_INET;

  //we bind to any ip form the host
  serv_addr->sin_addr.s_addr = INADDR_ANY;

  //we bind on the tcp port specified
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

int do_accept(int socket, struct sockaddr_in pointeur_host_addr){
  size_t host_addr_size = sizeof(struct sockaddr_in);
  int i = accept(socket, (struct sockaddr *) &pointeur_host_addr,(socklen_t *) &host_addr_size);
  if(i == -1){
    error("ERROR accepte server");
  }
  return i;
}


//------------------------------------------------------------------------------

void do_read(int socket, char *buffer){
  int i = read(socket,buffer,255);
  if(i == -1){
    error("ERROR read server");
  }
}

//------------------------------------------------------------------------------

void do_write(int fd, const void *buffer){
  size_t i = write(fd,buffer,255);
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


// struct user------------------------------------------
struct user{
  char pseudo[255];
  int fd;

  struct user *next;

};

// create a new user------------------------------------------

struct user *create_user(char pseudo[],int fd){
  struct user *new_user;
  new_user=malloc(sizeof(struct user));
  strcpy(new_user->pseudo,pseudo);
  new_user->fd=fd;
  new_user->next=NULL;
  return new_user;
}

// return the size of the user list------------------------------------------

int user_list_size(struct user *user_list){
  if (user_list==NULL){
    printf("taille nul\n" );

  return 0;
}
int size=0;
while (user_list!=NULL){

  size++;
  user_list=user_list->next;
  printf("%i\n",size );

}

return size;
}

// add une new user---------------------------------------------------------
struct user *user_add(struct user *user,char pseudo[],int fd){

	struct user *new_user=malloc(sizeof(struct user));
	new_user=create_user(pseudo,fd);

	if (user_list_size(user) ==0){
		new_user->next=user;
    printf("1er user %s\n",new_user->pseudo );
		return new_user;
	}
	else {
		struct user *temp;
		temp=user;
    if(temp->fd==fd){
      printf("deja indentifié\n");
      strcpy(temp->pseudo,pseudo);
      return temp;
    }
		while(temp->next!=NULL){

      if(temp->fd==fd){
        printf("deja indentifié\n" );

        return temp;
      }
      temp=temp->next;

		}
		temp->next=new_user;
		return user;
	}
}


// change the pseudo of an user---------------------------------------------------------
struct user *user_change_pseudo(struct user *user,char pseudo[],int fd){

		struct user *temp;
		temp=user;
    if(temp->fd==fd){
      printf("deja indentifié\n");
      strcpy(temp->pseudo,pseudo);
    }
		while(temp!=NULL){
      if(temp->fd==fd){
        printf("deja indentifié\n" );
        strcpy(temp->pseudo,pseudo);
      }
      temp=temp->next;
		}
  	return user;
	}



//display the list of user ----------------------------------------

int display_user_list(struct user *list_user,int fd){
  char buffer[255]=" [Server] : Online users are :";
	if (list_user==NULL)
	return 0;

	while (list_user!=NULL){
      char pseudo[255]="\n -";
       strcat(pseudo,list_user->pseudo);
       strcat(buffer,pseudo);
		list_user=list_user->next;
	}
  do_write(fd,buffer);
	return 1;

}



// return the user pseudo------------------------------------------


char *user_pseudo(struct user *user_list,int fd){
  if (user_list==NULL)
  return 0;

  while (user_list!=NULL){
    if(user_list->fd==fd){
      return user_list->pseudo;
    }
    user_list=user_list->next;
  }
  return 0;

}

// delete an user------------------------------------------

struct user *delete_user(struct user *user_list,int fd){
  printf("opssps\n" );

	if (user_list==NULL)
	return NULL;
  struct user *temp;
  temp=user_list;
  if(temp->fd==fd){
    return temp->next;
  }


  while(temp->next!=NULL){
    if ((temp->next)->next==NULL && (temp->next)->fd==fd) {
      temp->next=NULL;
      break;
    }
    if((temp->next)->fd==fd){
      temp->next=(temp->next)->next;
    }
    temp=temp->next;
  }
  return user_list;

}




void broadcast(int sender_fd, const void *buffer, struct user *list_user){
  if (list_user==NULL)
  return 0;

  while (list_user!=NULL){
    if(list_user->fd!=sender_fd){
      do_write(list_user->fd,buffer);

    }
    list_user=list_user->next;
  }
  return 1;
}

void unicast(int sender_fd, const void *buffer, struct user *list_user, char receiver_pseudo[]){
  if (list_user==NULL)
  return 0;

  while (list_user!=NULL){
    if(receiver_pseudo==list_user->pseudo){
      do_write(list_user->fd,buffer);
    }
    list_user=list_user->next;
  }
  return 1;
}

//Corps-------------------------------------------------------------------------

int main(int argc, char** argv)
{

  if (argc != 2)
  {
    fprintf(stderr, "usage: RE216_SERVER port\n");
    return 1;
  }

  char buffer[256];

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

  int nb_co=0;
  int nb_co_max=3;
  int event_fd;
  struct pollfd fds[200];
  memset(fds,-1,sizeof(fds));
  fds[0].fd=socket;
  fds[0].events=POLLIN;

  struct user *user_list=NULL;




  while(1){

    // wait for an activity
    event_fd=poll(fds,21,-1);


    for (int i = 1; i <= 20; i++) {
        if(fds[0].revents==POLLIN){
          if(fds[i].fd==-1){

            //accept connection from client-------------------------------------------

            struct sockaddr_in pointeur_host_addr;
            int new_socket = do_accept(socket,pointeur_host_addr);

            if(nb_co>=nb_co_max){
              // refuse the connection if there is too much client
              printf("Acceptation d'un nouveau client impossibla car trop de connection\n");
              do_write(new_socket,"Server cannot accept incoming connections anymore. Try again later.");
              close_socket(new_socket);
              break;
            }
            printf("Acceptation d'un nouveau client : client n)%i\n",i);
            fds[i].fd=new_socket;
            fds[i].events=POLLIN;
            nb_co++;
            printf("nombre de connection = %i\n",nb_co);
            memset (buffer, '\0', sizeof (buffer));
            strcpy(buffer,"[Server] : please logon with /nick <your pseudo>");
            do_write(fds[i].fd,buffer);
            break;
          }
        }
        else
            {
              if(fds[i].revents==POLLIN){
                //read what the client has to say------------------------------------------
                memset (buffer, '\0', sizeof (buffer));
                do_read(fds[i].fd,buffer);
                printf("Le message reçu est: %s\n",buffer);

                  //clean up client socket-------------------------------------------------
                  if(strcmp(buffer, "/quit") == 0 ){
                    printf("Fermeture socket client\n");
                    user_list=delete_user(user_list,fds[i].fd);

                    close_socket(fds[i].fd);
                    fds[i].fd=-1;
                    fds[i].events=-1;
                    nb_co--;
                    break;
                  }

                  //idee si / alors on regarde le prochain mot
                  if(strncmp(buffer, "/ ",1) == 0 ){

                    // int space[255];
                    // space[0]=0;
                    // int indice=0;
                    // int j=0;
                    // //permet de trouver les espaces dans la ligne
                    // while (buffer[indice]!='\0') {
                    //   if(buffer[indice]==' '){
                    //     space[j]=indice;
                    //     j++;
                    //   }
                    //   indice++;
                    // }

                    // char command[255];
                    // memset(command,'\0',sizeof(command));
                    // strncpy(command,buffer,space[0]);
                    // printf("%s\n",command);

// command /nick
                    if(strncmp(buffer,"/nick",5)==0){
                      char pseudo[255]="";
                      char envoie[255]="server|Welcome on the chat : ";
                      strncpy(pseudo,buffer+6,10);
                      if (user_pseudo(user_list,fds[i].fd)==0)
                        user_list=user_add(user_list,pseudo,fds[i].fd);
                        else
                        user_list=user_change_pseudo(user_list,pseudo,fds[i].fd);
                      do_write(fds[i].fd,strcat(envoie,user_pseudo(user_list,fds[i].fd)));

                      break;
                    }

            // command /who

                    if(strcmp(buffer,"/who")==0){
                      char pseudo[255]="";
                      char envoie[255]="Voici la liste des utilisateur actuellement en ligne\n";
                      printf("%s\n",envoie );
                      display_user_list(user_list,fds[i].fd);
                      break;
                    }
                    if(strncmp(buffer,"/msgall",7)==0){
                      broadcast(fds[i].fd,buffer+8,user_list);
                      break;
                    }
                    // if(strncmp(buffer,"/msg",4)==0){
                    //
                    //   unicast(fds[i].fd,buffer+4,user_list,);
                    //   break;
                    // }
                  }



                //we write back to the client---------------------------------------------
                char pseudo[255]="";
                strcat(pseudo,buffer);
                do_write(fds[i].fd,pse);
              }
            }
          }
        }

  //clean up server socket------------------------------------------------------
  printf("Fermeture socket serveur\n");
  close_socket(socket);
  return 0;

}
