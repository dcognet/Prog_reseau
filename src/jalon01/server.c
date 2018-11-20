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
#include "include/message.h"

//Corps-------------------------------------------------------------------------

int main(int argc, char** argv){

  if (argc != 2){
    fprintf(stderr, "usage: RE216_SERVER port\n");
    return 1;
  }

  //Variables--------------------------------------------------------------

  int i, valeur, event_fd, new_socket, space, file_name_size;
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

  struct trame *trame = NULL;
  trame = trame_init(trame);
  trame = trame_set_to_zero(trame);

  int port = atoi(argv[1])+1;

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

    trame = trame_set_to_zero(trame);

    // wait for an activity
    event_fd = poll(fds,nb_co_max+1,-1);

    for (i = 1; i <= nb_co_max; i++) {

      if(fds[0].revents == POLLIN){
        if(fds[i].fd == -1){

          //acceptation d'un nouveau client-------------------------------------------

          new_socket = do_accept(socket,pointeur_host_addr);
		//on test si le nombre de connection est >=nombre max de connection afin de rejeter le nouveau client


          if(nb_co >= nb_co_max){
            trame = trame_set_message(trame,"Server cannot accept incoming connections anymore. Try again later.");
            trame = trame_set_sender_name(trame,"Server");
            do_write(fds[i].fd,trame);
            close_socket(new_socket);
            break;
          }
          user_list = user_add(user_list,new_socket,pointeur_host_addr);
          printf("Acceptation d'un nouveau client : client %i\n",i);
          fds[i].fd = new_socket;
          fds[i].events = POLLIN;
          nb_co++;
          printf("Nombre de connection = %i\n",nb_co);
          trame = trame_set_message(trame,"please logon with /nick <your pseudo>");
          trame = trame_set_sender_name(trame,"Server");
          do_write(fds[i].fd,trame);
          break;
        }
      }
      else{
        if(fds[i].revents == POLLIN){

          //lit ce que le client envoie------------------------------
          valeur = do_read(fds[i].fd,buffer);
          printf("Le message reçu est: %s\n",buffer);

          //tant que le recpeteur n'a pas accepté ou refusé le fichier, on lui demande de le faire (dans le cas d'un transfert de fichier)
          if(user_receive_from(user_list,fds[i].fd) != -1 && (strcmp(buffer,"Y") != 0 &&  strcmp(buffer,"N") != 0)){
            trame = trame_set_message(trame,"Please write Y or N");
            trame = trame_set_sender_name(trame,"Server");
            do_write(fds[i].fd,trame);
            break;
          }

          //commande /nick
          if(strncmp(buffer,"/nick",strlen("/nick")) == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_message(trame,"Veuillez respecter la synthaxe : /nick <nom_utilisateur>");
            }
            else{
              strcpy(information,buffer+strlen("/nick "));
              if(user_look_for_pseudo(user_list,information) == 0){
                user_list = user_change_pseudo(user_list,information,fds[i].fd);
                sprintf(envoie,"Welcome on the chat : %s",information);
                trame = trame_set_message(trame,envoie);
              }
              else{
                trame = trame_set_message(trame,"This pseudo is already used, please take another one");
              }
            }
            trame = trame_set_sender_name(trame,"Server");
            do_write(fds[i].fd,trame);
            break;
          }

          //commande /who
          if(strncmp(buffer,"/who ",strlen("/who ")) == 0 || strcmp(buffer,"/who") == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_message(trame,"Veuillez respecter la synthaxe : /who");
            }
            else{
              strcpy(envoie,user_display_list(user_list,fds[i].fd));
              trame = trame_set_message(trame,envoie);
            }
            trame = trame_set_sender_name(trame,"Server");
            do_write(fds[i].fd,trame);
            break;
          }

          //commande /whois
          if(strncmp(buffer,"/whois ",strlen("/whois ")) == 0 || strcmp(buffer,"/whois") == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_message(trame,"Veuillez respecter la synthaxe : /whois <nom_utilisateur>");
            }
            else{
              strcpy(information,buffer+strlen("/whois "));
              if(user_look_for_pseudo(user_list,information) == 0){
                trame = trame_set_message(trame,"Ce nom d'utilisateur n'existe pas");
              }
              else{
                strcpy(envoie,user_connexion_information(fds[i].fd,user_list,information));
                trame = trame_set_message(trame,envoie);
              }
            }
            trame = trame_set_sender_name(trame,"Server");
            do_write(fds[i].fd,trame);
            break;
          }


          // broadcast--------------------------------------
          if(strncmp(buffer,"/msgall",strlen("/msgall")) == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_sender_name(trame,"Server");
              trame = trame_set_message(trame,"Veuillez respecter la synthase : /msgall <message>");
              do_write(fds[i].fd,trame);
            }
            else{
              trame = trame_set_sender_name(trame,user_pseudo(user_list,fds[i].fd));
              trame = trame_set_message(trame,buffer+strlen("/msgall "));
              broadcast(fds[i].fd,trame,user_list);
            }
            break;
          }

          //commande /quit----------------------------------------------
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

          // quitter le channel---------------------------------------------------
          if(strncmp(buffer,"/quit",strlen("/quit")) == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_message(trame,"Veuillez respecter la synthaxe :\n - /quit pour quitter chat\n - /quit <nom_salon> pour quitter salon");
            }
            else{
              strcpy(channel_name,user_channel_name(user_look_for_user(user_list,fds[i].fd)));
              strcpy(information,buffer+strlen("/quit "));
              if(strcmp(channel_name,information) == 0){
                channel_list = channel_down_number_member(channel_list,channel_name);
                user_list = user_change_name_channel(user_list,"Unspecified channel",fds[i].fd);
                if(channel_nombre_membre(channel_look_for_channel(channel_list,channel_name)) == 0){
                  channel_list =  channel_delete(channel_list,channel_name);
                }
                sprintf(envoie,"You left the channel : %s",channel_name);
              }
              else{
                sprintf(envoie,"The channel %s does not exist",channel_name);
              }
              trame = trame_set_message(trame,envoie);
            }
            trame = trame_set_sender_name(trame,"Server");
            do_write(fds[i].fd,trame);
            break;
          }

          // unicast-------------------------------------------
          if(strncmp(buffer,"/msg",strlen("/msg")) == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_message(trame,"Veuillez respecter la synthaxe : /msg <nom_utilisateur> <message>");
              trame = trame_set_sender_name(trame,"Server");
              do_write(fds[i].fd,trame);
            }
            else{
              space = 0;
              while(buffer[space+strlen("/msg ")] != ' ') space++;
              strncpy(pseudo,buffer+strlen("/msg "),space);
              if(user_look_for_pseudo(user_list,pseudo) == 0){
                trame = trame_set_sender_name(trame,"Server");
                trame = trame_set_message(trame,"Aucun utilisateur ne possède cet identifiant");
                do_write(fds[i].fd,trame);
                break;
              }
              sprintf(envoie,"[%s] %s",user_pseudo(user_list,fds[i].fd),buffer+1+space+strlen("/msg ") );
              trame = trame_set_sender_name(trame,user_pseudo(user_list,fds[i].fd));
              trame = trame_set_message(trame,buffer+1+space+strlen("/msg "));
              unicast(fds[i].fd,trame,user_list,pseudo);
            }
            break;
          }

          // commande create--------------------------------------
          if(strncmp(buffer,"/create",strlen("/create")) == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_message(trame,"Veuillez respecter la synthaxe : /create <nom_salon>");
            }
            else{
              strcpy(information,buffer+strlen("/create "));
              if(channel_look_for_name(channel_list,information) == 0 && user_appartient_channel(user_look_for_user(user_list,fds[i].fd)) == 1){
                channel_list = channel_add(channel_list,information);
                sprintf(envoie,"You have created channel : %s",information);
                trame = trame_set_sender_name(trame,information);
                trame = trame_set_message(trame,envoie);
              }
              else{
                trame = trame_set_message(trame,"This name of channel is already used");
              }
            }
            trame = trame_set_sender_name(trame,"Server");
            do_write(fds[i].fd,trame);
            break;
          }

          // commande join ---------------------------------
          if(strncmp(buffer,"/join",strlen("/join")) == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_message(trame,"Veuillez respecter la synthaxe : /join <nom_salon>");
            }
            else{
              strcpy(information,buffer+strlen("/join "));
              if(channel_look_for_name(channel_list,information) == 1 && user_appartient_channel(user_look_for_user(user_list,fds[i].fd)) == 1){
                channel_list = channel_up_number_member(channel_list,information);
                user_list = user_change_name_channel(user_list,information,fds[i].fd);
                sprintf(envoie,"You have joined : %s",information);
                trame = trame_set_sender_name(trame,information);
                trame = trame_set_message(trame,envoie);
              }
              else{
                trame = trame_set_message(trame,"This channel does not exist or you already are in a channel");
              }
            }
            trame = trame_set_sender_name(trame,"Server");
            do_write(fds[i].fd,trame);
            break;
          }

          //commande /channel
          if(strncmp(buffer,"/channel",strlen("/channel")) == 0){
            trame=trame_set_sender_name(trame,"Server");
            strcpy(envoie,channel_display_list(channel_list));
            trame=trame_set_message(trame,envoie);
            do_write(fds[i].fd,trame);
            break;
          }

          // multicast channel ---------------------------------
          if(strcmp(user_channel_name(user_look_for_user(user_list,fds[i].fd)),"Unspecified channel") != 0){
            trame = trame_set_channel_name(trame,user_pseudo(user_list,fds[i].fd));
            trame = trame_set_sender_name(trame,user_channel_name(user_look_for_user(user_list,fds[i].fd)));
            trame = trame_set_message(trame,buffer);
            multicast(fds[i].fd,trame,user_list,user_channel_name(user_look_for_user(user_list,fds[i].fd)));
            break;
          }

          // commande /send--------------------------------------

          if(strncmp(buffer,"/send",strlen("/send")) == 0){
            if(verification_synthaxe(buffer,-1) == 1){
              trame = trame_set_message(trame,"Veuillez respecter la synthaxe : /send <nom_utilisateur_destination> <lien_fichier>");
              trame = trame_set_sender_name(trame,"Server");
              do_write(fds[i].fd,trame);
            }
            else{
              space = 0;
              while(buffer[space+strlen("/send ")] != ' ') space++;
              char file_name[MSG_SIZE];
              file_name_size = 0;

              while(buffer[strlen(buffer)-file_name_size] != '/') file_name_size++;  //on cherche la taille du nom du fichier que l'on envoie

              strcpy(file_name,buffer+strlen(buffer)-file_name_size+1);
              strncpy(pseudo,buffer+strlen("/send "),space);

              if(user_look_for_pseudo(user_list,pseudo) == 0){
                trame = trame_set_sender_name(trame,"Server");
                trame = trame_set_message(trame,"Aucun utilisateur ne possède cet identifiant");
                do_write(fds[i].fd,trame);
                break;
              }

              sprintf(envoie,"wants you to accept the transfer of the file \"%s\" . Do you accept? [Y/N]",file_name);
              trame = trame_set_file(trame,file_name);
              trame = trame_set_message(trame,envoie);
              trame = trame_set_sender_name(trame,user_pseudo(user_list,fds[i].fd));
              trame = trame_set_port(trame,port);
              unicast(fds[i].fd,trame,user_list,pseudo);
              user_list = user_change_send_to(user_list,pseudo,fds[i].fd);
              user_list = user_change_receive_from(user_list,pseudo,fds[i].fd);
            }
            break;
          }


          //on regarde si le recpeteur est ok pour recevoir le fichier
          if(strncmp(buffer,"Y",1) == 0 && user_receive_from(user_list,fds[i].fd) != -1 ){
              trame = trame_set_message(trame,"accepted file transfert");
              trame = trame_set_sender_name(trame,user_pseudo(user_list,fds[i].fd));
              trame = trame_set_port(trame,port);
              unicast(fds[i].fd,trame,user_list,user_pseudo(user_list,user_receive_from(user_list,fds[i].fd)));
              user_list = user_change_receive_from(user_list,user_pseudo(user_list,fds[i].fd),-1);
              port++;
              break;
          }

          //on regarde si le recepteur ne veut pas recevoir le fichier
          if(strncmp(buffer,"N",1) == 0 && user_receive_from(user_list,fds[i].fd) != -1 ){
              trame = trame_set_message(trame,"cancelled file transfert");
              trame = trame_set_sender_name(trame,user_pseudo(user_list,fds[i].fd));
              unicast(fds[i].fd,trame,user_list,user_pseudo(user_list,user_receive_from(user_list,fds[i].fd)));
              user_list = user_change_send_to(user_list,user_pseudo(user_list,fds[i].fd),-1);
              user_list = user_change_receive_from(user_list,user_pseudo(user_list,fds[i].fd),-1);
              break;
          }


          //on revoie le même message que le client nous a envoyé---------------------------------------------
          trame = trame_set_message(trame,buffer);
          trame = trame_set_sender_name(trame,"Server");
          do_write(fds[i].fd,trame);

        }
      }
    }
  }


  //on ferme la socket d'écoute------------------------------------------------------
  printf("Fermeture socket serveur\n");
  close_socket(socket);
  return 0;

}
