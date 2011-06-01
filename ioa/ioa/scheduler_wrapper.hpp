#ifndef __scheduler_wrapper_hpp__
#define __scheduler_wrapper_hpp__

#include "automaton_handle.hpp"
#include "binding_handle.hpp"
#include "action.hpp"
#include "time.hpp"
#include "instance_generator.hpp"

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

    template <class C, class I, class D>
    void create (const C* ptr,
		 std::auto_ptr<generator_interface<I> > generator,
		 D& d) {
      m_scheduler.create (ptr, generator, d);
    }

    template <class I, class OI, class OM, class II, class IM, class D>
    void bind (const I* ptr,
	       const action<OI, OM>& output_action,
	       const action<II, IM>& input_action,
	       D& d) {
      m_scheduler.bind (ptr, output_action, input_action, d);
    }
    
    template <class C, class D>
    void unbind (const C* ptr,
		 const bid_t bid,
		 D& d) {
      m_scheduler.unbind (ptr, bid, d);
    }
    
    template <class C, class I, class D>
    void destroy (const C* ptr,
		  const automaton_handle<I>& automaton,
		  D& d) {
      m_scheduler.destroy (ptr, automaton, d);
    }

    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr,
		   time offset = time ()) {
      m_scheduler.schedule (ptr, member_ptr, offset);
    }

    template <class I>
    void run (std::auto_ptr<generator_interface<I> > generator) {
      m_scheduler.run (generator);
    }

    void clear (void) {
      m_scheduler.clear ();
    }

    template <class T>
    automaton_handle<T> get_current_aid (const T* t) {
      return m_scheduler.get_current_aid (t);
    }
  };

}

#endif
