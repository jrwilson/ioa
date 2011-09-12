#ifndef __action_runnable_hpp__
#define __action_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/action_executor.hpp>

namespace ioa {

  template <class I, class M>
  class action_runnable :
    public action_runnable_interface
  {
  private:
    //action_executor<I, M> m_exec;
    
  public:
    action_runnable (const automaton_handle<I>& handle,
		     M I::*member_ptr) // :
      //m_exec (handle, member_ptr)
    { }

    action_runnable (const automaton_handle<I>& handle,
		     M I::*member_ptr,
		     const typename M::parameter_type& parameter) // :
      //m_exec (handle, member_ptr, parameter)
    { }

    void operator() () {
      assert (false);
    }
    
    const action_executor_interface& get_action () const {
      //return m_exec;
      assert (false);
      return *std::auto_ptr<action_executor_interface> ();
    }
  };
  
  template <class I, class M>
  action_runnable<I, M>* make_action_runnable (const automaton_handle<I>& handle,
					       M I::*member_ptr) {
    return new action_runnable<I, M> (handle, member_ptr);
  }

  template <class I, class M>
  action_runnable<I, M>* make_action_runnable (const automaton_handle<I>& handle,
					       M I::*member_ptr,
					       const typename M::parameter_type& parameter) {
    return new action_runnable<I, M> (handle, member_ptr, parameter);
  }

  template <class I, class M>
  action_runnable<I, M>* make_action_runnable (const automaton_handle<I>& handle,
					       M I::*member_ptr,
					       const typename M::value_type& value,
					       typename M::action_category category) {
    return new action_runnable<I, M> (handle, member_ptr, value, category);
  }
  
}

#endif
