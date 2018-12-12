#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include "sem.h"
#include "cond.h"

#define MSG_MAX_SZ 100

typedef struct mailbox
{
    char name[25];
    uint8_t msg[MSG_MAX_SZ];
    int msg_front, msg_rear;
    int used_sz;    

    int occur;
    
    condition_t full;
    condition_t empty;
    mutex_lock_t mutex;
} mailbox_t;

void mbox_init();
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif