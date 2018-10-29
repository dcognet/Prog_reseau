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
#include "include/server_tools.h"
#include "include/user_tools.h"
#include "include/server_cast.h"
#include "include/server_channel.h"

#define nb_co_max 3

//Corps-------------------------------------------------------------------------

int main(int argc, char** argv){

  if (argc != 2){
    fprintf(stderr, "usage: RE216_SERVER port\n");
    return 1;
  }

  //Variables--------------------------------------------------------------

  int i, valeur, event_fd, new_socket, space;
  int nb_co = 0;
  struct pollfd fds[200];
  struct user *user_list = NULL;
  struct channel *channel_list = NULL;
  struct sockaddr_in *pointeur_host_addr = malloc(sizeof(struct sockaddr_in));
  char channel_name[MSG_SIZE];
  char envoie[MSG_SIZE];
  char buffer[MSG_SIZE];
  char information[MSG_SIZE];
  char pseudo[MSG_SIZE];




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

    memset(buffer,'\0',MSG_SIZE);
    memset(information,'\0',MSG_SIZE);
    memset(channel_name,'\0',MSG_SIZE);
    memset(envoie,'\0',MSG_SIZE);


    // wait for an activity

    event_fd = poll(fds,nb_co_max+1,-1);

    for (i = 1; i <= nb_co_max; i++) {

      if(fds[0].revents == POLLIN){
        if(fds[i].fd == -1){

          //accept connection from client-------------------------------------------

          new_socket = do_accept(socket,pointeur_host_addr);
          user_list = user_add(user_list,new_socket,pointeur_host_addr);

          if(nb_co >= nb_co_max){
            do_write(new_socket,"[Server] : Server cannot accept incoming connections anymore. Try again later.");
            close_socket(new_socket);
            break;
          }

          printf("Acceptation d'un nouveau client : client %i\n",i);
          fds[i].fd = new_socket;
          fds[i].events = POLLIN;
          nb_co++;
          printf("Nombre de connection = %i\n",nb_co);
          do_write(fds[i].fd,"[Server] : please logon with /nick <your pseudo>");
          break;
        }
      }
      else{
        if(fds[i].revents == POLLIN){

          //read what the do_readclient has to say------------------------------
          valeur = do_read(fds[i].fd,buffer);
          printf("Le message reçu est: %s %li\n",buffer,strlen(buffer));

          //command /nick
          if(strncmp(buffer,"/nick",strlen("/nick")) == 0){
            strcpy(information,buffer+strlen("/nick "));
            printf("L'information reçue est: %s %li\n",information,strlen(information));

            if(user_look_for_pseudo(user_list,information) == 0){
              user_list = user_change_pseudo(user_list,information,fds[i].fd);
              strcpy(envoie,"[Serveur] : Welcome on the chat : ");
              do_write(fds[i].fd,strcat(envoie,information));
            }
            else{
              do_write(fds[i].fd,"[Serveur] : This pseudo is already used, please take another one");
            }
            break;
          }

          //command /who
          if(strcmp(buffer,"/who") == 0){
            strcpy(envoie,user_display_list(user_list,fds[i].fd));
            do_write(fds[i].fd,envoie);
            break;
          }

          //command /whois
          if(strncmp(buffer,"/whois",strlen("/whois")) == 0){
            strcpy(information,buffer+strlen("/whois "));
            strcpy(envoie,user_connexion_information(fds[i].fd,user_list,information));
            do_write(fds[i].fd,envoie);
            break;
          }

          // command broadcast--------------------------------------
          if(strncmp(buffer,"/msgall",strlen("/msgall")) == 0){
            sprintf(envoie,"[%s] %s",user_pseudo(user_list,fds[i].fd),buffer+strlen("/msgall "));
            broadcast(fds[i].fd,envoie,user_list);
            break;
          }

          //clean up client socket----------------------------------------------
          if(strcmp(buffer,"/quit") == 0 || valeur == 0){
            strcpy(channel_name,user_channel_name(user_look_for_user(user_list,fds[i].fd)));
            if(user_appartient_channel(user_look_for_user(user_list,fds[i].fd)) == 0){
              channel_list = channel_down_number_member(channel_list,channel_name);
              if(channel_nombre_membre(channel_look_for_channel(channel_list,channel_name)) == 0){
                channel_list =  channel_delete(channel_list,channel_name);
              }
            }
            user_list = delete_user(user_list,fds[i].fd);
            printf("Fermeture socket client\n");
            close_socket(fds[i].fd);
            fds[i].fd = -1;
            fds[i].events = -1;
            nb_co--;
            break;
          }

          // quit the channel---------------------------------------------------
          if(strncmp(buffer,"/quit",strlen("/quit")) == 0){
            strcpy(channel_name,user_channel_name(user_look_for_user(user_list,fds[i].fd)));
            strcpy(information,buffer+strlen("/quit "));
            if(strcmp(channel_name,information) == 0){
              channel_list = channel_down_number_member(channel_list,channel_name);
              user_list = user_change_name_channel(user_list,"Unspecified channel",fds[i].fd);
              if(channel_nombre_membre(channel_look_for_channel(channel_list,channel_name)) == 0){
                channel_list =  channel_delete(channel_list,channel_name);
              }
              sprintf(envoie,"[Server] You left the channel : %s",channel_name);

            }
            else
              sprintf(envoie,"[Server] : The channel %s does not exist",channel_name);
            do_write(fds[i].fd,envoie);
            break;
          }

          // command unicast-------------------------------------------
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

          // command create--------------------------------------
          if(strncmp(buffer,"/create",strlen("/create")) == 0){
            strcpy(information,buffer+strlen("/create "));
            if(channel_look_for_name(channel_list,information) == 0 && user_appartient_channel(user_look_for_user(user_list,fds[i].fd)) == 1){
              channel_list = channel_add(channel_list,information);
              sprintf(envoie,"[%s] : You have created channel : %s",information,information);
              do_write(fds[i].fd,envoie);
            }
            else{
              do_write(fds[i].fd,"[Serveur] : This name of channel is already used");
            }
            break;
          }

          // commande join ---------------------------------
          if(strncmp(buffer,"/join",strlen("/join")) == 0){
            strcpy(information,buffer+strlen("/join "));
            if(channel_look_for_name(channel_list,information) == 1 && user_appartient_channel(user_look_for_user(user_list,fds[i].fd)) == 1){
              channel_list = channel_up_number_member(channel_list,information);
              user_list = user_change_name_channel(user_list,information,fds[i].fd);
              sprintf(envoie,"[%s] : You have joined : %s",information,information);
              do_write(fds[i].fd,envoie);
            }
            else{
              do_write(fds[i].fd,"[Serveur] : This channel does not exist or you already are in a channel");
            }
            break;
          }

          // multicast channel ---------------------------------
          if(strcmp(user_channel_name(user_look_for_user(user_list,fds[i].fd)),"Unspecified channel") != 0){
            strcpy(envoie,buffer);
            printf("channel\n");
            multicast(fds[i].fd,envoie,user_list,user_channel_name(user_look_for_user(user_list,fds[i].fd)));
            break;
          }

          // command /send--------------------------------------

          if(strncmp(buffer,"/send",5)==0){
            int space=0;
            while(buffer[space+strlen("/send ")]!=' '){
              space++;
            }
            memset(pseudo,'\0',MSG_SIZE);
            strncpy(pseudo,buffer+strlen("/send "),space);
            printf("%s\n",pseudo );
            sprintf(envoie,"[%s] wants you to accept the transfer of the file named . Do you accept? [Y/n]",user_pseudo(user_list,fds[i].fd));
            printf("%s\n",envoie );
            unicast(fds[i].fd,envoie,user_list,pseudo);
            user_list=user_change_send_to(user_list,pseudo,fds[i].fd);
            user_list=user_change_receive_from(user_list,pseudo,fds[i].fd);
            break;
          }

          //&& user_send(user_list,fds[i].fd)==1

          //
          if(strncmp(buffer,"y",1)==0 && user_receive_from(user_list,fds[i].fd)!=0 ){
            printf("yes\n" );
            sprintf(envoie,"[%s] accepted file transfert. %i",user_pseudo(user_list,fds[i].fd),user_port(user_list,fds[i].fd));
            printf("%s\n",envoie );
            unicast(fds[i].fd,envoie,user_list,user_pseudo(user_list,user_receive_from(user_list,fds[i].fd)));

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
