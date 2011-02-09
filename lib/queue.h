#ifndef __queue_h__
#define __queue_h__

#include <stddef.h>
#include <stdbool.h>

typedef struct queue_struct queue_t;

queue_t* queue_create (size_t);
void queue_destroy (queue_t*);

bool queue_empty (queue_t*);
size_t queue_size (queue_t*);

void queue_clear (queue_t*);
void queue_push (queue_t*, void*);
void queue_pop (queue_t*);
void* queue_front (queue_t*);

#endif /* __queue_h__ */
