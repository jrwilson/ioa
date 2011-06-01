#include "ioq.hh"

#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <algorithm>

ioq::ioq ()
{
  pthread_mutex_init (&m_mutex, NULL);
  if (pipe (m_pipes) != 0) {
    perror ("pipe");
    exit (EXIT_FAILURE);
  }
}

ioq::~ioq ()
{
  pthread_mutex_destroy (&m_mutex);  
}

int
ioq::interrupt_fd ()
{
  return m_pipes[0];
}

size_t
ioq::size ()
{
  size_t retval;
  pthread_mutex_lock (&m_mutex);
  retval = m_ios.size ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

bool
ioq::empty ()
{
  bool retval;
  pthread_mutex_lock (&m_mutex);
  retval = m_ios.empty ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

void
ioq::push (const io& io)
{
  pthread_mutex_lock (&m_mutex);
  if (std::find (m_ios.begin (),
		 m_ios.end (),
		 io) == m_ios.end ()) {
    m_ios.push_back (io);
  }
  pthread_mutex_unlock (&m_mutex);
  char c = 0;
  assert (write (m_pipes[1], &c, 1) == 1);
}

void
ioq::insert_alarm (aid_t aid, time_t secs, suseconds_t usecs)
{
  assert (aid != -1);
  assert (usecs >= 0 && usecs <= 999999);

  io io;
  io.type = IO_ALARM;
  io.aid = aid;
  io.alarm.tv.tv_sec = secs;
  io.alarm.tv.tv_usec = usecs;

  push (io);
}

void
ioq::insert_write (aid_t aid, int fd)
{
  assert (aid != -1);

  io io;
  io.type = IO_WRITE;
  io.aid = aid;
  io.write.fd = fd;

  push (io);
}

void
ioq::insert_read (aid_t aid, int fd)
{
  assert (aid != -1);

  io io;
  io.type = IO_READ;
  io.aid = aid;
  io.read.fd = fd;

  push (io);
}

ioq::io
ioq::pop ()
{
  pthread_mutex_lock (&m_mutex);
  io r = m_ios.front ();
  m_ios.pop_front ();
  pthread_mutex_unlock (&m_mutex);
  return r;
}
