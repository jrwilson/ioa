#include "ioq.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "table.h"

static bool
io_equal (const void* x0, const void* y0)
{
  const io_t* x = (const io_t*)x0;
  const io_t* y = (const io_t*)y0;
  if (x->type != y->type) {
    return false;
  }
  if (x->aid != y->aid) {
    return false;
  }
  switch (x->type) {
  case IO_ALARM:
  case IO_WRITE:
  case IO_READ:
    return true;
    break;
  }

  /* Not reached. */
  assert (0);
  return true;
}

/* static bool */
/* runnable_aid_equal (const void* x0, const void* y0) */
/* { */
/*   const runnable_t* x = x0; */
/*   const runnable_t* y = y0; */
/*   return x->aid == y->aid; */
/* } */

/* static bool */
/* runnable_aid_param_equal (const void* x0, const void* y0) */
/* { */
/*   const runnable_t* x = x0; */
/*   const runnable_t* y = y0; */
/*   return x->aid == y->aid && x->param == y->param; */
/* } */

struct ioq_struct {
  pthread_mutex_t mutex;
  int pipes[2];
  table_t* table;
  index_t* index;
};

ioq_t*
ioq_create (void)
{
  ioq_t* ioq = (ioq_t*)malloc (sizeof (ioq_t));
  pthread_mutex_init (&ioq->mutex, NULL);
  if (pipe (ioq->pipes) != 0) {
    perror ("pipe");
    exit (EXIT_FAILURE);
  }
  ioq->table = table_create (sizeof (io_t));
  ioq->index = index_create_list (ioq->table);
  return ioq;
}

void
ioq_destroy (ioq_t* ioq)
{
  assert (ioq != NULL);
  pthread_mutex_destroy (&ioq->mutex);
  table_destroy (ioq->table);
  free (ioq);
}

int
ioq_interrupt_fd (ioq_t* ioq)
{
  assert (ioq != NULL);

  return ioq->pipes[0];
}

size_t
ioq_size (ioq_t* ioq)
{
  assert (ioq != NULL);
  size_t retval;
  pthread_mutex_lock (&ioq->mutex);
  retval = index_size (ioq->index);
  pthread_mutex_unlock (&ioq->mutex);
  return retval;
}

bool
ioq_empty (ioq_t* ioq)
{
  assert (ioq != NULL);
  bool retval;
  pthread_mutex_lock (&ioq->mutex);
  retval = index_empty (ioq->index);
  pthread_mutex_unlock (&ioq->mutex);
  return retval;
}



static void
push (ioq_t* ioq, io_t* io)
{
  assert (ioq != NULL);
  assert (io != NULL);
  pthread_mutex_lock (&ioq->mutex);
  index_insert_unique (ioq->index, io_equal, io);
  pthread_mutex_unlock (&ioq->mutex);
  char c = 0;
  assert (write (ioq->pipes[1], &c, 1) == 1);
}

void
ioq_insert_alarm (ioq_t* ioq, aid_t aid, time_t secs, suseconds_t usecs)
{
  assert (ioq != NULL);
  assert (aid != -1);
  assert (usecs >= 0 && usecs <= 999999);

  io_t io;
  io.type = IO_ALARM;
  io.aid = aid;
  io.alarm.tv.tv_sec = secs;
  io.alarm.tv.tv_usec = usecs;

  push (ioq, &io);
}

void
ioq_insert_write (ioq_t* ioq, aid_t aid, int fd)
{
  assert (ioq != NULL);
  assert (aid != -1);

  io_t io;
  io.type = IO_WRITE;
  io.aid = aid;
  io.write.fd = fd;

  push (ioq, &io);
}

void
ioq_insert_read (ioq_t* ioq, aid_t aid, int fd)
{
  assert (ioq != NULL);
  assert (aid != -1);

  io_t io;
  io.type = IO_READ;
  io.aid = aid;
  io.read.fd = fd;

  push (ioq, &io);
}

/* void */
/* ioq_insert_system_input (ioq_t* ioq, aid_t aid) */
/* { */
/*   assert (ioq != NULL); */
/*   assert (aid != -1); */

/*   runnable_t runnable = { */
/*     .type = SYSTEM_INPUT, */
/*     .aid = aid, */
/*     .param = NULL, */
/*   }; */
/*   push (ioq, &runnable); */
/* } */

/* void */
/* ioq_insert_system_output (ioq_t* ioq, aid_t aid) */
/* { */
/*   assert (ioq != NULL); */
/*   assert (aid != -1); */

/*   runnable_t runnable = { */
/*     .type = SYSTEM_OUTPUT, */
/*     .aid = aid, */
/*     .param = NULL, */
/*   }; */
/*   push (ioq, &runnable); */
/* } */

/* void */
/* ioq_insert_output (ioq_t* ioq, aid_t aid, output_t output, void* param) */
/* { */
/*   assert (ioq != NULL); */
/*   assert (aid != -1); */
/*   assert (output != NULL); */

/*   runnable_t runnable = { */
/*     .type = OUTPUT, */
/*     .aid = aid, */
/*     .param = param, */
/*   }; */
/*   runnable.output.output = output; */
/*   push (ioq, &runnable); */
/* } */

/* void */
/* ioq_insert_internal (ioq_t* ioq, aid_t aid, internal_t internal, void* param) */
/* { */
/*   assert (ioq != NULL); */
/*   assert (aid != -1); */
/*   assert (internal != NULL); */

/*   runnable_t runnable = { */
/*     .type = INTERNAL, */
/*     .aid = aid, */
/*     .param = param, */
/*   }; */
/*   runnable.internal.internal = internal; */
/*   push (ioq, &runnable); */
/* } */

void
ioq_pop (ioq_t* ioq, io_t* io)
{
  assert (ioq != NULL);
  assert (io != NULL);

  pthread_mutex_lock (&ioq->mutex);
  io_t* r = (io_t*)index_front (ioq->index);
  *io = *r;
  index_pop_front (ioq->index);
  pthread_mutex_unlock (&ioq->mutex);
}

/* void */
/* ioq_purge_aid (ioq_t* ioq, aid_t aid) */
/* { */
/*   assert (ioq != NULL); */

/*   runnable_t key = { */
/*     .aid = aid, */
/*   }; */
/*   pthread_mutex_lock (&ioq->mutex); */
/*   index_remove (ioq->index, */
/* 		index_begin (ioq->index), */
/* 		index_end (ioq->index), */
/* 		runnable_aid_equal, */
/* 		&key); */
/*   pthread_mutex_unlock (&ioq->mutex); */
/* } */

/* void */
/* ioq_purge_aid_param (ioq_t* ioq, aid_t aid, void* param) */
/* { */
/*   assert (ioq != NULL); */

/*   runnable_t key = { */
/*     .aid = aid, */
/*     .param = param, */
/*   }; */
/*   pthread_mutex_lock (&ioq->mutex); */
/*   index_remove (ioq->index, */
/* 		index_begin (ioq->index), */
/* 		index_end (ioq->index), */
/* 		runnable_aid_param_equal, */
/* 		&key); */
/*   pthread_mutex_unlock (&ioq->mutex); */
/* } */
