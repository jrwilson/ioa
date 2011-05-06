#ifndef __runnable_hpp__
#define __runnable_hpp__

#include <boost/thread/shared_mutex.hpp>
#include "system.hpp"

namespace ioa {

  class runnable;

  class internal_scheduler_interface :
    public scheduler_interface
  {
  public:
    virtual void schedule (runnable*) = 0;
  };

  class runnable
  {
  private:
    static boost::shared_mutex m_mutex;
    static size_t m_count;

  protected:
    internal_scheduler_interface& m_scheduler;
  public:
    runnable (internal_scheduler_interface& scheduler) :
      m_scheduler (scheduler)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      ++m_count;
    }
    virtual ~runnable () {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      --m_count;
    }
    static size_t count () {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      return m_count;
    }
    virtual void operator() (system&) = 0;
  };

}

#endif
