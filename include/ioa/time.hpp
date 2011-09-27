/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __time_hpp__
#define __time_hpp__

#include <sys/time.h>

// Class for representing time offsets.

namespace ioa {

  class time
  {
  private:
    // These should either both be positive or both be negative.
    long m_sec;
    long m_usec;
    
    void check ();
    void normalize ();
    long long compare (const time& o) const;

  public:
    time ();
    time (long sec,
	  long usec);
    time (const time& o);
    time (const struct timeval& t);
    long sec () const;
    long usec () const;
    time& operator= (const time& o);
    time operator+ (const time& o) const;
    time operator- (const time& o) const;
    time& operator+= (const time& o);
    time& operator-= (const time& o);
    bool operator== (const time& o) const;
    bool operator!= (const time& o) const;
    bool operator> (const time& o) const;
    bool operator< (const time& o) const;
    bool operator>= (const time& o) const;
    bool operator<= (const time& o) const;
    operator struct timeval () const;
    static time now ();
  };

}

#endif
