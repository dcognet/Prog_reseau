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

//   /media/sf_Dossier_partagé_LINUX/S7/Prog_reseaux/Prog_reseau/src/jalon01/file.txt

//Corps-------------------------------------------------------------------------


int main(int argc,char** argv){
  if (argc != 3){
    fprintf(stderr,"usage: RE216_CLIENT  port hostname\n");
    return 1;
  }

  //Variables--------------------------------------------------------------

  char buffer[MSG_SIZE], saisie[MSG_SIZE],message[MSG_SIZE];
  char file_to_send_path[MSG_SIZE]; //path of the file the client want to send
  char file_receive_path[MSG_SIZE]; //path where the file will be saved
  char file_receive_name[MSG_SIZE]; // name of the file we receive
  int ready_to_receive = 0; // sera à 1 si on un utilisateur veut nous envoyer un fichier
  int error;
  int socket;
  struct sockaddr_in pointeur_serv_addr;
  int event_fd;
  int test_fd; //Changer nom
  int file_fd;  //fd du fichier que l'on veut envoyer
  char file[MSG_SIZE]; // name of the file we send
  int new_socket_receiver;
  int i;

  struct trame *trame=NULL; //trame send by the server
  int recieve_port; //port number for the peer to peer

  int socket_receiver;
  struct sockaddr_in pointeur_recep_addr;
  struct sockaddr_in pointeur_host_addr;
  struct sockaddr_in pointeur_sender_addr;
  int socket_sender;

  struct pollfd fds[200];
  int valeur;

  trame=trame_init(trame);
  trame=trame_set_to_zero(trame);

  //get the socket--------------------------------------------------------------
  printf("Etape : Création socket\n");
  socket = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  //init the serv_add structure-------------------------------------------------
  printf("Etape : Informations serveur\n");
  get_addr_info(atoi(argv[1]), &pointeur_serv_addr,argv[2]);

  //connect to remote socket----------------------------------------------------
  printf("Connexion au serveur\n");
  do_connect(socket,pointeur_serv_addr);


  //Obligation identification
  memset(buffer,'\0',MSG_SIZE);
  trame = trame_set_to_zero(trame);
  read(socket,trame,size_struc_trame());
  fprintf(stdout,"[%s] %s\n",trame_sender_name(trame),trame_message(trame));

  //on test si le serveur peut nous accepter
  if(strcmp(trame_message(trame),"Server cannot accept incoming connections anymore. Try again later.")==0){
    close(socket);
    return 0;
  }

  //tant que l'utilisateur n'a pas donné un pseudo correct
  while(1){
    memset(saisie,'\0',MSG_SIZE);
    do_read(STDIN_FILENO,saisie);
    if(strncmp(saisie,"/nick ",strlen("/nick ")) == 0){
      memset (message, '\0', MSG_SIZE);
      strncpy(message,saisie,strlen(saisie)-1);
      handle_client_message(socket,message);
      trame = trame_set_to_zero(trame);
      read(socket,trame,size_struc_trame());
      fprintf(stdout,"[%s] %s\n",trame_sender_name(trame),trame_message(trame));


      if(strncmp(trame_message(trame),"Welcome on the chat :",strlen("Welcome on the chat :")) == 0)
        break;

    }
    else{
      printf("[Client] : veuillez respecter la synthaxe\n");
    }
  }



  memset(fds,-1,sizeof(fds));
  fds[0].fd = socket;
  fds[0].events = POLLIN;
  fds[1].fd = STDIN_FILENO;
  fds[1].events = POLLIN;


  while(1){
    error = 0;

    // on attend qu'il y est une activité sur un des fd que l'on a enregistré dans la structure pollfd

    event_fd = poll(fds,3,-1);

    // test si il y a eu un evenement sur l'entrée standart
    if(fds[1].revents == POLLIN){
      //message tapé par le client--------------------------------------------------------------
      do_read(STDIN_FILENO,saisie);
      memset (message, '\0', MSG_SIZE);
      strncpy(message,saisie,strlen(saisie)-1);

      //fin de connexion---------------------------------------------------------------
      if(strcmp(message, "/quit") == 0){
        printf("Fermeture connexion client\n");
        break;
      }

      //commande /send
      if(strncmp(message,"/send",strlen("/send")) == 0){
        memset(file_to_send_path,'\0',MSG_SIZE);

        //boucle pour connaitre le chemin du fichier à envoyé
        for(i =strlen("/send ")+1;i<strlen(message);i++){
          if(message[i]==' '){
            strcpy(file_to_send_path,message+i+1);
            break;
          }
        }

        file_fd = open(file_to_send_path,O_RDONLY);
        if(file_fd <= 0){
          printf("The file does not exist, please try again\n");
          error = 1;
        }
        else{
          do_read(file_fd,file);
          close(file_fd);
        }
      }


      //test si le client ne veut pas recevoir le fichier
      if(strcmp(message,"N") == 0 &&  ready_to_receive == 1){
        ready_to_receive = 0;
      }

      //envoie un message sur il n'y a pas d'erreur--------------------------------------------------
      if(error == 0) handle_client_message(socket,message);

      //check if the client is ok to receive the file
      if(strcmp(message,"Y") == 0 &&  ready_to_receive == 1){

        socket_receiver = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);


        memset(&pointeur_recep_addr,0,sizeof(struct sockaddr_in));
        memset(&pointeur_host_addr,0,sizeof(struct sockaddr_in));


        get_addr_info(recieve_port, &pointeur_recep_addr,"127.0.0.1");

        //perform the binding---------------------------------------------------------
        do_bind(socket_receiver,pointeur_recep_addr);

        //listen for at most 1 concurrent client-------------------------------------
        listen_client(socket_receiver,1);

        new_socket_receiver = do_accept(socket_receiver,&pointeur_host_addr);

        fds[2].fd = new_socket_receiver;
        fds[2].events = POLLIN;
        do_read(new_socket_receiver,buffer);
        sprintf(file_receive_path,"/media/sf_Dossier_partagé_LINUX/S7/Prog_reseaux/Prog_reseau/src/jalon01/inbox/%s",file_receive_name);  // changer le chemin en fonction de la machine
        test_fd = open(file_receive_path,O_RDWR|O_CREAT);
        write(test_fd,buffer,strlen(buffer));
        printf("%s saved in %s\n",file_receive_name,file_receive_path);
        close(test_fd);
        close(new_socket_receiver);
        close(socket_receiver);
        ready_to_receive = 0;
      }


    }


    if(fds[0].revents == POLLIN){

      trame = trame_set_to_zero(trame);
      valeur = read(socket,trame,size_struc_trame());
      if(strncmp(trame_channel_name(trame),"Unspecified channel",strlen("Unspecified channel")) == 0){
        fprintf(stdout,"[%s] %s\n",trame_sender_name(trame),trame_message(trame));
      }
      else{
        fprintf(stdout,"[%s][%s] %s\n",trame_sender_name(trame),trame_channel_name(trame),trame_message(trame));
      }



      if(strncmp(trame_message(trame),"wants you to accept the transfer of the file",strlen("wants you to accept the transfer of the file")) == 0){
        ready_to_receive = 1;
        strcpy(file_receive_name,trame_file_name(trame));
        recieve_port = trame_port(trame);
      }


      if(strncmp(trame_message(trame),"accepted file transfert",strlen("accepted file transfert")) == 0){

        memset(&pointeur_sender_addr,0,sizeof(struct sockaddr_in));

        socket_sender = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        printf("%i\n",trame_port(trame) );
        //init the serv_add structures-------------------------------------------------
        get_addr_info(trame_port(trame), &pointeur_sender_addr,"127.0.0.1");

        //connect to remote socket----------------------------------------------------
        do_connect(socket_sender,pointeur_sender_addr);
        fds[2].fd = socket_sender;
        fds[2].events = POLLIN;
        handle_client_message(socket_sender,file);
        close(socket_sender);

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
