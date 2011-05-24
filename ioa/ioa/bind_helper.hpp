#ifndef __bind_helper_hpp__
#define __bind_helper_hpp__

#include "observer.hpp"

namespace ioa {

  template <class T, class OH, class OM, class IH, class IM>
  class bind_helper
  {
  private:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    struct output_observer :
      public observer
    {
      bind_helper& m_bind_helper;
      OH* m_output;

      output_observer (bind_helper& bind_helper,
		       OH* output) :
	m_bind_helper (bind_helper),
	m_output (output)
      {
	BOOST_ASSERT (m_output != 0);
	m_output->add_observer (this);
      }

      ~output_observer () {
	if (m_output != 0) {
	  m_output->remove_observer (this);
	}
      }

      void observe () {
	m_bind_helper.set_output_handle (m_output->get_handle ());
      }

      void stop_observing () {
	m_output = 0;
      }

    };

    struct input_observer :
      public observer
    {
      bind_helper& m_bind_helper;
      IH* m_input;

      input_observer (bind_helper& bind_helper,
		      IH* input) :
	m_bind_helper (bind_helper),
	m_input (input)
      {
	BOOST_ASSERT (m_input != 0);
	m_input->add_observer (this);
      }

      ~input_observer () {
	if (m_input != 0) {
	  m_input->remove_observer (this);
	}
      }

      void observe () {
	m_bind_helper.set_input_handle (m_input->get_handle ());
      }

      void stop_observing () {
	m_input = 0;
      }

    };

    typedef enum {
      START,
      HAVE_OUTPUT,
      HAVE_INPUT,
      BIND_SENT,
      BIND_RECV1,
      BIND_RECV2,
      UNBIND_SENT
    } state_type;

    state_type m_state;
    const T* m_this;
    output_observer m_output_observer;
    automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;

    input_observer m_input_observer;
    IM II::*m_input_member_ptr;
    automaton_handle<II> m_input_handle;

    void set_output_handle (const automaton_handle<OI>& output_handle) {
      switch (m_state) {
      case START:
	m_output_handle = output_handle;
	m_state = HAVE_OUTPUT;
	break;
      case HAVE_OUTPUT:
	BOOST_ASSERT (false);
	break;
      case HAVE_INPUT:
	m_output_handle = output_handle;
	ioa::scheduler.bind (m_this, m_output_handle, m_output_member_ptr, m_input_handle, m_input_member_ptr, *this);
	m_state = BIND_SENT;
	break;
      case BIND_SENT:
	BOOST_ASSERT (false);
	break;
      case BIND_RECV1:
	BOOST_ASSERT (false);
	break;
      case BIND_RECV2:
	BOOST_ASSERT (false);
	break;
      case UNBIND_SENT:
	BOOST_ASSERT (false);
	break;
      }
    }

    void set_input_handle (const automaton_handle<II>& input_handle) {
      switch (m_state) {
      case START:
	m_input_handle = input_handle;
	m_state = HAVE_INPUT;
	break;
      case HAVE_OUTPUT:
	m_input_handle = input_handle;
	ioa::scheduler.bind (m_this, m_output_handle, m_output_member_ptr, m_input_handle, m_input_member_ptr, *this);
	m_state = BIND_SENT;
	break;
      case HAVE_INPUT:
	BOOST_ASSERT (false);
	break;
      case BIND_SENT:
	BOOST_ASSERT (false);
	break;
      case BIND_RECV1:
	BOOST_ASSERT (false);
	break;
      case BIND_RECV2:
	BOOST_ASSERT (false);
	break;
      case UNBIND_SENT:
	BOOST_ASSERT (false);
	break;
      }
    }

  public:
    bind_helper (const T* t,
		 OH* output_helper,
		 OM OI::*output_member_ptr,
		 IH* input_helper,
		 IM II::*input_member_ptr) :
      m_state (START),
      m_this (t),
      m_output_observer (*this, output_helper),
      m_output_member_ptr (output_member_ptr),
      m_input_observer (*this, input_helper),
      m_input_member_ptr (input_member_ptr)
    { }

    void unbind () {
      switch (m_state) {
      case START:
	delete this;
	break;
      case HAVE_OUTPUT:
	delete this;
	break;
      case HAVE_INPUT:
	delete this;
	break;
      case BIND_SENT:
	m_state = BIND_RECV2;
	break;
      case BIND_RECV1:
	ioa::scheduler.unbind (m_this, m_output_handle, m_output_member_ptr, m_input_handle, m_input_member_ptr, *this);
	m_state = UNBIND_SENT;
	break;
      case BIND_RECV2:
	BOOST_ASSERT (false);
	break;
      case UNBIND_SENT:
	BOOST_ASSERT (false);
	break;
      }
    }

    void bind_output_automaton_dne () {
      BOOST_ASSERT (false);
    }

    void bind_input_automaton_dne () {
      BOOST_ASSERT (false);
    }

    void bind_output_parameter_dne () {
      BOOST_ASSERT (false);
    }

    void bind_input_parameter_dne () {
      BOOST_ASSERT (false);
    }

    void binding_exists () {
      BOOST_ASSERT (false);
    }
    
    void input_action_unavailable () {
      BOOST_ASSERT (false);
    }

    void output_action_unavailable () {
      BOOST_ASSERT (false);
    }

    void bound () {
      switch (m_state) {
      case START:
	BOOST_ASSERT (false);
	break;
      case HAVE_OUTPUT:
	BOOST_ASSERT (false);
	break;
      case HAVE_INPUT:
	BOOST_ASSERT (false);
	break;
      case BIND_SENT:
	m_state = BIND_RECV1;
	break;
      case BIND_RECV1:
	BOOST_ASSERT (false);
	break;
      case BIND_RECV2:
	ioa::scheduler.unbind (m_this, m_output_handle, m_output_member_ptr, m_input_handle, m_input_member_ptr, *this);
	m_state = UNBIND_SENT;
	break;
      case UNBIND_SENT:
	BOOST_ASSERT (false);
	break;
      }
    }

    void unbound () {
      delete this;
    }

    void unbind_output_automaton_dne () {
      BOOST_ASSERT (false);
    }

    void unbind_input_automaton_dne () {
      BOOST_ASSERT (false);
    }

    void unbind_output_parameter_dne () {
      BOOST_ASSERT (false);
    }

    void unbind_input_parameter_dne () {
      BOOST_ASSERT (false);
    }

    void binding_dne () {
      BOOST_ASSERT (false);
    }

  };

}

#endif
