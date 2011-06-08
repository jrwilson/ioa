#ifndef __self_helper_hpp__
#define __self_helper_hpp__

namespace ioa {
  
  template <class T>
  class self_helper :
    public observable
  {
  private:
    automaton_handle<T> m_handle;
    
  public:
    typedef T instance;
    
    self_helper ()
    {
      m_handle = scheduler::get_current_aid ();
    }
    
    automaton_handle<T> get_handle () const {
      return m_handle;
    }
    
  };

}

#endif
