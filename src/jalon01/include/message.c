#include "message.h"


struct trame{
  char sender_name[MSG_SIZE];
  char message[MSG_SIZE];
  char file_name[MSG_SIZE];
  int msg_size;
  int port;
  char channel_name[MSG_SIZE];


};

size_t size_struc_trame(){
  return sizeof(struct trame);
}


struct trame *trame_init(struct trame *trame){
  trame= malloc(sizeof(struct trame));
  strcpy(trame->sender_name,"");
  strcpy(trame->message,"");
  strcpy(trame->file_name,"");
  trame->msg_size=0;
  trame->port=0;
  strcpy(trame->channel_name,"");
  return trame;


}

struct trame *trame_set_to_zero(struct trame *trame){
  memset(trame->sender_name,'\0',MSG_SIZE);
  memset(trame->message,'\0',MSG_SIZE);
  memset(trame->file_name,'\0',MSG_SIZE);
  trame->msg_size=0;
  trame->port=0;
  memset(trame->channel_name,'\0',MSG_SIZE);
  strcpy(trame->channel_name,"Unspecified channel");

  return trame;
}

struct trame *trame_set_message(struct trame *trame,char message[]){
  strcpy(trame->message,message);
  return trame;
}

struct trame *trame_set_sender_name(struct trame *trame,char sender_name[]){
  strcpy(trame->sender_name,sender_name);
  return trame;
}

struct trame *trame_set_port(struct trame *trame,int port){
  trame->port=port;
  return trame;
}

struct trame *trame_set_msg_size(struct trame *trame,int msg_size){
  trame->msg_size=msg_size;
  return trame;
}

struct trame *trame_set_file(struct trame *trame,char file[]){
  strcpy(trame->file_name,file);
  return trame;
}

struct trame *trame_set_channel_name(struct trame *trame,char channel_name[]){
  strcpy(trame->channel_name,channel_name);
  return trame;
}


char *trame_message(struct trame *trame){
  return trame->message;
}

char *trame_sender_name(struct trame *trame){
  return trame->sender_name;
}

char *trame_channel_name(struct trame *trame){
  return trame->channel_name;
}

char *trame_file_name(struct trame *trame){
  return trame->file_name;
}

int trame_port(struct trame *trame){
  return trame->port;
}
