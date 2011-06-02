#ifndef __destroy_runnable_hpp__
#define __destroy_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/system.hpp>

namespace ioa {
  
  template <class C, class I, class D>
  class destroy_runnable :
    public runnable_interface
  {
  private:
    const automaton_handle<C> m_automaton;
    const automaton_handle<I> m_target;
    D& m_d;
    
  public:
    destroy_runnable (const automaton_handle<C>& automaton,
		      const automaton_handle<I>& target,
		      D& d) :
      m_automaton (automaton),
      m_target (target),
      m_d (d)
    { }
    
    void operator() () {
      system::destroy (m_automaton, m_target, m_d);
    }
  };
  
  template <class C, class I, class D>
  destroy_runnable<C, I, D>* make_destroy_runnable (const automaton_handle<C>& automaton,
						    const automaton_handle<I>& target,
						    D& d) {
    return new destroy_runnable<C, I, D> (automaton, target, d);
  }
  
}

#endif
