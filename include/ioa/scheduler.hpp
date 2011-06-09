#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include <ioa/aid.hpp>
#include <ioa/time.hpp>
#include <ioa/shared_ptr.hpp>
#include <ioa/generator_interface.hpp>
#include <ioa/action.hpp>

namespace ioa {

  class scheduler
  {
  public:
<<<<<<< HEAD
    template <class I>
    static automaton_handle<I> get_current_aid (const I* ptr);

    template <class C, class I, class D>
    static void create (const C* ptr,
			std::auto_ptr<generator_interface<I> > generator,
			D& d);

    template <class C, class OI, class OM, class II, class IM, class D>
    static void bind (const C* ptr,
		      const action<OI, OM>& output_action,
		      const action<II, IM>& input_action,
		      D& d);

    template <class C, class D>
    static void unbind (const C* ptr,
			const bid_t bid,
			D& d);

    template <class C, class I, class D>
    static void destroy (const C* ptr,
			 const automaton_handle<I>& automaton,
			 D& d);

=======
    static aid_t get_current_aid ();
    
>>>>>>> new_syscall_interface
    template <class I, class M>
    static void schedule (M I::*member_ptr);
    
    template <class I, class M>
<<<<<<< HEAD
    static void schedule (const I* ptr,
			  M I::*member_ptr,
			  const typename M::parameter_type & param);

    template <class I, class M>
    static void schedule (const I* ptr,
			  M I::*member_ptr,
			  time offset);

    template <class I>
    static void run (std::auto_ptr<generator_interface<I> > generator);

    static void clear ();
=======
    static void schedule (M I::*member_ptr,
			  time offset);
    
    static void run (shared_ptr<generator_interface> generator);
>>>>>>> new_syscall_interface
  };

}

#include <ioa/simple_scheduler.hpp>

#endif
