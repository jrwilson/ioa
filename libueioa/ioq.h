#ifndef __ioq_h__
#define __ioq_h__

#include <ueioa.h>
#include <sys/time.h>

typedef enum {
  IO_ALARM,
  IO_WRITE,
  IO_READ
} io_type_t;

typedef struct {
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
} io_t;

typedef struct ioq_struct ioq_t;

ioq_t* ioq_create (void);
void ioq_destroy (ioq_t*);

int ioq_interrupt_fd (ioq_t*);
size_t ioq_size (ioq_t*);
bool ioq_empty (ioq_t*);

void ioq_insert_alarm (ioq_t*, aid_t, time_t, suseconds_t);
void ioq_insert_write (ioq_t*, aid_t, int);
void ioq_insert_read (ioq_t*, aid_t, int);

void ioq_pop (ioq_t*, io_t*);
/* void ioq_purge_aid (ioq_t*, aid_t); */
/* void ioq_purge_aid_param (ioq_t*, aid_t, void*); */

#endif /* __ioq_h__ */
