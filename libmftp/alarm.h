#ifndef __alarm_h__
#define __alarm_h__

#include <ueioa.h>

extern descriptor_t alarm_descriptor;

typedef struct {
  time_t secs;
  suseconds_t usecs;
} alarm_set_in_t;

void alarm_set_in (void*, void*, bid_t);

bid_t alarm_alarm_out (void*, void*);

#endif /* __alarm_h__ */
