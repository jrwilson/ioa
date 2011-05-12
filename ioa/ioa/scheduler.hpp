#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include <boost/thread.hpp>
#include <queue>
#include "system.hpp"
#include "runnable.hpp"

namespace ioa {

  template <class S>
  class scheduler_wrapper
  {
  private:
    S& m_scheduler;
  public:
    scheduler_wrapper (S& scheduler) :
      m_scheduler (scheduler)
    { }

    template <class C>
    void create (const C* ptr,
		 automaton_interface* instance) {
      m_scheduler.create (ptr, instance);
    }
    
    template <class C>
    void declare (const C* ptr,
		  void* parameter) {
      m_scheduler.declare (ptr, parameter);
    }

    template <class C, class OI, class OM, class II, class IM>
    void bind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      m_scheduler.bind (ptr, output_automaton, output_member_ptr, input_automaton, input_member_ptr);
    }

    template <class OI, class OM, class OP, class II, class IM>
    void bind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      m_scheduler.bind (ptr, output_member_ptr, output_parameter, input_automaton, input_member_ptr);
    }
    
    template <class OI, class OM, class II, class IM, class IP>
    void bind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter) {
      m_scheduler.bind (ptr, output_automaton, output_member_ptr, input_member_ptr, input_parameter);
    }

    template <class C, class OI, class OM, class II, class IM>
    void unbind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      m_scheduler.unbind (ptr, output_automaton, output_member_ptr, input_automaton, input_member_ptr);
    }

    template <class OI, class OM, class OP, class II, class IM>
    void unbind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      m_scheduler.unbind (ptr, output_member_ptr, output_parameter, input_automaton, input_member_ptr);
    }
    
    template <class OI, class OM, class II, class IM, class IP>
    void unbind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter) {
      m_scheduler.unbind (ptr, output_automaton, output_member_ptr, input_member_ptr, input_parameter);
    }

    template <class C>
    void rescind (const C* ptr,
		  const generic_parameter_handle& parameter) {
      m_scheduler.rescind (ptr, parameter);
    }

    template <class C>
    void destroy (const C* ptr,
		  const generic_automaton_handle& automaton) {
      m_scheduler.destroy (ptr, automaton);
    }

    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr) {
      m_scheduler.schedule (ptr, member_ptr);
    }

    template <class T>
    void run (T* instance) {
      m_scheduler.run (instance);
    }

    void clear (void) {
      m_scheduler.clear ();
    }
  };

}

#endif
