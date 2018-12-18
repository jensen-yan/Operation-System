#include "string.h"
#include "mailbox.h"
#include "lock.h"
#include "syscall.h"

#define MAX_NUM_BOX 32
static mailbox_t mboxs[MAX_NUM_BOX];
static mutex_lock_t mbox_lock;

void mbox_init()
{
    int i;
    mutex_lock_init(&mbox_lock);
    for(i = 0; i < MAX_NUM_BOX; i++)
    {
        mboxs[i].name[0] = '\0';
        mboxs[i].msg_front = mboxs[i].msg_rear = 0;
        mboxs[i].used_sz = 0;
        mboxs[i].occur = 0;
        condition_init(&mboxs[i].full);
        condition_init(&mboxs[i].empty);
        mutex_lock_init(&mboxs[i].mutex);
    }
}

mailbox_t *mbox_open(char *name)
{
    int mailbox_id, i;
    mailbox_id = -1;
    mutex_lock_acquire(&mbox_lock);
    for(i = 0; i < MAX_NUM_BOX; i++)
    {
        if(mboxs[i].name[0] == '\0' && mailbox_id < 0)
        {
            mailbox_id = i;
            mboxs[i].occur = 1;
            continue;
        }
        else if(!strcmp(mboxs[i].name, name))
        {
            mailbox_id = i;
            mboxs[i].occur++;
            break;
        }
    }
    if(mboxs[mailbox_id].occur == 1)
        strcpy(mboxs[mailbox_id].name, name);
    mutex_lock_release(&mbox_lock);
    return &mboxs[mailbox_id];
}

void mbox_close(mailbox_t *mailbox)
{
    mutex_lock_acquire(&mbox_lock);
    --mailbox->occur;
    if(mailbox->occur <= 0)
    {
        mailbox->name[0] = '\0';
        mailbox->msg_front = mailbox->msg_rear = 0;
        mailbox->used_sz = 0;
        mailbox->occur = 0;
        condition_init(&mailbox->full);
        condition_init(&mailbox->empty);
        mutex_lock_init(&mailbox->mutex);
    }
    mutex_lock_release(&mbox_lock);
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
    mutex_lock_acquire(&mailbox->mutex);
    while(MSG_MAX_SZ - mailbox->used_sz < msg_length)
        condition_wait(&mailbox->mutex, &mailbox->empty);
    /* copy */
    if(MSG_MAX_SZ - mailbox->msg_rear < msg_length)
    {
        memcpy((uint8_t *)(mailbox->msg + mailbox->msg_rear), (uint8_t *)msg, MSG_MAX_SZ - mailbox->msg_rear);
        mailbox->msg_rear = msg_length - (MSG_MAX_SZ - mailbox->msg_rear);
        memcpy((uint8_t *)mailbox->msg, (uint8_t *)(msg + msg_length - mailbox->msg_rear), mailbox->msg_rear);
    }
    else
    {
        memcpy((uint8_t *)(mailbox->msg + mailbox->msg_rear), (uint8_t *)msg, msg_length);
        mailbox->msg_rear += msg_length;
    }
    //memcpy((uint8_t *)(mailbox->msg + mailbox->msg_rear), (uint8_t *)msg, msg_length);
    //mailbox->msg_rear += msg_length;
    mailbox->used_sz += msg_length;
    condition_broadcast(&mailbox->full);
    mutex_lock_release(&mailbox->mutex);
}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
    mutex_lock_acquire(&mailbox->mutex);
    while(mailbox->used_sz < msg_length)
    {
        condition_wait(&mailbox->mutex, &mailbox->full);
    }
    /* copy */
    if(MSG_MAX_SZ - mailbox->msg_front < msg_length)
    {
        memcpy((uint8_t *)msg, (uint8_t *)(mailbox->msg + mailbox->msg_front), MSG_MAX_SZ - mailbox->msg_front);
        mailbox->msg_front = msg_length - (MSG_MAX_SZ - mailbox->msg_front);
        memcpy((uint8_t *)(msg + msg_length - mailbox->msg_front), (uint8_t *)mailbox->msg, mailbox->msg_front);
    }
    else
    {
        memcpy((uint8_t *)msg, (uint8_t *)(mailbox->msg + mailbox->msg_front), msg_length);
        mailbox->msg_front += msg_length;
    }
    //memcpy((uint8_t *)msg, (uint8_t *)(mailbox->msg + mailbox->msg_front), msg_length);
    //mailbox->msg_front += msg_length;
    mailbox->used_sz -= msg_length;
    condition_broadcast(&mailbox->empty);
    mutex_lock_release(&mailbox->mutex);
}