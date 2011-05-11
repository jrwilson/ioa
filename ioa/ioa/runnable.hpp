#ifndef __runnable_hpp__
#define __runnable_hpp__

#include <boost/thread/shared_mutex.hpp>

namespace ioa {

  class runnable_interface
  {
  private:
    static boost::shared_mutex m_mutex;
    static size_t m_count;

  public:
    runnable_interface ()
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      ++m_count;
    }
    virtual ~runnable_interface () {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      --m_count;
    }
    static size_t count () {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      return m_count;
    }

    virtual void operator() () = 0;
  };

}

#endif
