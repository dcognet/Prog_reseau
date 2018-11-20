#include "server_channel.h"
#include "user_tools.h"

//------------------------------------------------------------------------------

struct channel{
  char channel_name[MSG_SIZE];
  struct channel *next_channel;
  int nombre_membre;
};

//------------------------------------------------------------------------------

struct channel *channel_create(char *channel_name){
  struct channel *new_channel = malloc(sizeof(struct channel));
  strcpy(new_channel->channel_name,channel_name);
  new_channel->next_channel = NULL;
  new_channel->nombre_membre = 0;
  return new_channel;
}

//------------------------------------------------------------------------------

int channel_list_size(struct channel *list_channel){
  int size = 0;
  if (list_channel == NULL)
  return 0;
  while (list_channel != NULL){
    size++;
    list_channel = list_channel->next_channel;
  }
  return size;
}

//Add une new user--------------------------------------------------------------

struct channel *channel_add(struct channel * list_channel, char *channel_name){
  struct channel *new_channel = channel_create(channel_name);

  if (channel_list_size(list_channel) == 0){
    new_channel->next_channel = list_channel;
    return new_channel;
  }
  else {
    struct channel *temp = list_channel;
    while(temp->next_channel != NULL){
      temp = temp->next_channel;
    }
    temp->next_channel = new_channel;
    return list_channel;
  }
}

//------------------------------------------------------------------------------

int channel_look_for_name(struct channel *list_channel,char *channel_name){
  while (list_channel != NULL){
    if(strcmp(list_channel->channel_name,channel_name) == 0)
    return 1;
    list_channel = list_channel->next_channel;
  }
  return 0;
}

//------------------------------------------------------------------------------

struct channel *channel_up_number_member(struct channel *list_channel, char *channel_name){
  struct channel *temp = list_channel;
  while(temp != NULL){
    if(strcmp(temp->channel_name,channel_name) == 0){
      temp->nombre_membre = temp->nombre_membre + 1;
      printf("On vient d'ajouter un membre, il y en a maintenant : %i\n", temp->nombre_membre);
    }
    temp = temp->next_channel;
  }
  return list_channel;
}

//------------------------------------------------------------------------------

struct channel *channel_down_number_member(struct channel * list_channel, char *channel_name){
  struct channel *temp = list_channel;
  while(temp != NULL){
    if(strcmp(temp->channel_name,channel_name) == 0){
      temp->nombre_membre = temp->nombre_membre - 1;
      printf("On vient d'enlever un membre, il y en a maintenant : %i\n", temp->nombre_membre);
    }
    temp = temp->next_channel;
  }
  return list_channel;
}

//------------------------------------------------------------------------------

struct channel *channel_delete(struct channel *channel_list,char *channel_name){

  struct channel *temp = channel_list;

  if (channel_list == NULL){
    return NULL;
  }

  //Supprimer premier maillon

  if(strcmp(temp->channel_name,channel_name) == 0){
    return temp->next_channel;
  }

  while(temp->next_channel != NULL){

    //Supprimer dernier maillon

    if ((temp->next_channel)->next_channel == NULL && strcmp((temp->next_channel)->channel_name,channel_name) == 0) {
      temp->next_channel = NULL;
      break;
    }

    //Supprimer maillon entre le premier et le dernier

    if(strcmp((temp->next_channel)->channel_name,channel_name) == 0){
      temp->next_channel = (temp->next_channel)->next_channel;
    }
    temp = temp->next_channel;
  }
  return channel_list;
}

//------------------------------------------------------------------------------

struct channel *channel_look_for_channel(struct channel *channel_list,char *channel_name){
  if (channel_list == NULL){
    return NULL;
  }
  while (channel_list != NULL){
    if(strcmp(channel_list->channel_name,channel_name) == 0){
      return channel_list;
    }
    channel_list = channel_list->next_channel;
  }
  return NULL;
}

//------------------------------------------------------------------------------

int channel_nombre_membre(struct channel* channel){
  int i = channel->nombre_membre;
  return i;
}


//Display the list of channel -----------------------------------------------------

char *channel_display_list(struct channel *channel_list){
  char *buffer = malloc(sizeof(char)*MSG_SIZE);
  memset(buffer,'\0',MSG_SIZE);
  strcpy(buffer,"The channel are :");
  char channel[MSG_SIZE];
  memset(channel,'\0',MSG_SIZE);

  if(channel_list_size(channel_list)==0){
    strcpy(buffer,"There is not any channel");
    return buffer;
  }

  while (channel_list!= NULL){
    strcat(channel,"\n -");
    strcat(channel,channel_list->channel_name);
    channel_list = channel_list->next_channel;
  }
  strcat(buffer,channel);
  return buffer;
}
