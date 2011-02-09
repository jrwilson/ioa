#include "queue.h"

#include <assert.h>
#include <string.h>

#include "xstdlib.h"

/* TODO: Reimplement using slab allocation. */

typedef struct element_struct element_t;
struct element_struct {
  element_t* next;
  unsigned char data[0];
};

struct queue_struct {
  size_t elsize;
  size_t size;
  element_t* front;
  element_t** back;
};

queue_t*
queue_create (size_t elsize)
{
  assert (elsize > 0);
  queue_t* queue = xmalloc (sizeof (queue_t));
  queue->elsize = elsize;
  queue->size = 0;
  queue->front = NULL;
  queue->back = &queue->front;
  return queue;
}

void
queue_destroy (queue_t* queue)
{
  assert (queue != NULL);

  while (queue->front) {
    element_t* temp = queue->front;
    queue->front = queue->front->next;
    xfree (temp);
  }
  xfree (queue);
}

bool
queue_empty (queue_t* queue)
{
  assert (queue != NULL);
  return queue->front == NULL;
}

size_t
queue_size (queue_t* queue)
{
  assert (queue != NULL);
  return queue->size;
}

void
queue_clear (queue_t* queue)
{
  assert (queue != NULL);
  while (!queue_empty (queue)) {
    queue_pop (queue);
  }
}

void
queue_push (queue_t* queue, void* ptr)
{
  assert (queue != NULL);
  assert (ptr != NULL);
  element_t* element = xmalloc (sizeof (element_t) + queue->elsize);
  memcpy (element->data, ptr, queue->elsize);
  *queue->back = element;
  queue->back = &element->next;
  ++queue->size;
}

void
queue_pop (queue_t* queue)
{
  assert (queue != NULL);
  assert (!queue_empty (queue));
  element_t* temp = queue->front;
  queue->front = temp->next;
  if (queue->front == NULL) {
    queue->back = &queue->front;
  }
  xfree (temp);
  --queue->size;
}

void*
queue_front (queue_t* queue)
{
  assert (queue != NULL);
  assert (!queue_empty (queue));
  return queue->front->data;
}
