#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Fonctions -------------------------------------------------------------------

int do_socket(int domain, int type, int protocol) {
  int socket1;
  int yes = 1;

  //create the socket1
  socket1 = socket(domain,type,protocol);

  //check for socket1 validity
  if(socket1 == -1){
    error("ERROR socket1 creation");
  }

  // set socket1 option, to prevent "already in use" issue when rebooting the server right on
  if (setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    error("ERROR setting socket options");
  }

  return socket1;
}


//------------------------------------------------------------------------------


void get_addr_info(const char* port, struct sockaddr_in* serv_addr,const char* host) {

  int portno;
  int iphost;

  //clean the serv_add structure
  memset(serv_addr,'\0',sizeof(serv_addr));

  //cast the port from a string to an int
  portno = atoi(port);

  iphost=inet_addr(host);

  //internet family protocol
  serv_addr->sin_family = AF_INET;

  //we bind to any ip form the host
    serv_addr->sin_addr.s_addr = htonl(iphost);

  //we bind on the tcp port specified
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
  int i = send(socket,message, strlen(message),0);
  if(i == -1){
    error("ERROR envoi message server");
  }
}

//------------------------------------------------------------------------------

void do_read(int socket, char *buffer){
  int i = read(socket,buffer,255);
  if(i == -1){
    error("ERROR read server");
  }
}


//Corps-------------------------------------------------------------------------


int main(int argc,char** argv){
  char buffer[256];

  if (argc != 3)
  {
    fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
    return 1;
  }


    //get the socket--------------------------------------------------------------
    printf("Création socket\n");
    int socket = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);


    //init the serv_add structure-------------------------------------------------
    printf("Informations serveur\n");
    struct sockaddr_in pointeur_serv_addr;
    get_addr_info(argv[1], &pointeur_serv_addr,argv[2]);


    //connect to remote socket----------------------------------------------------
    printf("Connexion serveur\n");
    do_connect(socket,pointeur_serv_addr);
    while(1){


    //get user input--------------------------------------------------------------
    printf("Lecture saisie\n");
    const char saisie[256];
    gets(saisie);
    const void* msg = saisie;


    //send message to the server--------------------------------------------------
    printf("Envoi message utilisateur\n");
    handle_client_message(socket,msg);


    //read what the client has to say---------------------------------------------
    printf("Lecture message initial\n");
    memset (buffer, '\0', sizeof (buffer));
    do_read(socket,buffer);
    printf("Le message reçu est: %s\n",buffer);


    //connexion end---------------------------------------------------------------
    if(strcmp(buffer, "/quit") == 0 ){
      printf("Fermeture connexion client\n");
      break;
    }

  }

  //close socket----------------------------------------------------------------
  printf("Fermeture socket\n");
  close(socket);

  return 0;


}
