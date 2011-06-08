#ifndef __bind_helper_hpp__
#define __bind_helper_hpp__

#include <ioa/automaton_interface.hpp>
#include <ioa/observer.hpp>

#include <cassert>

namespace ioa {

  template <class OI, class OM, class OPS, class II, class IM, class IPS> class bind_helper_impl;

  template <class OI, class OM, class II, class IM>
  class bind_helper_core :
    public system_bind_helper_interface,
    public observable
  {
  private:
    struct output_observer :
      public observer
    {
      bind_helper_core* m_bind_helper;
      automaton_helper_interface<OI>* m_output;

      output_observer (bind_helper_core* bind_helper,
  		       automaton_helper_interface<OI>* output) :
  	m_bind_helper (bind_helper),
  	m_output (output)
      {
  	assert (m_output != 0);

  	automaton_handle<OI> h = m_output->get_handle ();
  	if (h != -1) {
  	  m_bind_helper->set_output_handle (h);
  	}
	else {
	  add_observable (m_output);
	}
      }

      void observe (observable* o) {
	assert (o == m_output);
  	m_bind_helper->set_output_handle (m_output->get_handle ());
	remove_observable (m_output);
      }

      void stop_observing (observable* o) {
	// The output automaton is dead.
	// We have no way of making progress.
  	m_bind_helper->unbind ();
      }

    };

    struct input_observer :
      public observer
    {
      bind_helper_core* m_bind_helper;
      automaton_helper_interface<II>* m_input;

      input_observer (bind_helper_core* bind_helper,
  		      automaton_helper_interface<II>* input) :
  	m_bind_helper (bind_helper),
  	m_input (input)
      {
  	assert (m_input != 0);

  	automaton_handle<II> h = m_input->get_handle ();
  	if (h != -1) {
  	  m_bind_helper->set_input_handle (h);
  	}
	else {
	  add_observable (m_input);
	}
      }

      void observe (observable* o) {
	assert (o == m_input);
  	m_bind_helper->set_input_handle (m_input->get_handle ());
	remove_observable (m_input);
      }

      void stop_observing () {
	// The input automaton is dead.
	// We have no way of making progress.
  	m_bind_helper->unbind ();
      }

    };

  protected:
    automaton_interface* m_automaton;

  protected:
    automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;

  private:
    output_observer m_output_observer;
    input_observer m_input_observer;

  private:
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
    bind_helper_core (automaton_interface* automaton,
  		      automaton_helper_interface<OI>* output,
  		      OM OI::*output_member_ptr,
  		      automaton_helper_interface<II>* input,
  		      IM II::*input_member_ptr) :
      m_automaton (automaton),
      m_output_handle (),
      m_output_member_ptr (output_member_ptr),
      m_input_handle (),
      m_input_member_ptr (input_member_ptr),
      // This need to be initialized after the handles because they might set the handle.
      m_output_observer (this, output),
      m_input_observer (this, input)
    { }

  protected:

    virtual ~bind_helper_core () { }

  public:

    void unbind () {
      if (m_output_handle != -1 && m_input_handle != -1) {
	m_automaton->unbind (this);
      }
      else {
	delete this;
      }
    }

    void output_automaton_dne () {
      delete this;
    }

    void input_automaton_dne () {
      delete this;
    }

    void binding_exists () {
      delete this;
    }
    
    void input_action_unavailable () {
      delete this;
    }

    void output_action_unavailable () {
      delete this;
    }

    void bound () {
      // TODO:  Set a variable or something so someone can inspect.
      notify_observers ();
    }

    void unbound () {
      delete this;
    }

  };

  // No parameters.
  template <class OI, class OM, class II, class IM>
  class bind_helper_impl<OI, OM, unparameterized, II, IM, unparameterized> :
    public bind_helper_core<OI, OM, II, IM>
  {
  private:

    ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
      return ioa::make_bind_executor (ioa::make_action (this->m_output_handle, this->m_output_member_ptr),
				      ioa::make_action (this->m_input_handle, this->m_input_member_ptr));
    }
    
  public:
    bind_helper_impl (automaton_interface* automaton,
  		      automaton_helper_interface<OI>* output,
  		      OM OI::*output_member_ptr,
  		      automaton_helper_interface<II>* input,
  		      IM II::*input_member_ptr) :
      bind_helper_core<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr)
    { }

  };

  // // Parameterized output.
  // template <class T, class OH, class OM, class IH, class IM>
  // class bind_helper_impl<T, OH, OM, parameterized, IH, IM, unparameterized> :
  //   public bind_helper_core<T, OH, OM, IH, IM>
  // {
  // private:
  //   typedef typename OH::instance OI;
  //   typedef typename IH::instance II;
  //   typedef typename OM::parameter_type OP;

  //   OP m_output_parameter;

  //   void bind_dispatch () {
  //     scheduler::bind (this->m_this,
  // 		       make_action (this->m_output_handle, this->m_output_member_ptr, m_output_parameter),
  // 		       make_action (this->m_input_handle, this->m_input_member_ptr),
  // 		       *this);
  //   }

  // public:
  //   bind_helper_impl (const T* t,
  // 		      OH* output_helper,
  // 		      OM OI::*output_member_ptr,
  // 		      const OP& output_parameter,
  // 		      IH* input_helper,
  // 		      IM II::*input_member_ptr) :
  //     bind_helper_core<T, OH, OM, IH, IM> (t, output_helper, output_member_ptr, input_helper, input_member_ptr),
  //     m_output_parameter (output_parameter)
  //   { }

  // };

  // // Parameterized input.
  // template <class T, class OH, class OM, class IH, class IM>
  // class bind_helper_impl<T, OH, OM, unparameterized, IH, IM, parameterized> :
  //   public bind_helper_core<T, OH, OM, IH, IM>
  // {
  // private:
  //   typedef typename OH::instance OI;
  //   typedef typename IH::instance II;
  //   typedef typename IM::parameter_type IP;

  //   IP m_input_parameter;

  //   void bind_dispatch () {
  //     scheduler::bind (this->m_this,
  // 		       make_action (this->m_output_handle, this->m_output_member_ptr),
  // 		       make_action (this->m_input_handle, this->m_input_member_ptr, m_input_parameter),
  // 		       *this);
  //   }

  // public:
  //   bind_helper_impl (const T* t,
  // 		      OH* output_helper,
  // 		      OM OI::*output_member_ptr,
  // 		      IH* input_helper,
  // 		      IM II::*input_member_ptr,
  // 		      const IP& input_parameter) :
  //     bind_helper_core<T, OH, OM, IH, IM> (t, output_helper, output_member_ptr, input_helper, input_member_ptr),
  //     m_input_parameter (input_parameter)
  //   { }

  // };

  // // Parameterized output and input.
  // template <class T, class OH, class OM, class IH, class IM>
  // class bind_helper_impl<T, OH, OM, parameterized, IH, IM, parameterized> :
  //   public bind_helper_core<T, OH, OM, IH, IM>
  // {
  // private:
  //   typedef typename OH::instance OI;
  //   typedef typename IH::instance II;
  //   typedef typename OM::parameter_type OP;
  //   typedef typename IM::parameter_type IP;

  //   OP m_output_parameter;
  //   IP m_input_parameter;

  //   void bind_dispatch () {
  //     scheduler::bind (this->m_this,
  // 		       make_action (this->m_output_handle, this->m_output_member_ptr, m_output_parameter),
  // 		       make_action (this->m_input_handle, this->m_input_member_ptr, m_input_parameter),
  // 		       *this);
  //   }

  // public:
  //   bind_helper_impl (const T* t,
  // 		      OH* output_helper,
  // 		      OM OI::*output_member_ptr,
  // 		      const OP& output_parameter,
  // 		      IH* input_helper,
  // 		      IM II::*input_member_ptr,
  // 		      const IP& input_parameter) :
  //     bind_helper_core<T, OH, OM, IH, IM> (t, output_helper, output_member_ptr, input_helper, input_member_ptr),
  //     m_output_parameter (output_parameter),
  //     m_input_parameter (input_parameter)
  //   { }

  // };

  template <class OI, class OM, class II, class IM>
  class bind_helper :
    public bind_helper_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status>
  {
  private:
    typedef typename OM::parameter_type OP;
    typedef typename IM::parameter_type IP;
    
  public:
    bind_helper (automaton_interface* automaton,
  		 automaton_helper_interface<OI>* output,
  		 OM OI::*output_member_ptr,
  		 automaton_helper_interface<II>* input,
  		 IM II::*input_member_ptr) :
      bind_helper_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status> (automaton, output, output_member_ptr, input, input_member_ptr)
    { }

    bind_helper (automaton_interface* automaton,
  		 automaton_helper_interface<OI>* output,
  		 OM OI::*output_member_ptr,
  		 const OP& output_parameter,
  		 automaton_helper_interface<II>* input,
  		 IM II::*input_member_ptr) :
      bind_helper_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status> (automaton, output, output_member_ptr, output_parameter, input, input_member_ptr)
    { }

    bind_helper (automaton_interface* automaton,
  		 automaton_helper_interface<OI>* output,
  		 OM OI::*output_member_ptr,
  		 automaton_helper_interface<II>* input,
  		 IM II::*input_member_ptr,
  		 const IP& input_parameter) :
      bind_helper_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status> (automaton, output, output_member_ptr, input, input_member_ptr, input_parameter)
    { }

    bind_helper (automaton_interface* automaton,
  		 automaton_helper_interface<OI>* output,
  		 OM OI::*output_member_ptr,
  		 const OP& output_parameter,
  		 automaton_helper_interface<II>* input,
  		 IM II::*input_member_ptr,
  		 const IP& input_parameter) :
      bind_helper_impl<OI, OM, typename OM::parameter_status, II, IM, typename IM::parameter_status> (automaton, output, output_member_ptr, output_parameter, input, input_member_ptr, input_parameter)
    { }

  };

  template <class OI, class OM, class II, class IM>
  bind_helper<OI, OM, II, IM>* make_bind_helper (automaton_interface* automaton,
			 automaton_helper_interface<OI>* output,
			 OM OI::*output_member_ptr,
			 automaton_helper_interface<II>* input,
			 IM II::*input_member_ptr) {
    return new bind_helper<OI, OM, II, IM> (automaton, output, output_member_ptr, input, input_member_ptr);
  }

}

#endif
