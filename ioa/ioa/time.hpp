#ifndef __time_hpp__
#define __time_hpp__

#include <sys/time.h>
#include <boost/assert.hpp>

namespace ioa {

  struct time
  {
    // These should either both be positive or both be negative.
    long sec;
    long usec;

    void check () {
      BOOST_ASSERT ((sec <= 0 && usec <= 0 && usec > -1000000) ||
		    (sec >= 0 && usec >= 0 && usec < 1000000));
    }

    void normalize () {
      if (usec < -1000000 || usec > 1000000) {
	sec += usec / 1000000;
	usec = usec % 1000000;
      }

      if (sec > 0 && usec < 0) {
	sec -= 1;
	usec += 1000000;
      }
      else if (sec < 0 && usec > 0) {
	sec += 1;
	usec -= 1000000;
      }
      check ();
    }

    time () :
      sec (0),
      usec (0)
    { }
    
    time (long sec,
	  long usec) :
      sec (sec),
      usec (usec)
    {
      check ();
    }

    time (const time& o) {
      sec = o.sec;
      usec = o.usec;
      check ();
    }

    time (const struct timeval& t) :
      sec (t.tv_sec),
      usec (t.tv_usec)
    {
      check ();
    }

    time& operator= (const time& o) {
      if (this != &o) {
	sec = o.sec;
	usec = o.usec;
	check ();
      }
      return *this;
    }

    bool operator== (const time& o) const {
      return sec == o.sec && usec == o.usec;
    }

    time& operator+= (const time& o) {
      sec += o.sec;
      usec += o.usec;
      normalize ();
      return *this;
    }

    time operator- (const time& o) const {
      time result;
      result.sec = sec - o.sec;
      result.usec = usec - o.usec;
      result.normalize ();
      return result;
    }

    bool operator< (const time& o) const {
      if (sec != o.sec) {
	return sec < o.sec;
      }
      return usec < o.usec;
    }

    operator struct timeval () const {
      struct timeval retval;
      retval.tv_sec = sec;
      retval.tv_usec = usec;
      return retval;
    }
  };

}

#endif
