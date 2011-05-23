#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include "automaton_handle.hpp"
#include "parameter_handle.hpp"

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

    template <class C, class G, class D>
    void create (const C* ptr,
		 G generator,
		 D& d) {
      m_scheduler.create (ptr, generator, d);
    }
    
    template <class C, class P, class D>
    void declare (const C* ptr,
		  P* parameter,
		  D& d) {
      m_scheduler.declare (ptr, parameter, d);
    }

    template <class C, class OI, class OM, class II, class IM, class D>
    void bind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       D& d) {
      m_scheduler.bind (ptr, output_automaton, output_member_ptr, input_automaton, input_member_ptr, d);
    }

    template <class OI, class OM, class OP, class II, class IM, class D>
    void bind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       D& d) {
      m_scheduler.bind (ptr, output_member_ptr, output_parameter, input_automaton, input_member_ptr, d);
    }
    
    template <class OI, class OM, class II, class IM, class IP, class D>
    void bind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter,
	       D& d) {
      m_scheduler.bind (ptr, output_automaton, output_member_ptr, input_member_ptr, input_parameter, d);
    }

    template <class C, class OI, class OM, class II, class IM, class D>
    void unbind (const C* ptr,
		 const automaton_handle<OI>& output_automaton,
		 OM OI::*output_member_ptr,
		 const automaton_handle<II>& input_automaton,
		 IM II::*input_member_ptr,
		 D& d) {
      m_scheduler.unbind (ptr, output_automaton, output_member_ptr, input_automaton, input_member_ptr, d);
    }
    
    template <class OI, class OM, class OP, class II, class IM, class D>
    void unbind (const OI* ptr,
		 OM OI::*output_member_ptr,
		 const parameter_handle<OP>& output_parameter,
		 const automaton_handle<II>& input_automaton,
		 IM II::*input_member_ptr,
		 D& d) {
      m_scheduler.unbind (ptr, output_member_ptr, output_parameter, input_automaton, input_member_ptr, d);
    }
    
    template <class OI, class OM, class II, class IM, class IP, class D>
    void unbind (const II* ptr,
		 const automaton_handle<OI>& output_automaton,
		 OM OI::*output_member_ptr,
		 IM II::*input_member_ptr,
		 const parameter_handle<IP>& input_parameter,
		 D& d) {
      m_scheduler.unbind (ptr, output_automaton, output_member_ptr, input_member_ptr, input_parameter, d);
    }

    template <class C, class P, class D>
    void rescind (const C* ptr,
		  const parameter_handle<P>& parameter,
		  D& d) {
      m_scheduler.rescind (ptr, parameter, d);
    }

    template <class C, class I, class D>
    void destroy (const C* ptr,
		  const automaton_handle<I>& automaton,
		  D& d) {
      m_scheduler.destroy (ptr, automaton, d);
    }

    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr) {
      m_scheduler.schedule (ptr, member_ptr);
    }

    template <class G>
    void run (G generator) {
      m_scheduler.run (generator);
    }

    void clear (void) {
      m_scheduler.clear ();
    }
  };

}

#endif
