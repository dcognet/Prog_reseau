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
#include <poll.h>
#include <sys/stat.h>
#include <fcntl.h>



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
  char copie[MSG_SIZE];
  int socket;
  struct sockaddr_in pointeur_serv_addr;
  int event_fd;
  int file_fd;
  char file[MSG_SIZE];
  int new_socket_receiver;


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

  struct pollfd fds[200];
  int valeur;

  memset(fds,-1,sizeof(fds));
  fds[0].fd = socket;
  fds[0].events = POLLIN;
  fds[1].fd=STDIN_FILENO;
  fds[1].events = POLLIN;


  while(1){

    // wait for an activity

    event_fd = poll(fds,3,-1);

    if(fds[1].revents == POLLIN){
      printf("test\n" );
      //get user input--------------------------------------------------------------
      do_read(STDIN_FILENO,saisie);
      memset (message, '\0', MSG_SIZE);
      strncpy(message,saisie,strlen(saisie)-1);

      //send message to the server--------------------------------------------------
      handle_client_message(socket,message);

      //connexion end---------------------------------------------------------------
      if(strncmp(message, "/quit",strlen("/quit")) == 0){
        printf("Fermeture connexion client\n");
        break;
      }

      if(strncmp(message, "/send",strlen("/send")) == 0){

        file_fd=open("/media/sf_Dossier_partagé_LINUX/S7/Prog_reseaux/Prog_reseau/src/jalon01/file.txt",O_RDONLY);
        printf("%i\n",file_fd );
        memset (file, '\0', MSG_SIZE);
        do_read(file_fd,file);
        printf("%s\n",file );
      }


          if(strcmp(message,"Y") == 0){

            printf("Etape : Création socket\n");
            int socket_receiver= do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

            //init the serv_add structure
            printf("Etape : Informations serveur\n");
            struct sockaddr_in pointeur_recep_addr;
            get_addr_info("1025", &pointeur_recep_addr,"127.0.0.1");

            //perform the binding---------------------------------------------------------
            printf("Etape : Bind\n");
            do_bind(socket_receiver,pointeur_recep_addr);

            //listen for at most 20 concurrent client-------------------------------------
            printf("Etape : Ecoute\n");
            listen_client(socket_receiver,1);
            struct sockaddr_in *pointeur_host_addr = malloc(sizeof(struct sockaddr_in));

            new_socket_receiver=do_accept(socket_receiver,pointeur_host_addr);

            fds[2].fd = new_socket_receiver;
            fds[2].events = POLLIN;
            do_read(new_socket_receiver,buffer);
            int test_fd=open("/media/sf_Dossier_partagé_LINUX/S7/Prog_reseaux/Prog_reseau/src/jalon01/test.txt",O_RDWR);

            write(test_fd,buffer,strlen(buffer));
            //fprintf(stdout,"%s\n",buffer);
            close(new_socket_receiver);
            close(socket_receiver);
          }

    }





    if(fds[0].revents == POLLIN){
      //read what the server has to say---------------------------------------------
      memset (buffer, '\0', strlen(buffer));
      valeur=do_read(socket,buffer);
      fprintf(stdout,"%s\n",buffer);

      if(strncmp(buffer+4, "accepted file transfert.",strlen("accepted file transfert.")) == 0){
        char port[4];
        memset (port, '\0', strlen(port));

        printf("Etape : Création socket vers le recepteur\n");
        struct sockaddr_in pointeur_sender_addr;
        int socket_sender;
        socket_sender = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        //init the serv_add structure-------------------------------------------------
        printf("Etape : Informations du recepteur\n");
        get_addr_info("1025", &pointeur_sender_addr,"127.0.0.1");

        //connect to remote socket----------------------------------------------------
        printf("Connexion au recpeteur\n");
        do_connect(socket_sender,pointeur_sender_addr);
        fds[2].fd = socket_sender;
        fds[2].events = POLLIN;
        handle_client_message(socket_sender,file);
      }

      if(valeur == 0){
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
