#include <ueioa.h>

#include <assert.h>
#include <stdlib.h>

typedef struct queue_item_struct queue_item_t;
struct queue_item_struct {
  bid_t bid;
  queue_item_t* next;
};

struct bidq_struct {
  queue_item_t* front;
  queue_item_t** back;
};

bidq_t*
bidq_create (void)
{
  bidq_t* bidq = malloc (sizeof (bidq_t));
  bidq->front = NULL;
  bidq->back = &bidq->front;
  return bidq;
}

void
bidq_push_back (bidq_t* bidq, bid_t bid)
{
  assert (bidq != NULL);

  queue_item_t* item = malloc (sizeof (queue_item_t));
  item->bid = bid;
  item->next = NULL;
  *bidq->back = item;
  bidq->back = &item->next;
}

bool
bidq_empty (bidq_t* bidq)
{
  assert (bidq != NULL);
  return bidq->front == NULL;
}

bid_t
bidq_front (bidq_t* bidq)
{
  assert (bidq != NULL);
  assert (bidq->front != NULL);
  return bidq->front->bid;
}

void
bidq_pop_front (bidq_t* bidq)
{
  assert (bidq != NULL);
  assert (bidq->front != NULL);

  queue_item_t* temp = bidq->front;
  bidq->front = temp->next;
  free (temp);

  if (bidq->front == NULL) {
    bidq->back = &bidq->front;
  }
}

