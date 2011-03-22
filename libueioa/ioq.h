#ifndef __ioq_h__
#define __ioq_h__

#include <ueioa.h>
#include <sys/time.h>

typedef enum {
  IO_ALARM,
  IO_WRITE,
  IO_READ
} io_type_t;

class Io {
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
  bool operator== (const Io& io) const;
};

class Ioq {
 public:
  Ioq ();
  ~Ioq ();

  int interrupt_fd ();
  size_t size ();
  bool empty ();
  
  void insert_alarm (aid_t, time_t, suseconds_t);
  void insert_write (aid_t, int);
  void insert_read (aid_t, int);
  
  Io pop ();

 private:
  pthread_mutex_t m_mutex;
  int m_pipes[2];
  typedef std::list<Io> IoList;
  IoList m_ios;

  void push (const Io& io);

};

#endif /* __ioq_h__ */
