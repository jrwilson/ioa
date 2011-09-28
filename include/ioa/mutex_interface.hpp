#ifndef __mutex_interface_hpp__
#define __mutex_interface_hpp__

namespace ioa {

  class mutex_interface
  {
  public:
    virtual ~mutex_interface () { }
    virtual void lock () = 0;
    virtual void unlock () = 0;
  };

}

#endif
