#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#define MSG_SIZE 256


//Fonctions---------------------------------------------------------------------

void error(const char *msg)
{
  perror(msg);
  exit(1);
}

//------------------------------------------------------------------------------


int do_socket(int domain, int type, int protocol) {
  int socket1;
  int yes = 1;

  socket1 = socket(domain,type,protocol);
  if(socket1 == -1){
    error("ERROR socket1 creation");
  }
  if (setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    error("ERROR setting socket options");
  }

  return socket1;
}


//------------------------------------------------------------------------------


void get_addr_info(const char* port, struct sockaddr_in* serv_addr,const char* host) {

  int portno;

  memset(serv_addr,'\0',sizeof(serv_addr));
  portno = atoi(port);
  serv_addr->sin_family = AF_INET;
  inet_aton(host,&(serv_addr->sin_addr));
  serv_addr->sin_port = htons(portno);

}

//------------------------------------------------------------------------------


void do_connect(int socket, struct sockaddr_in pointeur_serv_addr){
  int i = connect(socket, (struct sockaddr*) &pointeur_serv_addr, sizeof(struct sockaddr));
  if(i == -1){
    error("ERROR connexion client");
  }
}

//------------------------------------------------------------------------------


void handle_client_message(int socket, const void *message){
  int i = send(socket,message,strlen(message),0);
  if(i == -1){
    error("ERROR envoi message server");
  }
}

//------------------------------------------------------------------------------

void do_read(int socket, char *buffer){
  memset (buffer, '\0', MSG_SIZE);
  int i = read(socket,buffer,MSG_SIZE);
  if(i == -1){
    error("ERROR read server");
  }
}



//Corps-------------------------------------------------------------------------


int main(int argc,char** argv){
  if (argc != 3)
  {
    fprintf(stderr,"usage: RE216_CLIENT  port hostname\n");
    return 1;
  }

  //Variables--------------------------------------------------------------

  char buffer[MSG_SIZE];
  char saisie[MSG_SIZE];
  char pseudo[MSG_SIZE];
  char copie[MSG_SIZE];
  int socket;
  struct sockaddr_in pointeur_serv_addr;
  const void* msg = copie;

  //get the socket--------------------------------------------------------------
  printf("Etape : Création socket\n");
  socket = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  //init the serv_add structure-------------------------------------------------
  printf("Etape : Informations serveur\n");
  get_addr_info(argv[1], &pointeur_serv_addr,argv[2]);

  //connect to remote socket----------------------------------------------------
  printf("Connexion au serveur\n");
  do_connect(socket,pointeur_serv_addr);


  //Obligation identification
  memset(buffer,'\0',MSG_SIZE);
  do_read(socket,buffer);
  printf("%s\n",buffer);



  int i=0;

  while(1){
    memset(saisie,'\0',MSG_SIZE);
    do_read(STDIN_FILENO,saisie);
    //strcpy(saisie,"/nick ");
    //strcat(saisie,argv[3]);
    if(strncmp(saisie,"/nick ",strlen("/nick "))==0){
      memset (copie, '\0', MSG_SIZE);
      strncpy(copie,saisie,strlen(saisie)-1);
      handle_client_message(socket,copie);
      printf("Identification réussie\n");
      break;
    }
    else{
      printf("Identification échec : veuillez respecter la synthaxe\n");
    }
  }

  pid_t pid=fork();

  while(1){


    if(pid==0){
      //read what the client has to say---------------------------------------------
      memset (buffer, '\0', sizeof (buffer));
      do_read(socket,buffer);
      memset (pseudo, '\0', sizeof (pseudo));
      i=0;
      while(buffer[i]!='|'){
        i++;
      }
      strncpy(pseudo,buffer,i);
      fprintf(stdout,"[%s] %s\n",pseudo,buffer+i+1);
    }
    else{
      //get user input--------------------------------------------------------------
      do_read(STDIN_FILENO,saisie);
      memset (copie, '\0', MSG_SIZE);
      strncpy(copie,saisie,strlen(saisie)-1);
      msg = copie;

      //send message to the server--------------------------------------------------
      handle_client_message(socket,msg);

      //connexion end---------------------------------------------------------------
      if(strncmp(msg, "/quit",strlen("/quit")-1) == 0){
        printf("Fermeture connexion client\n");
        break;
      }

    }
  }


  //close socket----------------------------------------------------------------
  printf("Fermeture socket\n");
  close(socket);

  return 0;

}
