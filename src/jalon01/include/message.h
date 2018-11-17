#include <string.h>
#include <stdlib.h>


#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#define MSG_SIZE 256

struct trame;

size_t size_struc_trame();


struct trame *trame_init(struct trame *trame);

struct trame *trame_set_to_zero(struct trame *trame);

struct trame *trame_set_message(struct trame *trame,char message[]);

struct trame *trame_set_sender_name(struct trame *trame,char sender_name[]);

struct trame *trame_set_port(struct trame *trame,int port);

struct trame *trame_set_msg_size(struct trame *trame,int msg_size);

struct trame *trame_set_file(struct trame *trame,char file[]);

struct trame *trame_set_channel_name(struct trame *trame,char channel[]);


char *trame_message(struct trame *trame);

char *trame_sender_name(struct trame *trame);

char *trame_channel_name(struct trame *trame);

char *trame_file_name(struct trame *trame);

int trame_port(struct trame *trame);

#endif
