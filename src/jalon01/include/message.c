#include "message.h"


struct trame{
  char sender_name[MSG_SIZE]; //nom de l'expediteur de la trame
  char message[MSG_SIZE]; // message à envoyer
  char file_name[MSG_SIZE]; //nom du fichier à envoyer (pour l'echange de fichier)
  int port;  //port sur lequel le recpeteur va ecouter et l'éxpéditeur va se connecter (pour l'echange de fichier)
  char channel_name[MSG_SIZE]; // nom du channel


};

//retourne la taille de la structure trame
size_t size_struc_trame(){
  return sizeof(struct trame);
}

//initialisation de la structure trame
struct trame *trame_init(struct trame *trame){
  trame = malloc(sizeof(struct trame));
  strcpy(trame->sender_name,"");
  strcpy(trame->message,"");
  strcpy(trame->file_name,"");
  trame->port = 0;
  strcpy(trame->channel_name,"");
  return trame;
}

//met tous les champs de la trame à zéro
struct trame *trame_set_to_zero(struct trame *trame){
  memset(trame->sender_name,'\0',MSG_SIZE);
  memset(trame->message,'\0',MSG_SIZE);
  memset(trame->file_name,'\0',MSG_SIZE);
  trame->port=0;
  memset(trame->channel_name,'\0',MSG_SIZE);
  strcpy(trame->channel_name,"Unspecified channel");
  return trame;
}

//met le message dans le champs message de la trame
struct trame *trame_set_message(struct trame *trame,char message[]){
  strcpy(trame->message,message);
  return trame;
}

//met le nom de l'expediteur du message dans le trame
struct trame *trame_set_sender_name(struct trame *trame,char sender_name[]){
  strcpy(trame->sender_name,sender_name);
  return trame;
}

//met le port sur lequel le recpeteur va ecouter et l'éxpéditeur va se connecter (pour l'echange de fichier)
struct trame *trame_set_port(struct trame *trame,int port){
  trame->port = port;
  return trame;
}

//met le nom du fichier à envoyer dans la trame
struct trame *trame_set_file(struct trame *trame,char file[]){
  strcpy(trame->file_name,file);
  return trame;
}

//met le nom du channel dans la trame
struct trame *trame_set_channel_name(struct trame *trame,char channel_name[]){
  strcpy(trame->channel_name,channel_name);
  return trame;
}

//renvoie le message contenue dans la trame
char *trame_message(struct trame *trame){
  return trame->message;
}

//renvoie le nom de l'expediteur contenue dans la trame
char *trame_sender_name(struct trame *trame){
  return trame->sender_name;
}

//renvoie le nom du channel contenue dans la trame
char *trame_channel_name(struct trame *trame){
  return trame->channel_name;
}

//renvoie le nom du fichier contenue dans la trame
char *trame_file_name(struct trame *trame){
  return trame->file_name;
}

//renvoie le port contenue dans la trame
int trame_port(struct trame *trame){
  return trame->port;
}
