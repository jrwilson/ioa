#include <ioa/time.hpp>
#include <cassert>

#define MILLION 1000000

namespace ioa {

  void time::check () {
    assert ((m_sec <= 0 && m_usec <= 0 && m_usec > -MILLION) ||
	    (m_sec >= 0 && m_usec >= 0 && m_usec < MILLION));
  }
  
  void time::normalize () {
    if (m_usec <= -MILLION || m_usec >= MILLION) {
      m_sec += m_usec / MILLION;
      m_usec = m_usec % MILLION;
    }
    
    if (m_sec > 0 && m_usec < 0) {
      m_sec -= 1;
      m_usec += MILLION;
    }
    else if (m_sec < 0 && m_usec > 0) {
      m_sec += 1;
      m_usec -= MILLION;
    }
    check ();
  }

  long long time::compare (const time& o) const {
    return ((long long)(m_sec) * (long long)(MILLION) + (long long)(m_usec)) -
      ((long long)(o.m_sec) * (long long)(MILLION) + (long long)(o.m_usec));
  }

  time::time () :
    m_sec (0),
    m_usec (0)
  { }
  
  time::time (long sec,
	      long usec) :
    m_sec (sec),
    m_usec (usec)
  {
    normalize ();
  }
  
  time::time (const time& o) {
    m_sec = o.m_sec;
    m_usec = o.m_usec;
  }
  
  time::time (const struct timeval& t) :
    m_sec (t.tv_sec),
    m_usec (t.tv_usec)
  {
    check ();
  }
  
  long time::sec () const {
    return m_sec;
  }

  
  long time::usec () const {
    return m_usec;
  }

  time& time::operator= (const time& o) {
    if (this != &o) {
      m_sec = o.m_sec;
      m_usec = o.m_usec;
      check ();
    }
    return *this;
  }

  time time::operator+ (const time& o) const {
    time result;
    result.m_sec = m_sec + o.m_sec;
    result.m_usec = m_usec + o.m_usec;
    result.normalize ();
    return result;
  }
  
  time time::operator- (const time& o) const {
    time result;
    result.m_sec = m_sec - o.m_sec;
    result.m_usec = m_usec - o.m_usec;
    result.normalize ();
    return result;
  }

  time& time::operator+= (const time& o) {
    m_sec += o.m_sec;
    m_usec += o.m_usec;
    normalize ();
    return *this;
  }

  time& time::operator-= (const time& o) {
    m_sec -= o.m_sec;
    m_usec -= o.m_usec;
    normalize ();
    return *this;
  }

  bool time::operator== (const time& o) const {
    return compare (o) == 0;
  }

  bool time::operator!= (const time& o) const {
    return compare (o) != 0;
  }

  bool time::operator> (const time& o) const {
    return compare (o) > 0;
  }

  bool time::operator< (const time& o) const {
    return compare (o) < 0;
  }

  bool time::operator>= (const time& o) const {
    return compare (o) >= 0;
  }

  bool time::operator<= (const time& o) const {
    return compare (o) <= 0;
  }
    
  time::operator struct timeval () const {
    assert (m_sec >= 0 && m_usec >= 0);
    struct timeval retval;
    retval.tv_sec = m_sec;
    retval.tv_usec = m_usec;
    return retval;
  }

  time time::now () {
    timeval tv;
    int result = gettimeofday (&tv, 0);
    assert (result == 0);
    return time (tv);
  }

}
