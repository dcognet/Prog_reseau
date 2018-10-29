#include "server_tools.h"

//------------------------------------------------------------------------------

void error(const char *msg){
  perror(msg);
  exit(1);
}

//------------------------------------------------------------------------------

int do_socket(int domain, int type, int protocol) {
  int socket1 = socket(domain,type,protocol);
  int yes = 1;

  if(socket1 == -1){
    error("ERROR socket creation");
  }
  if (setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    error("ERROR setting socket options");
  }

  return socket1;
}

//------------------------------------------------------------------------------

void get_addr_info(const char* port, struct sockaddr_in* serv_addr) {

  int portno;

  memset(serv_addr,'\0',sizeof(serv_addr));
  portno = atoi(port);
  serv_addr->sin_family = AF_INET;
  serv_addr->sin_addr.s_addr = INADDR_ANY;
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

int do_accept(int socket, struct sockaddr_in *pointeur_host_addr){
  size_t host_addr_size = sizeof(struct sockaddr_in);
  int i = accept(socket, (struct sockaddr *)pointeur_host_addr,(socklen_t *)&host_addr_size);
  printf("Je me connecte avec l'adresse : %s\n",inet_ntoa(pointeur_host_addr->sin_addr));

  if(i == -1){
    error("ERROR accepte server");
  }
  return i;
}

//------------------------------------------------------------------------------

int do_read(int socket, char *buffer){
  int i = read(socket,buffer,MSG_SIZE);
  if(i == -1){
    error("ERROR read server");
  }
  return i;
}

//------------------------------------------------------------------------------

void do_write(int fd, const void *buffer){
  size_t i = write(fd,buffer,MSG_SIZE);
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
