#include "queue.h"
#include "sched.h"

typedef pcb_t item_t;

void queue_init(queue_t *queue)
{
    queue->head = queue->tail = NULL;
}

int queue_is_empty(queue_t *queue)
{
    if (queue->head == NULL)
    {
        return 1;
    }
    return 0;
}

void queue_push(queue_t *queue, void *item)
{
    item_t *_item = (item_t *)item;
    /* queue is empty */
    if (queue->head == NULL)
    {
        queue->head = item;
        queue->tail = item;
        _item->next = NULL;
        _item->prev = NULL;
    }
    else
    {
        ((item_t *)(queue->tail))->next = item;
        _item->next = NULL;
        _item->prev = queue->tail;
        queue->tail = item;    
    }
}

void priority_queue_push(queue_t *queue, void *item)
{
    item_t *_item = (item_t *)item;
    /* queue is empty */
    if (queue->head == NULL)
    {
        queue->head = item;
        queue->tail = item;
        _item->next = NULL;
        _item->prev = NULL;
    }
    else
    {
        /*if(((item_t *)(queue->head))->priority < _item->priority)
        {
            _item->next = queue->head;
            _item->prev = NULL;
            ((item_t *)(queue->head))->prev = item;
            queue->head = item;
        }
        else
        {
            item_t *pos = queue->head;
            while(pos->next != NULL && ((item_t *)(pos->next))->priority >= _item->priority)
                pos = pos->next;    
            if(pos->next == NULL)
                queue->tail = item;
            _item->next = pos->next;
            _item->prev = pos;
            pos->next = item;
        }*/
        ((item_t *)(queue->tail))->next = item;
        _item->next = NULL;
        _item->prev = queue->tail;
        queue->tail = item;   
    }
}

void *queue_dequeue(queue_t *queue)
{
    item_t *temp = (item_t *)queue->head;

    /* this queue only has one item */
    if (temp->next == NULL)
    {
        queue->head = queue->tail = NULL;
    }
    else
    {
        queue->head = ((item_t *)(queue->head))->next;
        ((item_t *)(queue->head))->prev = NULL;
    }

    temp->prev = NULL;
    temp->next = NULL;

    return (void *)temp;
}

/* remove this item and return next item */
void *queue_remove(queue_t *queue, void *item)
{
    item_t *_item = (item_t *)item;
    item_t *next = (item_t *)_item->next;

    if (item == queue->head && item == queue->tail)
    {
        queue->head = NULL;
        queue->tail = NULL;
    }
    else if (item == queue->head)
    {
        queue->head = _item->next;
        ((item_t *)(queue->head))->prev = NULL;
    }
    else if (item == queue->tail)
    {
        queue->tail = _item->prev;
        ((item_t *)(queue->tail))->next = NULL;
    }
    else
    {
        ((item_t *)(_item->prev))->next = _item->next;
        ((item_t *)(_item->next))->prev = _item->prev;
    }

    _item->prev = NULL;
    _item->next = NULL;

    return (void *)next;
}
