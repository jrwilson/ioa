#ifndef __bind_runnable_hpp__
#define __bind_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/system.hpp>

namespace ioa {

  template <class OI, class OM, class II, class IM, class C, class D>
  class bind_runnable :
    public runnable_interface
  {
  private:
    const action<OI, OM> m_output_action;
    const action<II, IM> m_input_action;
    const automaton_handle<C> m_automaton;
    D& m_d;
    
  public:
    bind_runnable (const action<OI, OM> output_action,
		   const action<II, IM> input_action,
		   const automaton_handle<C>& automaton,
		   D& d) :
      m_output_action (output_action),
      m_input_action (input_action),
      m_automaton (automaton),
      m_d (d)
    { }
    
    void operator() () {
      system::bind (m_output_action, m_input_action, m_automaton, m_d);
    }
  };
  
  template <class OI, class OM, class II, class IM, class C, class D>
  bind_runnable<OI, OM, II, IM, C, D>* make_bind_runnable (const action<OI, OM> output_action,
							   const action<II, IM> input_action,
							   const automaton_handle<C>& automaton,
							   D& d) {
    return new bind_runnable<OI, OM, II, IM, C, D> (output_action, input_action, automaton, d);
  }
  
}

#endif
