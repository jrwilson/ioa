#include "ueioa.h"

#include <assert.h>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>

#include "ioq.h"
#include "runq.h"
#include "automata.h"
#include "receipts.h"
#include "table.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 128

static ioq_t* ioq;
static runq_t* runq;
static automata_t* automata;
static buffers_t* buffers;
static receipts_t* receipts;

static void*
thread_func (void* arg)
{
  for (;;) {
    runnable_t runnable;
    runq_pop (runq, &runnable);
    switch (runnable.type) {
    case SYSTEM_INPUT:
      automata_system_input_exec (automata, receipts, runq, buffers, runnable.aid);
      break;
    case SYSTEM_OUTPUT:
      automata_system_output_exec (automata, receipts, runq, ioq, buffers, runnable.aid);
      break;
    case OUTPUT:
      automata_output_exec (automata, buffers, runnable.aid, runnable.output.output, runnable.param);
      break;
    case INTERNAL:
      automata_internal_exec (automata, runnable.aid, runnable.internal.internal, runnable.param);
      break;
    }
  }

  pthread_exit (NULL);
}

typedef struct {
  aid_t aid;
  struct timeval tv;
} alarm_t;

static bool
alarm_aid_equal (const void* x0, void* y0)
{
  const alarm_t* x = x0;
  const alarm_t* y = y0;

  return x->aid == y->aid;
}

static bool
alarm_lt (const void* x0, void* y0)
{
  const alarm_t* x = x0;
  const alarm_t* y = y0;

  if (x->tv.tv_sec != y->tv.tv_sec) {
    return x->tv.tv_sec < y->tv.tv_sec;
  }
  return x->tv.tv_usec < y->tv.tv_usec;
}

typedef struct {
  aid_t aid;
  int fd;
} fd_t;

static bool
fd_aid_equal (const void* x0, void* y0)
{
  const fd_t* x = x0;
  const fd_t* y = y0;

  return x->aid == y->aid;
}

typedef struct {
  fd_set fds;
} fdset_t;

static void
fdset_set (const void* e, void* a)
{
  const fd_t* fd = e;
  fdset_t* fdset = a;

  FD_SET (fd->fd, &fdset->fds);
}

static bool
fdset_clear (const void* e, void* a)
{
  const fd_t* fd = e;
  fdset_t* fdset = a;

  if (FD_ISSET (fd->fd, &fdset->fds)) {
    receipts_push_write_wakeup (receipts, fd->aid);
    runq_insert_system_input (runq, fd->aid);
    FD_CLR (fd->fd, &fdset->fds);
    return true;
  }
  else {
    return false;
  }
}

void
ueioa_run (descriptor_t* descriptor, int thread_count)
{
  assert (descriptor != NULL);
  assert (descriptor_check (descriptor));
  assert (thread_count > 0 && thread_count <= MAX_THREADS);

  pthread_t a_thread[MAX_THREADS];

  ioq = ioq_create ();
  runq = runq_create ();
  automata = automata_create ();
  buffers = buffers_create ();
  receipts = receipts_create ();
  
  automata_create_automaton (automata, receipts, runq, descriptor);

  /* Spawn some threads. */
  int idx;
  for (idx = 0; idx < thread_count; ++idx) {
    if (pthread_create (&a_thread[idx], NULL, thread_func, NULL) != 0) {
      perror ("pthread_create");
      exit (EXIT_FAILURE);
    }
  }

  /* Start the I/O thread. */
  int interrupt = ioq_interrupt_fd (ioq);
  fd_set readfds;
  fdset_t write_arg;
  FD_ZERO (&write_arg.fds);
  struct timeval timeout;
  io_t io;
  table_t* alarm_table = table_create (sizeof (alarm_t));
  index_t* alarm_index = index_create_ordered_list (alarm_table, alarm_lt);
  table_t* write_table = table_create (sizeof (fd_t));
  index_t* write_index = index_create_list (write_table);

  for (;;) {
    /* Add the interrupt. */
    FD_ZERO (&readfds);
    FD_SET (interrupt, &readfds);

    /* Initialize the writes. */
    index_for_each (write_index,
		    index_begin (write_index),
		    index_end (write_index),
		    fdset_set,
		    &write_arg);

    /* Initialize the timeout. */
    struct timeval* timeout_ptr;
    if (index_empty (alarm_index)) {
      /* Wait forever. */
      timeout_ptr = NULL;
    }
    else {
      alarm_t* alarm = index_front (alarm_index);
      alarm_t now;
      gettimeofday (&now.tv, NULL);
      if (alarm_lt (alarm, &now)) {
	/* In the past so poll. */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
      }
      else {
	/* Make relative. */
	timeout.tv_sec = alarm->tv.tv_sec;
	timeout.tv_usec = alarm->tv.tv_usec;

	if (timeout.tv_usec < now.tv.tv_usec) {
	  /* Borrow. */
	  --timeout.tv_sec;
	  timeout.tv_usec += 1000000;
	}
	timeout.tv_sec -= now.tv.tv_sec;
	timeout.tv_usec -= now.tv.tv_usec;
      }
      timeout_ptr = &timeout;
    }

    int res = select (FD_SETSIZE, &readfds, &write_arg.fds, NULL, timeout_ptr);
    if (res < 0) {
      perror ("select");
      exit (EXIT_FAILURE);
    }
    else {
      /* Process the timers. */
      while (!index_empty (alarm_index)) {
	alarm_t* alarm = index_front (alarm_index);
	alarm_t now;
	gettimeofday (&now.tv, NULL);
	if (alarm_lt (alarm, &now)) {
	  receipts_push_wakeup (receipts, alarm->aid);
	  runq_insert_system_input (runq, alarm->aid);
	  index_pop_front (alarm_index);
	}
	else {
	  break;
	}
      }

      if (res > 0) {
	/* Process the file descriptors. */
	index_remove (write_index,
		      index_begin (write_index),
		      index_end (write_index),
		      fdset_clear,
		      &write_arg);
	
	if (FD_ISSET (interrupt, &readfds)) {
	  char c;
	  read (interrupt, &c, 1);
	  ioq_pop (ioq, &io);
	  
	  switch (io.type) {
	  case ALARM:
	    {
	      /* Get the current time. */
	      struct timeval tv;
	      gettimeofday (&tv, NULL);
	      /* Add the requested interval. */
	      tv.tv_sec += io.alarm.tv.tv_sec;
	      tv.tv_usec += io.alarm.tv.tv_usec;
	      if (tv.tv_usec > 999999) {
		++tv.tv_sec;
		tv.tv_usec -= 1000000;
	      }
	      /* Insert alarm. */
	      alarm_t key = {
		.aid = io.aid,
		.tv = tv,
	      };
	      index_insert_unique (alarm_index, alarm_aid_equal, &key);
	    }
	    break;
	  case WRITE:
	    {
	      fd_t key = {
		.aid = io.aid,
		.fd = io.write.fd,
	      };
	      index_insert_unique (write_index, fd_aid_equal, &key);
	    }
	    break;
	  }
	}
      }
    }
  }

  /* This code is never reached. */
  for (idx = 0; idx < thread_count; ++idx) {
    pthread_join (a_thread[idx], NULL);
  }

  receipts_destroy (receipts);
  buffers_destroy (buffers);
  automata_destroy (automata);
  runq_destroy (runq);
  ioq_destroy (ioq);
}

void
schedule_system_output (void)
{
  runq_insert_system_output (runq, automata_get_current_aid (automata));
}

int
schedule_output (output_t output, void* param)
{
  aid_t aid = automata_get_current_aid (automata);
  if (automata_output_exists (automata, aid, output, param)) {
    runq_insert_output (runq, aid, output, param);
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_internal (internal_t internal, void* param)
{
  aid_t aid = automata_get_current_aid (automata);
  if (automata_internal_exists (automata, aid, internal, param)) {
    runq_insert_internal (runq, aid, internal, param);
    return 0;
  }
  else {
    return -1;
  }
}

bid_t
buffer_alloc (size_t size)
{
  return buffers_alloc (buffers, automata_get_current_aid (automata), size);
}

void*
buffer_write_ptr (bid_t bid)
{
  return buffers_write_ptr (buffers, automata_get_current_aid (automata), bid);
}

const void*
buffer_read_ptr (bid_t bid)
{
  return buffers_read_ptr (buffers, automata_get_current_aid (automata), bid);
}

size_t
buffer_size (bid_t bid)
{
  return buffers_size (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_incref (bid_t bid)
{
  buffers_incref (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_decref (bid_t bid)
{
  buffers_decref (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_add_child (bid_t parent, bid_t child)
{
  buffers_add_child (buffers, automata_get_current_aid (automata), parent, child);
}

void
buffer_remove_child (bid_t parent, bid_t child)
{
  buffers_remove_child (buffers, automata_get_current_aid (automata), parent, child);
}

bid_t
buffer_dup (bid_t bid)
{
  return buffers_dup (buffers, automata_get_current_aid (automata), bid);
}
