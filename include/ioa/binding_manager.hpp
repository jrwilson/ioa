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

#ifndef __binding_manager_hpp__
#define __binding_manager_hpp__

#include <ioa/automaton.hpp>
#include <ioa/observer.hpp>

#include <cassert>

namespace ioa {

  class binding_manager_interface :
    public observable
  {
  public:
    enum state_t {
      START,
      OUTPUT_AUTOMATON_DNE,
      INPUT_AUTOMATON_DNE,
      BINDING_EXISTS,
      INPUT_ACTION_UNAVAILABLE,
      OUTPUT_ACTION_UNAVAILABLE,
      BOUND,
      UNBOUND
    };
    virtual ~binding_manager_interface () { }
    virtual void unbind () = 0;
    virtual state_t get_state () const = 0;
  };

  template <class OI, class OM, class OPS, class II, class IM, class IPS> class binding_manager_impl;

  template <class OI, class OM, class II, class IM>
  class binding_manager_core :
    public system_binding_manager_interface,
    public binding_manager_interface
  {
  private:
    struct output_observer :
      public observer
    {
      binding_manager_core* m_binding_manager;
      automaton_handle_interface<OI>* m_output;

      output_observer (binding_manager_core* binding_manager,
  		       automaton_handle_interface<OI>* output) :
  	m_binding_manager (binding_manager),
  	m_output (output)
      {
  	assert (m_output != 0);

  	automaton_handle<OI> h = m_output->get_handle ();
  	if (h != -1) {
  	  m_binding_manager->set_output_handle (h);
  	}
	else {
	  add_observable (m_output);
	}
      }

      void observe (observable* o) {
	assert (o == m_output);
  	m_binding_manager->set_output_handle (m_output->get_handle ());
	remove_observable (m_output);
      }

      void stop_observing (observable* o) {
	// The output automaton is dead.
	// We have no way of making progress.
  	m_binding_manager->unbind ();
      }

    };

    struct input_observer :
      public observer
    {
      binding_manager_core* m_binding_manager;
      automaton_handle_interface<II>* m_input;

      input_observer (binding_manager_core* binding_manager,
  		      automaton_handle_interface<II>* input) :
  	m_binding_manager (binding_manager),
  	m_input (input)
      {
  	assert (m_input != 0);

  	automaton_handle<II> h = m_input->get_handle ();
  	if (h != -1) {
  	  m_binding_manager->set_input_handle (h);
  	}
	else {
	  add_observable (m_input);
	}
      }

      void observe (observable* o) {
	assert (o == m_input);
  	m_binding_manager->set_input_handle (m_input->get_handle ());
	remove_observable (m_input);
      }

      void stop_observing () {
	// The input automaton is dead.
	// We have no way of making progress.
  	m_binding_manager->unbind ();
      }

    };

  protected:
    automaton* m_automaton;
    automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;

  private:
    output_observer m_output_observer;
    input_observer m_input_observer;

    state_t m_state;

    void set_output_handle (const automaton_handle<OI>& output_handle) {
      m_output_handle = output_handle;
      if (m_output_handle != -1 && m_input_handle != -1) {
  	m_automaton->bind (this);
      }
    }

    void set_input_handle (const automaton_handle<II>& input_handle) {
      m_input_handle = input_handle;
      if (m_output_handle != -1 && m_input_handle != -1) {
  	m_automaton->bind (this);
      }
    }

  public:
    binding_manager_core (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_handle (),
      m_output_member_ptr (output_member_ptr),
      m_input_handle (),
      m_input_member_ptr (input_member_ptr),
      // This need to be initialized after the handles because they might set the handle.
      m_output_observer (this, output),
      m_input_observer (this, input),
      m_state (START)
    { }

  protected:

    virtual ~binding_manager_core () { }

  public:

    void unbind () {
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton->unbind (this);
      }
      else {
	delete this;
      }
    }

    void bound (const bound_t result) {
      switch (result) {
      case BIND_KEY_EXISTS_RESULT:
	// System is incorrect.
	assert (false);
	break;
      case OUTPUT_AUTOMATON_DNE_RESULT:
	m_state = OUTPUT_AUTOMATON_DNE;
	notify_observers ();
	delete this;
	break;
      case INPUT_AUTOMATON_DNE_RESULT:
	m_state = INPUT_AUTOMATON_DNE;
	notify_observers ();
	delete this;
	break;
      case BINDING_EXISTS_RESULT:
	m_state = BINDING_EXISTS;
	notify_observers ();
	delete this;
	break;
      case INPUT_ACTION_UNAVAILABLE_RESULT:
	m_state = INPUT_ACTION_UNAVAILABLE;
	notify_observers ();
	delete this;
	break;
      case OUTPUT_ACTION_UNAVAILABLE_RESULT:
	m_state = OUTPUT_ACTION_UNAVAILABLE;
	notify_observers ();
	delete this;
	break;
      case BOUND:
	m_state = BOUND;
	notify_observers ();
	break;
      }
    }

    void unbound (const unbound_t result) {
      switch (result) {
      case BIND_KEY_DNE_RESULT:
	// System is incorrect.
	assert (false);
	break;
      case UNBOUND_RESULT:
	m_state = UNBOUND;
	notify_observers ();
	delete this;
      }
    }

    state_t get_state () const {
      return m_state;
    }
  };

  // no no.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, unparameterized, II, IM, unparameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:

    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr,
				 this->m_input_handle, this->m_input_member_ptr);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr)
    { }

  };

  // no yes.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, unparameterized, II, IM, parameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:
    typedef typename IM::parameter_type IP;

    IP m_input_parameter;

    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr,
				 this->m_input_handle, this->m_input_member_ptr, this->m_input_parameter);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr,
			  const IP& input_parameter) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr),
      m_input_parameter (input_parameter)
    { }

  };

  // no auto.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, unparameterized, II, IM, auto_parameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:
    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr,
				 this->m_input_handle, this->m_input_member_ptr);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr)
    { }

  };

  // yes no.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, parameterized, II, IM, unparameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:
    typedef typename OM::parameter_type OP;

    OP m_output_parameter;

    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr, this->m_output_parameter,
				 this->m_input_handle, this->m_input_member_ptr);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  const OP& output_parameter,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr),
      m_output_parameter (output_parameter)
    { }
  };

  // yes yes.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, parameterized, II, IM, parameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:
    typedef typename OM::parameter_type OP;
    typedef typename IM::parameter_type IP;

    OP m_output_parameter;
    IP m_input_parameter;

    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr, m_output_parameter,
				 this->m_input_handle, this->m_input_member_ptr, m_input_parameter);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  const OP& output_parameter,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr,
			  const IP& input_parameter) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr),
      m_output_parameter (output_parameter),
      m_input_parameter (input_parameter)
    { }

  };

  // yes auto.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, parameterized, II, IM, auto_parameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:
    typedef typename OM::parameter_type OP;

    OP m_output_parameter;

    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr, m_output_parameter,
				 this->m_input_handle, this->m_input_member_ptr);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  const OP& output_parameter,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr),
      m_output_parameter (output_parameter)
    { }

  };

  // auto no.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, auto_parameterized, II, IM, unparameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:
    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr,
				 this->m_input_handle, this->m_input_member_ptr);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr)
    { }

  };

  // auto yes.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, auto_parameterized, II, IM, parameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:
    typedef typename IM::parameter_type IP;

    IP m_input_parameter;

    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr,
				 this->m_input_handle, this->m_input_member_ptr, this->m_input_parameter);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr,
			  const IP& input_parameter) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr),
      m_input_parameter (input_parameter)
    { }

  };

  // auto auto.
  template <class OI, class OM, class II, class IM>
  class binding_manager_impl<OI, OM, auto_parameterized, II, IM, auto_parameterized> :
    public binding_manager_core<OI, OM, II, IM>
  {
  private:
    std::auto_ptr<bind_executor_interface> get_executor () const {
      return make_bind_executor (this->m_output_handle, this->m_output_member_ptr,
				 this->m_input_handle, this->m_input_member_ptr);
    }

  public:
    binding_manager_impl (automaton* automaton,
			  automaton_handle_interface<OI>* output,
			  OM OI::*output_member_ptr,
			  automaton_handle_interface<II>* input,
			  IM II::*input_member_ptr) :
      binding_manager_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr)
    { }

  };


  template <class OI, class OM, class II, class IM>
  class binding_manager :
    public binding_manager_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status>
  {
  private:
    typedef typename OM::parameter_type OP;
    typedef typename IM::parameter_type IP;

  public:
    binding_manager (automaton* automaton,
		     automaton_handle_interface<OI>* output,
		     OM OI::*output_member_ptr,
		     automaton_handle_interface<II>* input,
		     IM II::*input_member_ptr) :
      binding_manager_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status> (automaton, output, output_member_ptr, input, input_member_ptr)
    { }

    binding_manager (automaton* automaton,
		     automaton_handle_interface<OI>* output,
		     OM OI::*output_member_ptr,
		     const OP& output_parameter,
		     automaton_handle_interface<II>* input,
		     IM II::*input_member_ptr) :
      binding_manager_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status> (automaton, output, output_member_ptr, output_parameter, input, input_member_ptr)
    { }

    binding_manager (automaton* automaton,
		     automaton_handle_interface<OI>* output,
		     OM OI::*output_member_ptr,
		     automaton_handle_interface<II>* input,
		     IM II::*input_member_ptr,
		     const IP& input_parameter) :
      binding_manager_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status> (automaton, output, output_member_ptr, input, input_member_ptr, input_parameter)
    { }

    binding_manager (automaton* automaton,
		     automaton_handle_interface<OI>* output,
		     OM OI::*output_member_ptr,
		     const OP& output_parameter,
		     automaton_handle_interface<II>* input,
		     IM II::*input_member_ptr,
		     const IP& input_parameter) :
      binding_manager_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status> (automaton, output, output_member_ptr, output_parameter, input, input_member_ptr, input_parameter)
    { }

  };

  template <class OI, class OM, class II, class IM>
  binding_manager<OI, OM, II, IM>* make_binding_manager (automaton* automaton,
							 automaton_handle_interface<OI>* output,
							 OM OI::*output_member_ptr,
							 automaton_handle_interface<II>* input,
							 IM II::*input_member_ptr) {
    return new binding_manager<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr);
  }

  template <class OI, class OM, class II, class IM>
  binding_manager<OI, OM, II, IM>* make_binding_manager (automaton* automaton,
							 automaton_handle_interface<OI>* output,
							 OM OI::*output_member_ptr,
							 typename OM::parameter_type parameter,
							 automaton_handle_interface<II>* input,
							 IM II::*input_member_ptr) {
    return new binding_manager<OI, OM, II, IM> (automaton, output, output_member_ptr, parameter, input, input_member_ptr);
  }

  template <class OI, class OM, class II, class IM>
  binding_manager<OI, OM, II, IM>* make_binding_manager (automaton* automaton,
							 automaton_handle_interface<OI>* output,
							 OM OI::*output_member_ptr,
							 automaton_handle_interface<II>* input,
							 IM II::*input_member_ptr,
							 typename IM::parameter_type parameter) {
    return new binding_manager<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr, parameter);
  }

  template <class OI, class OM, class II, class IM>
  binding_manager<OI, OM, II, IM>* make_binding_manager (automaton* automaton,
							 automaton_handle_interface<OI>* output,
							 OM OI::*output_member_ptr,
							 typename OM::parameter_type output_parameter,
							 automaton_handle_interface<II>* input,
							 IM II::*input_member_ptr,
							 typename IM::parameter_type input_parameter) {
    return new binding_manager<OI, OM, II, IM> (automaton, output, output_member_ptr, output_parameter, input, input_member_ptr, input_parameter);
  }

}

#endif
