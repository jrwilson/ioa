#ifndef __runnable_interface_hpp__
#define __runnable_interface_hpp__

#include <ioa/shared_mutex.hpp>

namespace ioa {

  class runnable_interface
  {
  private:
    // TODO:  Make the scheuler keep track of this.
    // We keep track of the number of runnables in existence.
    // When the count reaches 0, we can stop.
    static shared_mutex m_mutex;
    static size_t m_count;

  public:
    runnable_interface ();
    virtual ~runnable_interface ();
    static size_t count ();
    virtual void operator() () = 0;
  };

}

#endif
