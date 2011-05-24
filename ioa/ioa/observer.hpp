#ifndef __observer_hpp__
#define __observer_hpp__

namespace ioa {

  class observer
  {
  public:
    virtual ~observer () { }
    virtual void observe () = 0;
    virtual void stop_observing () = 0;
  };

  class observable
  {
  public:
    virtual ~observable () { }
    virtual void add_observer (observer*) = 0;
    virtual void remove_observer (observer*) = 0;
  };

}

#endif
