/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

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
    action_executor<I, M> m_exec;
    
  public:
    action_runnable (const automaton_handle<I>& handle,
		     M I::*member_ptr) :
      m_exec (handle, member_ptr)
    { }

    action_runnable (const automaton_handle<I>& handle,
		     M I::*member_ptr,
		     const typename M::parameter_type& parameter) :
      m_exec (handle, member_ptr, parameter)
    { }

    action_runnable (const automaton_handle<I>& handle,
		     M I::*member_ptr,
		     const typename M::value_type& value,
		     system_input_category sic) :
      m_exec (handle, member_ptr, value, sic)
    { }

    void operator() (model_interface& model) {
      model.execute (m_exec);
    }
    
    const action_executor_interface& get_action () const {
      return m_exec;
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
