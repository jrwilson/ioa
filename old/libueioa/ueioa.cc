#include "ueioa.hh"

#include <cassert>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <cstdlib>

#include "ioq.hh"
#include "runq.hh"
#include "automata.hh"
#include "receipts.hh"

#define MAX_THREADS 128

static ioq ioq;
static runq runq;
static automata automata;
static buffers buffers;
static receipts receipts;

static void*
thread_func (void* arg)
{
  for (;;) {
    runq::runnable runnable = runq.pop ();
    switch (runnable.type) {
    case runq::SYSTEM_INPUT:
      automata.system_input_exec (receipts, runq, buffers, runnable.aid);
      break;
    case runq::SYSTEM_OUTPUT:
      automata.system_output_exec (receipts, runq, ioq, buffers, runnable.aid);
      break;
    case runq::ALARM_INPUT:
      automata.alarm_input_exec (buffers, runnable.aid);
      break;
    case runq::READ_INPUT:
      automata.read_input_exec (buffers, runnable.aid);
      break;
    case runq::WRITE_INPUT:
      automata.write_input_exec (buffers, runnable.aid);
      break;
    case runq::FREE_INPUT:
      automata.free_input_exec (buffers, runnable.free_input.caller_aid, runnable.aid, runnable.free_input.free_input, runnable.free_input.bid);
      break;
    case runq::OUTPUT:
      automata.output_exec (buffers, runnable.aid, runnable.output.output, runnable.param);
      break;
    case runq::INTERNAL:
      automata.internal_exec (runnable.aid, runnable.internal.internal, runnable.param);
      break;
    }
  }

  pthread_exit (NULL);
}

bool operator< (const struct timeval& x, const struct timeval& y)
{
  if (x.tv_sec != y.tv_sec) {
    return x.tv_sec < y.tv_sec;
  }
  return x.tv_usec < y.tv_usec;
}

struct alarm_t {
  aid_t aid;
  struct timeval tv;
  alarm_t (const aid_t& _aid, const struct timeval& _tv) :
    aid (_aid),
    tv (_tv) { }
  bool operator< (const alarm_t& x) const {
    return tv < x.tv;
  }
};
    
class alarm_aid_equal {
private:
  const aid_t m_aid;
public:
  alarm_aid_equal (const aid_t& aid) :
    m_aid (aid) { }
  bool operator () (const alarm_t& alarm) const {
    return m_aid == alarm.aid;
  }
};

struct fd_t {
  aid_t aid;
  int fd;
  fd_t (const aid_t& _aid, const int& _fd) :
    aid (_aid),
    fd (_fd) { }
};

class fd_aid_equal {
private:
  const aid_t m_aid;
public:
  fd_aid_equal (const aid_t& aid) :
    m_aid (aid) { }
  bool operator() (const fd_t& x) const {
    return m_aid == x.aid;
  }
};

typedef struct {
  fd_set fds;
} fdset_t;

class fdset_set {
private:
  fdset_t& m_fdset;
public:
  fdset_set (fdset_t& fdset) :
    m_fdset (fdset) { }
  void operator() (const fd_t& fd) const { FD_SET (fd.fd, &m_fdset.fds); }
};

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
  int interrupt = ioq.interrupt_fd ();
  fdset_t read_arg;
  FD_ZERO (&read_arg.fds);
  fdset_t write_arg;
  FD_ZERO (&write_arg.fds);
  struct timeval timeout;
  std::set<alarm_t> alarm_index;
  std::list<fd_t> write_index;
  std::list<fd_t> read_index;

  for (;;) {
    FD_ZERO (&read_arg.fds);
    FD_ZERO (&write_arg.fds);

    /* Add the interrupt. */
    FD_SET (interrupt, &read_arg.fds);

    /* Initialize the reads. */
    std::for_each (read_index.begin (),
		   read_index.end (),
		   fdset_set (read_arg));

    /* Initialize the writes. */
    std::for_each (write_index.begin (),
		   write_index.end (),
		   fdset_set (write_arg));

    /* Initialize the timeout. */
    struct timeval* timeout_ptr;
    if (alarm_index.empty ()) {
      /* Wait forever. */
      timeout_ptr = NULL;
    }
    else {
      std::set<alarm_t>::const_iterator pos = alarm_index.begin ();
      struct timeval now;
      gettimeofday (&now, NULL);
      if (pos->tv < now) {
  	/* In the past so poll. */
  	timeout.tv_sec = 0;
  	timeout.tv_usec = 0;
      }
      else {
  	/* Make relative. */
  	timeout.tv_sec = pos->tv.tv_sec;
  	timeout.tv_usec = pos->tv.tv_usec;

  	if (timeout.tv_usec < now.tv_usec) {
  	  /* Borrow. */
  	  --timeout.tv_sec;
  	  timeout.tv_usec += 1000000;
  	}
  	timeout.tv_sec -= now.tv_sec;
  	timeout.tv_usec -= now.tv_usec;
      }
      timeout_ptr = &timeout;
    }

    int res = select (FD_SETSIZE, &read_arg.fds, &write_arg.fds, NULL, timeout_ptr);
    if (res < 0) {
      perror ("select");
      exit (EXIT_FAILURE);
    }
    else {
      /* Process the timers. */
      while (!alarm_index.empty ()) {
	std::set<alarm_t>::const_iterator pos = alarm_index.begin ();
  	struct timeval now;
  	gettimeofday (&now, NULL);
  	if (pos->tv < now) {
  	  runq.insert_alarm_input (pos->aid);
  	  alarm_index.erase (pos);
  	}
  	else {
  	  break;
  	}
      }

      if (res > 0) {
  	/* Process the file descriptors. */
	std::list<fd_t>::iterator pos;
	pos = read_index.begin ();
	while (pos != read_index.end ()) {
	  if (FD_ISSET (pos->fd, &read_arg.fds)) {
	    runq.insert_read_input (pos->aid);
	    /* FD_CLR (fd->fd, &fdset->fds); */
	    pos = read_index.erase (pos);
	  }
	  else {
	    ++pos;
	  }
	}

	pos = write_index.begin ();
	while (pos != write_index.end ()) {
	  if (FD_ISSET (pos->fd, &write_arg.fds)) {
	    runq.insert_write_input (pos->aid);
	    /* FD_CLR (fd->fd, &fdset->fds); */
	    pos = write_index.erase (pos);
	  }
	  else {
	    ++pos;
	  }
	}

  	if (FD_ISSET (interrupt, &read_arg.fds)) {
  	  char c;
  	  assert (read (interrupt, &c, 1) == 1);
  	  if (!ioq.empty ()) {
  	    /* Pushing an I/O operation writes to the file descriptor.
  	       However, the push might not actually do anything if it is a duplicate.
  	       Consequently, one must check that the queue is not empty.
  	    */
	    ioq::io io = ioq.pop ();
	    
  	    switch (io.type) {
  	    case ioq::IO_ALARM:
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
		if (std::find_if (alarm_index.begin (),
				  alarm_index.end (),
				  alarm_aid_equal (io.aid)) == alarm_index.end ()) {
		  alarm_index.insert (alarm_t (io.aid, tv));
		}
  	      }
  	      break;
  	    case ioq::IO_WRITE:
  	      {
		if (std::find_if (write_index.begin (),
				  write_index.end (),
				  fd_aid_equal (io.aid)) == write_index.end ()) {
		  write_index.push_back (fd_t (io.aid, io.write.fd));
		}
  	      }
  	      break;
  	    case ioq::IO_READ:
  	      {
		if (std::find_if (read_index.begin (),
				  read_index.end (),
				  fd_aid_equal (io.aid)) == read_index.end ()) {
		  read_index.push_back (fd_t (io.aid, io.read.fd));
		}
  	      }
  	      break;
  	    }
  	  }
  	}
      }
    }
  }

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
