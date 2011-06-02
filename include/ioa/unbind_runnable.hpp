#ifndef __unbind_runnable_hpp__
#define __unbind_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/system.hpp>

namespace ioa {
  
  template <class C, class D>
  class unbind_runnable :
    public runnable_interface
  {
  private:
    const bid_t m_bid;
    const automaton_handle<C> m_automaton;
    D& m_d;
    
  public:
    unbind_runnable (const bid_t bid,
		     const automaton_handle<C>& automaton,
		     D& d) :
      m_bid (bid),
      m_automaton (automaton),
      m_d (d)
    { }
    
    void operator() () {
      system::unbind (m_bid, m_automaton, m_d);
    }
  };
  
  template <class C, class D>
  unbind_runnable<C, D>* make_unbind_runnable (const bid_t bid,
					       const automaton_handle<C>& automaton,
					       D& d) {
    return new unbind_runnable<C, D> (bid, automaton, d);
  }
  
}

#endif
