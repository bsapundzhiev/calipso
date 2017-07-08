#ifndef _QUEUE_H
#define _QUEUE_H
/*
struct dllist
{
	void* data;
	struct dllist* next;
	struct dllist* prev;
};
*/
typedef struct dlqlist {
    struct dllist* head;
    struct dllist* tail;
} queue_t;

struct dlqlist * queue_new();
int queue_enqueue(struct dlqlist*, void*);
int queue_dequeue(struct dlqlist*);
struct dlqlist*  queue_delete( struct dlqlist* s );
int queue_remove(struct dlqlist*s, void * elem);
void queue_print(struct dlqlist* );

#endif