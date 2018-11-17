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
  char file_to_send_path[MSG_SIZE];
  char file_receive_path[MSG_SIZE];
  char sender_name[MSG_SIZE];
  char file_receive_name[MSG_SIZE];
  int ready_to_receive=0; // sera à 1 si on un utilisateur veut nous envoyer un fichier
  int error;
  int socket;
  struct sockaddr_in pointeur_serv_addr;
  int event_fd;
  int file_fd;
  char file[MSG_SIZE];
  int new_socket_receiver;
  struct trame *trame=NULL;
  trame=trame_init(trame);
  trame=trame_set_to_zero(trame);



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
  trame=trame_set_to_zero(trame);
  read(socket,trame,size_struc_trame());
  fprintf(stdout,"[%s] %s\n",trame_sender_name(trame),trame_message(trame));

  while(1){
    memset(saisie,'\0',MSG_SIZE);
    do_read(STDIN_FILENO,saisie);
    if(strncmp(saisie,"/nick ",strlen("/nick ")) == 0){
      memset (message, '\0', MSG_SIZE);
      strncpy(message,saisie,strlen(saisie)-1);
      handle_client_message(socket,message);
      trame=trame_set_to_zero(trame);
      read(socket,trame,size_struc_trame());
      fprintf(stdout,"[%s] %s\n",trame_sender_name(trame),trame_message(trame));

      if(strcmp(trame_message(trame),"This pseudo is already used, please take another one")!=0){
        break;
      }
    }
    else{
      printf("[Client] : veuillez respecter la synthaxe\n");
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
    error=0;

    // wait for an activity

    event_fd = poll(fds,3,-1);

    if(fds[1].revents == POLLIN){
      //get user input--------------------------------------------------------------
      do_read(STDIN_FILENO,saisie);
      memset (message, '\0', MSG_SIZE);
      strncpy(message,saisie,strlen(saisie)-1);

      //connexion end---------------------------------------------------------------
      if(strcmp(message, "/quit") == 0){
        printf("Fermeture connexion client\n");
        break;
      }

      if(strncmp(message, "/send",strlen("/send")) == 0){
        memset(file_to_send_path,'\0',MSG_SIZE);

      //   /media/sf_Dossier_partagé_LINUX/S7/Prog_reseaux/Prog_reseau/src/jalon01/file.txt

        for(int i =strlen("/send ")+1;i<strlen(message);i++){
          if(message[i]==' '){
            strcpy(file_to_send_path,message+i+1);
            break;
          }
        }
        file_fd=open(file_to_send_path,O_RDONLY);
        if(file_fd<=0){
          printf("The file does not exist, please try again\n");
          error=1;
        }
        else{
          do_read(file_fd,file);
        }
      }



      if(strcmp(message,"N") == 0 &&  ready_to_receive==1){
          ready_to_receive=0;
      }

      //send message to the server--------------------------------------------------
      if(error==0)
        handle_client_message(socket,message);


      if(strcmp(message,"Y") == 0 &&  ready_to_receive==1){

            int socket_receiver= do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

            //init the serv_add structure
            struct sockaddr_in pointeur_recep_addr;
            get_addr_info("1025", &pointeur_recep_addr,"127.0.0.1");

            //perform the binding---------------------------------------------------------
            do_bind(socket_receiver,pointeur_recep_addr);

            //listen for at most 1 concurrent client-------------------------------------
            listen_client(socket_receiver,1);
            struct sockaddr_in *pointeur_host_addr = malloc(sizeof(struct sockaddr_in));

            new_socket_receiver=do_accept(socket_receiver,pointeur_host_addr);
            fds[2].fd = new_socket_receiver;
            fds[2].events = POLLIN;
            printf("accept\n");
            do_read(new_socket_receiver,buffer);
            printf("read \n");
            sprintf(file_receive_path,"/media/sf_Dossier_partagé_LINUX/S7/Prog_reseaux/Prog_reseau/src/jalon01/inbox/%s",file_receive_name);  // changer le chemin en fonction de la machine
            int test_fd=open(file_receive_path,O_RDWR|O_CREAT);
            write(test_fd,buffer,strlen(buffer));
            printf("%s saved in %s\n",file_receive_name,file_receive_path);
            close(new_socket_receiver);
            close(socket_receiver);
            ready_to_receive=0;
          }


    }


    if(fds[0].revents == POLLIN){

      trame=trame_set_to_zero(trame);
      valeur= read(socket,trame,size_struc_trame());
      if(strncmp(trame_channel_name(trame),"Unspecified channel",strlen("Unspecified channel"))==0){
        fprintf(stdout,"[%s] %s\n",trame_sender_name(trame),trame_message(trame));
      }
      else{
        fprintf(stdout,"[%s][%s] %s\n",trame_sender_name(trame),trame_channel_name(trame),trame_message(trame));
      }



      if(strncmp(trame_message(trame),"wants you to accept the transfer of the file",strlen("wants you to accept the transfer of the file")) == 0){
          ready_to_receive=1;
          strcpy(file_receive_name,trame_file_name(trame));
      }


      if(strncmp(trame_message(trame),"accepted file transfert",strlen("accepted file transfert")) == 0){
        char port[4];
        memset (port, '\0', strlen(port));

        struct sockaddr_in pointeur_sender_addr;
        int socket_sender;
        socket_sender = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        //init the serv_add structures-------------------------------------------------
        get_addr_info("1025", &pointeur_sender_addr,"127.0.0.1");

        //connect to remote socket----------------------------------------------------
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
