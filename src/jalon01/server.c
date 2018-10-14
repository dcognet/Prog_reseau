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
#include "server_tools.h"
#include "user_tools.h"
#include "server_cast.H"







// char *msg_to_send(char sender[],char buffer[]){
//   strcat(sender,"|");
//   strcat(sender,buffer);
//   printf("%s\n", sender);
//   return sender;
// }

//Corps-------------------------------------------------------------------------

int main(int argc, char** argv)
{

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
  struct user *user_list=NULL;
  struct sockaddr_in *pointeur_host_addr = malloc(sizeof(struct sockaddr_in));
  char envoie[MSG_SIZE];
  char *msg=malloc(MSG_SIZE*sizeof(char));
  char server[MSG_SIZE]="Server";



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

    for (i = 1; i <= nb_co_max; i++) {

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
      else
      {
        if(fds[i].revents == POLLIN){

          //read what the do_readclient has to say------------------------------
          memset(buffer,'\0',MSG_SIZE);

          valeur = do_read(fds[i].fd,buffer);
          printf("Le message reçu est: %s\n",buffer);

          //command /nick

          if(strncmp(buffer,"/nick",strlen("/nick")) == 0){
            memset(pseudo,'\0',MSG_SIZE);
            strcpy(pseudo,buffer+strlen("/nick "));
            strcpy(envoie,"[Server] Welcome on the chat : ");
            strncpy(pseudo,buffer+6,10);
            printf("%s\n",pseudo);
            user_list = user_change_pseudo(user_list,pseudo,fds[i].fd);
            do_write(fds[i].fd,strcat(envoie,user_pseudo(user_list,fds[i].fd)));
            break;
          }

          //command /who

          if(strcmp(buffer,"/who") == 0){
            display_user_list(user_list,fds[i].fd);
            break;
          }

          //command /whois

          if(strncmp(buffer,"/whois",strlen("/whois")) == 0){
            memset(pseudo,'\0',MSG_SIZE);
            strcpy(pseudo,buffer+strlen("/whois "));
            user_date_connexion(fds[i].fd,user_list,pseudo);
            break;
          }
          if(strncmp(buffer,"/msgall",7)==0){
            sprintf(envoie,"[%s] %s",user_pseudo(user_list,fds[i].fd),buffer+strlen("/msgall ") );
            broadcast(fds[i].fd,envoie,user_list);
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
          if(strncmp(buffer,"/msg",4)==0){
            int space=0;
            while(buffer[space+strlen("/msg ")]!=' '){
              space++;
            }
            memset(pseudo,'\0',MSG_SIZE);
            strncpy(pseudo,buffer+strlen("/msg "),space);
            printf("%s\n",pseudo );
            sprintf(envoie,"[%s] %s",user_pseudo(user_list,fds[i].fd),buffer+1+space+strlen("/msg ") );
            printf("%s\n",envoie );
            unicast(fds[i].fd,envoie,user_list,pseudo);
            break;
          }




          //we write back to the client---------------------------------------------
          sprintf(envoie,"[Server] %s",buffer);
          do_write(fds[i].fd,envoie);
        }
      }
    }
  }

  //clean up server socket------------------------------------------------------
  printf("Fermeture socket serveur\n");
  close_socket(socket);
  return 0;

}
