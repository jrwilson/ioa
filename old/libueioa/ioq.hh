#ifndef __ioq_hh__
#define __ioq_hh__

#include <ueioa.hh>
#include <sys/time.h>
#include <pthread.h>
#include <list>
#include <cassert>

class ioq {
public:
  typedef enum {
    IO_ALARM,
    IO_WRITE,
    IO_READ
  } io_type_t;
  
  class io {
  public:
    io_type_t type;
    aid_t aid;
    union {
      struct {
	struct timeval tv;
      } alarm;
      struct {
	int fd;
      } write;
      struct {
	int fd;
      } read;
    };

    bool operator== (const io& io) const
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

  };
  
  ioq ();
  ~ioq ();
  
  int interrupt_fd ();
  size_t size ();
  bool empty ();
  
  void insert_alarm (aid_t, time_t, suseconds_t);
  void insert_write (aid_t, int);
  void insert_read (aid_t, int);
  
  io pop ();
  
private:  
  pthread_mutex_t m_mutex;
  int m_pipes[2];
  typedef std::list<io> io_list;
  io_list m_ios;
  
  void push (const io& io);
};

#endif /* __ioq_hh__ */
