#ifndef __time_hpp__
#define __time_hpp__

#include <sys/time.h>

// Class for representing time offsets.

namespace ioa {

  class time
  {
  private:
    // These should either both be positive or both be negative.
    long sec;
    long usec;
    
    void check ();
    void normalize ();

  public:
    time ();
    time (long sec,
	  long usec);
    time (const time& o);
    time (const struct timeval& t);
    long get_sec () const;
    long get_usec () const;
    time& operator= (const time& o);
    bool operator== (const time& o) const;
    time& operator+= (const time& o);
    time operator+ (const time& o) const;
    time operator- (const time& o) const;
    bool operator< (const time& o) const;
    bool operator> (const time& o) const;
    operator struct timeval () const;
    static time now ();
  };

}

#endif
