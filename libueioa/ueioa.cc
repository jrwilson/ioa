#include "ueioa.h"

#include <assert.h>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>

#include "ioq.hh"
#include "runq.hh"
#include "automata.hh"
#include "receipts.hh"
#include "table.hh"

#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 128

static Ioq ioq;
static Runq runq;
static Automata automata;
static Buffers buffers;
static Receipts receipts;

static void*
thread_func (void* arg)
{
  for (;;) {
    runnable_t runnable;
    runq.pop (&runnable);
    switch (runnable.type) {
    case SYSTEM_INPUT:
      automata.system_input_exec (receipts, runq, buffers, runnable.aid);
      break;
    case SYSTEM_OUTPUT:
      automata.system_output_exec (receipts, runq, ioq, buffers, runnable.aid);
      break;
    case ALARM_INPUT:
      automata.alarm_input_exec (buffers, runnable.aid);
      break;
    case READ_INPUT:
      automata.read_input_exec (buffers, runnable.aid);
      break;
    case WRITE_INPUT:
      automata.write_input_exec (buffers, runnable.aid);
      break;
    case FREE_INPUT:
      automata.free_input_exec (buffers, runnable.free_input.caller_aid, runnable.aid, runnable.free_input.free_input, runnable.free_input.bid);
      break;
    case OUTPUT:
      automata.output_exec (buffers, runnable.aid, runnable.output.output, runnable.param);
      break;
    case INTERNAL:
      automata.internal_exec (runnable.aid, runnable.internal.internal, runnable.param);
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
alarm_aid_equal (const void* x0, const void* y0)
{
  const alarm_t* x = (const alarm_t*)x0;
  const alarm_t* y = (const alarm_t*)y0;

  return x->aid == y->aid;
}

static bool
alarm_lt (const void* x0, const void* y0)
{
  const alarm_t* x = (const alarm_t*)x0;
  const alarm_t* y = (const alarm_t*)y0;

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
fd_aid_equal (const void* x0, const void* y0)
{
  const fd_t* x = (const fd_t*)x0;
  const fd_t* y = (const fd_t*)y0;

  return x->aid == y->aid;
}

typedef struct {
  fd_set fds;
} fdset_t;

static void
fdset_set (const void* e, void* a)
{
  const fd_t* fd = (const fd_t*)e;
  fdset_t* fdset = (fdset_t*)a;

  FD_SET (fd->fd, &fdset->fds);
}

static bool
fdset_clear_read (const void* e, const void* a)
{
  const fd_t* fd = (const fd_t*)e;
  const fdset_t* fdset = (const fdset_t*)a;

  if (FD_ISSET (fd->fd, &fdset->fds)) {
    runq.insert_read_input (fd->aid);
    /* FD_CLR (fd->fd, &fdset->fds); */
    return true;
  }
  else {
    return false;
  }
}

static bool
fdset_clear_write (const void* e, const void* a)
{
  const fd_t* fd = (const fd_t*)e;
  const fdset_t* fdset = (const fdset_t*)a;

  if (FD_ISSET (fd->fd, &fdset->fds)) {
    runq.insert_write_input (fd->aid);
    /* FD_CLR (fd->fd, &fdset->fds); */
    return true;
  }
  else {
    return false;
  }
}

void
ueioa_run (const descriptor_t* descriptor, const void* arg, int thread_count)
{

  assert (descriptor != NULL);
  assert (thread_count > 0 && thread_count <= MAX_THREADS);
  
  pthread_t a_thread[MAX_THREADS];
  
  automata.create_automaton (receipts, runq, descriptor, arg);
  
  /* Spawn some threads. */
  int idx;
  for (idx = 0; idx < thread_count; ++idx) {
    if (pthread_create (&a_thread[idx], NULL, thread_func, NULL) != 0) {
      perror ("pthread_create");
      exit (EXIT_FAILURE);
    }
  }

  /* Start the I/O thread. */
  // int interrupt = ioq.interrupt_fd (ioq);
  // fdset_t read_arg;
  // FD_ZERO (&read_arg.fds);
  // fdset_t write_arg;
  // FD_ZERO (&write_arg.fds);
  // struct timeval timeout;
  // io_t io;
  // table* alarm_table = new table (sizeof (alarm_t));
  // index_t* alarm_index = index_create_ordered_list (alarm_table, alarm_lt);
  // table* write_table = new table (sizeof (fd_t));
  // index_t* write_index = index_create_list (write_table);
  // table* read_table = new table (sizeof (fd_t));
  // index_t* read_index = index_create_list (read_table);

  // for (;;) {
  //   FD_ZERO (&read_arg.fds);
  //   FD_ZERO (&write_arg.fds);

  //   /* Add the interrupt. */
  //   FD_SET (interrupt, &read_arg.fds);

  //   /* Initialize the reads. */
  //   index_for_each (read_index,
  // 		    index_begin (read_index),
  // 		    index_end (read_index),
  // 		    fdset_set,
  // 		    &read_arg);

  //   /* Initialize the writes. */
  //   index_for_each (write_index,
  // 		    index_begin (write_index),
  // 		    index_end (write_index),
  // 		    fdset_set,
  // 		    &write_arg);

  //   /* Initialize the timeout. */
  //   struct timeval* timeout_ptr;
  //   if (index_empty (alarm_index)) {
  //     /* Wait forever. */
  //     timeout_ptr = NULL;
  //   }
  //   else {
  //     alarm_t* alarm = (alarm_t*)index_front (alarm_index);
  //     alarm_t now;
  //     gettimeofday (&now.tv, NULL);
  //     if (alarm_lt (alarm, &now)) {
  // 	/* In the past so poll. */
  // 	timeout.tv_sec = 0;
  // 	timeout.tv_usec = 0;
  //     }
  //     else {
  // 	/* Make relative. */
  // 	timeout.tv_sec = alarm->tv.tv_sec;
  // 	timeout.tv_usec = alarm->tv.tv_usec;

  // 	if (timeout.tv_usec < now.tv.tv_usec) {
  // 	  /* Borrow. */
  // 	  --timeout.tv_sec;
  // 	  timeout.tv_usec += 1000000;
  // 	}
  // 	timeout.tv_sec -= now.tv.tv_sec;
  // 	timeout.tv_usec -= now.tv.tv_usec;
  //     }
  //     timeout_ptr = &timeout;
  //   }

  //   int res = select (FD_SETSIZE, &read_arg.fds, &write_arg.fds, NULL, timeout_ptr);
  //   if (res < 0) {
  //     perror ("select");
  //     exit (EXIT_FAILURE);
  //   }
  //   else {
  //     /* Process the timers. */
  //     while (!index_empty (alarm_index)) {
  // 	alarm_t* alarm = (alarm_t*)index_front (alarm_index);
  // 	alarm_t now;
  // 	gettimeofday (&now.tv, NULL);
  // 	if (alarm_lt (alarm, &now)) {
  // 	  runq_insert_alarm_input (runq, alarm->aid);
  // 	  index_pop_front (alarm_index);
  // 	}
  // 	else {
  // 	  break;
  // 	}
  //     }

  //     if (res > 0) {
  // 	/* Process the file descriptors. */
  // 	index_remove (read_index,
  // 		      index_begin (read_index),
  // 		      index_end (read_index),
  // 		      fdset_clear_read,
  // 		      &read_arg);

  // 	index_remove (write_index,
  // 		      index_begin (write_index),
  // 		      index_end (write_index),
  // 		      fdset_clear_write,
  // 		      &write_arg);

  // 	if (FD_ISSET (interrupt, &read_arg.fds)) {
  // 	  char c;
  // 	  assert (read (interrupt, &c, 1) == 1);
  // 	  if (!ioq_empty (ioq)) {
  // 	    /* Pushing an I/O operation writes to the file descriptor.
  // 	       However, the push might not actually do anything if it is a duplicate.
  // 	       Consequently, one must check that the queue is not empty.
  // 	    */
  // 	    ioq_pop (ioq, &io);
	    
  // 	    switch (io.type) {
  // 	    case IO_ALARM:
  // 	      {
  // 		/* Get the current time. */
  // 		struct timeval tv;
  // 		gettimeofday (&tv, NULL);
  // 		/* Add the requested interval. */
  // 		tv.tv_sec += io.alarm.tv.tv_sec;
  // 		tv.tv_usec += io.alarm.tv.tv_usec;
  // 		if (tv.tv_usec > 999999) {
  // 		  ++tv.tv_sec;
  // 		  tv.tv_usec -= 1000000;
  // 		}
  // 		/* Insert alarm. */
  // 		alarm_t key;
  // 		key.aid = io.aid;
  // 		key.tv = tv;
  // 		index_insert_unique (alarm_index, alarm_aid_equal, &key);
  // 	      }
  // 	      break;
  // 	    case IO_WRITE:
  // 	      {
  // 		fd_t key;
  // 		key.aid = io.aid;
  // 		key.fd = io.write.fd;
  // 		index_insert_unique (write_index, fd_aid_equal, &key);
  // 	      }
  // 	      break;
  // 	    case IO_READ:
  // 	      {
  // 		fd_t key;
  // 		key.aid = io.aid;
  // 		key.fd = io.read.fd;
  // 		index_insert_unique (read_index, fd_aid_equal, &key);
  // 	      }
  // 	      break;
  // 	    }
  // 	  }
  // 	}
  //     }
  //   }
  // }

  /* This code is never reached. */
  for (idx = 0; idx < thread_count; ++idx) {
    pthread_join (a_thread[idx], NULL);
  }
}

int
schedule_system_output (void)
{
  aid_t aid = automata.get_current_aid ();
  if (automata.system_output_exists (aid)) {
    runq.insert_system_output (automata.get_current_aid ());
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_alarm_input (time_t secs, suseconds_t usecs)
{
  aid_t aid = automata.get_current_aid ();
  if (automata.alarm_input_exists (aid)) {
    ioq.insert_alarm (automata.get_current_aid (), secs, usecs);
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_read_input (int fd)
{
  aid_t aid = automata.get_current_aid ();
  if (automata.read_input_exists (aid)) {
    ioq.insert_read (automata.get_current_aid (), fd);
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_write_input (int fd)
{
  aid_t aid = automata.get_current_aid ();
  if (automata.write_input_exists (aid)) {
    ioq.insert_write (automata.get_current_aid (), fd);
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_free_input (aid_t aid, input_t free_input, bid_t bid)
{
  aid_t caller_aid = automata.get_current_aid ();
  /*Check that free_input and bid exist. */
  if (automata.free_input_exists (aid, free_input) &&
      buffers.exists (caller_aid, bid)) {
    /* Increment the reference count on the buffer. */
    runq.insert_free_input (caller_aid, aid, free_input, bid);
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_output (output_t output, void* param)
{
  aid_t aid = automata.get_current_aid ();
  if (automata.output_exists (aid, output, param)) {
    runq.insert_output (aid, output, param);
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_internal (internal_t internal, void* param)
{
  aid_t aid = automata.get_current_aid ();
  if (automata.internal_exists (aid, internal, param)) {
    runq.insert_internal (aid, internal, param);
    return 0;
  }
  else {
    return -1;
  }
}

bid_t
buffer_alloc (size_t size)
{
  return buffers.alloc (automata.get_current_aid (), size);
}

bid_t
buffer_alloc_aligned (size_t size, size_t alignment)
{
  return buffers.alloc_aligned (automata.get_current_aid (), size, alignment);
}

void*
buffer_write_ptr (bid_t bid)
{
  return buffers.write_ptr (automata.get_current_aid (), bid);
}

const void*
buffer_read_ptr (bid_t bid)
{
  return buffers.read_ptr (automata.get_current_aid (), bid);
}

size_t
buffer_size (bid_t bid)
{
  return buffers.size (automata.get_current_aid (), bid);
}

void
buffer_incref (bid_t bid)
{
  buffers.incref (automata.get_current_aid (), bid);
}

void
buffer_decref (bid_t bid)
{
  buffers.decref (automata.get_current_aid (), bid);
}

void
buffer_add_child (bid_t parent, bid_t child)
{
  buffers.add_child (automata.get_current_aid (), parent, child);
}

void
buffer_remove_child (bid_t parent, bid_t child)
{
  buffers.remove_child (automata.get_current_aid (), parent, child);
}

bid_t
buffer_dup (bid_t bid, size_t size)
{
  return buffers.dup (automata.get_current_aid (), bid, size);
}
