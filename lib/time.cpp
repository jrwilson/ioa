#include <ioa/time.hpp>
#include <cassert>

namespace ioa {

  void time::check () {
    assert ((sec <= 0 && usec <= 0 && usec > -1000000) ||
	    (sec >= 0 && usec >= 0 && usec < 1000000));
  }
  
  void time::normalize () {
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

  time::time () :
    sec (0),
    usec (0)
  { }
  
  time::time (long sec,
	      long usec) :
    sec (sec),
    usec (usec)
  {
    check ();
  }
  
  time::time (const time& o) {
    sec = o.sec;
    usec = o.usec;
    check ();
  }
  
  time::time (const struct timeval& t) :
    sec (t.tv_sec),
    usec (t.tv_usec)
  {
    check ();
  }
  
  time& time::operator= (const time& o) {
    if (this != &o) {
      sec = o.sec;
      usec = o.usec;
      check ();
    }
    return *this;
  }
  
  bool time::operator== (const time& o) const {
    return sec == o.sec && usec == o.usec;
  }
  
  time& time::operator+= (const time& o) {
    sec += o.sec;
    usec += o.usec;
    normalize ();
    return *this;
  }
  
  time time::operator- (const time& o) const {
    time result;
    result.sec = sec - o.sec;
    result.usec = usec - o.usec;
    result.normalize ();
    return result;
  }
  
  bool time::operator< (const time& o) const {
    if (sec != o.sec) {
      return sec < o.sec;
    }
    return usec < o.usec;
  }
  
  time::operator struct timeval () const {
    struct timeval retval;
    retval.tv_sec = sec;
    retval.tv_usec = usec;
    return retval;
  }

}
