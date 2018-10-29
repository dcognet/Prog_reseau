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
#include "include/client_tools.h"
#include <signal.h>



//Corps-------------------------------------------------------------------------


int main(int argc,char** argv){
  if (argc != 3){
    fprintf(stderr,"usage: RE216_CLIENT  port hostname\n");
    return 1;
  }

  //Variables--------------------------------------------------------------

  char buffer[MSG_SIZE];
  char saisie[MSG_SIZE];
  char message[MSG_SIZE];
  int socket, valeur;
  struct sockaddr_in pointeur_serv_addr;
  int i = 0;

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

  while(1){
    memset(saisie,'\0',MSG_SIZE);
    do_read(STDIN_FILENO,saisie);
    //strcpy(saisie,"/nick ");
    //strcat(saisie,argv[3]);
    if(strncmp(saisie,"/nick ",strlen("/nick ")) == 0){
      memset (message, '\0', MSG_SIZE);
      strncpy(message,saisie,strlen(saisie)-1);
      handle_client_message(socket,message);
      do_read(socket,buffer);
      printf("%s\n",buffer);
      if(strcmp(buffer,"[Serveur] : Pseudo déjà existant, veuillez en choisir un nouveau")!=0){
        break;
      }
    }
    else{
      printf("[Cient] : veuillez respecter la synthaxe\n");
    }
  }

  pid_t pid = fork();

  while(1){

    if(pid == 0){
      //read what the client has to say---------------------------------------------
      memset (buffer, '\0', strlen(buffer));
      valeur = do_read(socket,buffer);
      fprintf(stdout,"%s\n",buffer);
      if(valeur == 0){
        printf("Fermeture connexion client\n");
        kill(pid,SIGKILL);
        break;
      }

    }
    else{
      //get user input--------------------------------------------------------------
      do_read(STDIN_FILENO,saisie);
      memset (message, '\0', MSG_SIZE);
      strncpy(message,saisie,strlen(saisie)-1);

      //send message to the server--------------------------------------------------
      handle_client_message(socket,message);

      //connexion end---------------------------------------------------------------
      if(strncmp(message, "/quit",strlen("/quit")-1) == 0 && strlen(message) == strlen("/quit")){
        printf("Fermeture connexion client\n");
        kill(pid,SIGKILL);
        break;
      }
    }
  }


  //close socket----------------------------------------------------------------
  printf("Fermeture socket\n");
  close(socket);

  return 0;

}
