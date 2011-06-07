#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include <memory>
#include <ioa/generator_interface.hpp>
#include <ioa/action.hpp>
#include <ioa/time.hpp>

/*
  The scheduler from the user's perspective.
  These functions must be implemented by a scheduler implmentation.
 */

namespace ioa {

  class scheduler
  {
  public:
    template <class I>
    static automaton_handle<I> get_current_aid (const I* ptr);

    template <class C>
    static void create (const C* ptr,
			std::auto_ptr<generator_interface> generator,
			void* aux);

    template <class C, class OI, class OM, class II, class IM, class D>
    static void bind (const C* ptr,
		      const action<OI, OM>& output_action,
		      const action<II, IM>& input_action,
		      D& d);

    template <class C, class D>
    static void unbind (const C* ptr,
			const int bid,
			D& d);

    template <class C, class I, class D>
    static void destroy (const C* ptr,
			 const automaton_handle<I>& automaton,
			 D& d);
    
    template <class I, class M>
    static void schedule (const I* ptr,
			  M I::*member_ptr);

    template <class I, class M>
    static void schedule (const I* ptr,
			  M I::*member_ptr,
			  time offset);
    
    static void run (std::auto_ptr<generator_interface> generator);

    static void clear ();
  };

}

#endif
