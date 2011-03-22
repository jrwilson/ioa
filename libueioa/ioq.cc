#include "ioq.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "table.hh"

bool
Io::operator== (const Io& io) const
{
  if (type != io.type) {
    return false;
  }
  if (aid != io.aid) {
    return false;
  }
  switch (type) {
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

Ioq::Ioq () :
  m_index (m_table)
{
  pthread_mutex_init (&m_mutex, NULL);
  if (pipe (m_pipes) != 0) {
    perror ("pipe");
    exit (EXIT_FAILURE);
  }
}

Ioq::~Ioq ()
{
  pthread_mutex_destroy (&m_mutex);  
}

int
Ioq::interrupt_fd ()
{
  return m_pipes[0];
}

size_t
Ioq::size ()
{
  size_t retval;
  pthread_mutex_lock (&m_mutex);
  retval = m_index.size ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

bool
Ioq::empty ()
{
  bool retval;
  pthread_mutex_lock (&m_mutex);
  retval = m_index.empty ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

void
Ioq::push (const Io& io)
{
  pthread_mutex_lock (&m_mutex);
  m_index.insert_unique (io);
  pthread_mutex_unlock (&m_mutex);
  char c = 0;
  assert (write (m_pipes[1], &c, 1) == 1);
}

void
Ioq::insert_alarm (aid_t aid, time_t secs, suseconds_t usecs)
{
  assert (aid != -1);
  assert (usecs >= 0 && usecs <= 999999);

  Io io;
  io.type = IO_ALARM;
  io.aid = aid;
  io.alarm.tv.tv_sec = secs;
  io.alarm.tv.tv_usec = usecs;

  push (io);
}

void
Ioq::insert_write (aid_t aid, int fd)
{
  assert (aid != -1);

  Io io;
  io.type = IO_WRITE;
  io.aid = aid;
  io.write.fd = fd;

  push (io);
}

void
Ioq::insert_read (aid_t aid, int fd)
{
  assert (aid != -1);

  Io io;
  io.type = IO_READ;
  io.aid = aid;
  io.read.fd = fd;

  push (io);
}

Io
Ioq::pop ()
{
  pthread_mutex_lock (&m_mutex);
  Io r = m_index.front ();
  m_index.pop_front ();
  pthread_mutex_unlock (&m_mutex);
  return r;
}
