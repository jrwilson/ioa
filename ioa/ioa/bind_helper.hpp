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
      START_B,
      HAVE_OUTPUT,
      HAVE_OUTPUT_B,
      HAVE_INPUT,
      HAVE_INPUT_B,
      HAVE_BOTH,
      BIND_SENT,
      BIND_RECV1,
      BIND_RECV2,
      UNBIND_SENT,
      STOP,
      ERROR
    } state_type;

    state_type m_state;
    const T* m_this;

    output_observer m_output_observer;
    automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;

    input_observer m_input_observer;
    IM II::*m_input_member_ptr;
    automaton_handle<II> m_input_handle;

    bid_t m_bid;

    void set_output_handle (const automaton_handle<OI>& output_handle) {
      switch (m_state) {
      case START:
	m_output_handle = output_handle;
	m_state = HAVE_OUTPUT;
	break;
      case START_B:
	m_output_handle = output_handle;
	m_state = HAVE_OUTPUT_B;
	break;
      case HAVE_INPUT:
	m_output_handle = output_handle;
	m_state = HAVE_BOTH;
	break;
      case HAVE_INPUT_B:
	m_output_handle = output_handle;
	ioa::scheduler.bind (m_this,
			     ioa::make_action (m_output_handle, m_output_member_ptr),
			     ioa::make_action (m_input_handle, m_input_member_ptr),
			     *this);
	m_state = BIND_SENT;
	break;
      default:
	break;
      }
    }

    void set_input_handle (const automaton_handle<II>& input_handle) {
      switch (m_state) {
      case START:
	m_input_handle = input_handle;
	m_state = HAVE_INPUT;
	break;
      case START_B:
	m_input_handle = input_handle;
	m_state = HAVE_INPUT_B;
	break;
      case HAVE_OUTPUT:
	m_input_handle = input_handle;
	m_state = HAVE_BOTH;
	break;
      case HAVE_OUTPUT_B:
	m_input_handle = input_handle;
	ioa::scheduler.bind (m_this,
			     ioa::make_action (m_output_handle, m_output_member_ptr),
			     ioa::make_action (m_input_handle, m_input_member_ptr),
			     *this);
	m_state = BIND_SENT;
	break;
      default:
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
      m_input_member_ptr (input_member_ptr),
      m_bid (-1)
    { }

    void bind () {
      switch (m_state) {
      case START:
	m_state = START_B;
	break;
      case HAVE_OUTPUT:
	m_state = HAVE_OUTPUT_B;
	break;
      case HAVE_INPUT:
	m_state = HAVE_INPUT_B;
	break;
      case HAVE_BOTH:
	ioa::scheduler.bind (m_this,
			     ioa::make_action (m_output_handle, m_output_member_ptr),
			     ioa::make_action (m_input_handle, m_input_member_ptr),
			     *this);
	m_state = BIND_SENT;
	break;
      default:
	break;
      }
    }

    void unbind () {
      switch (m_state) {
      case START:
      case START_B:
      case HAVE_OUTPUT:
      case HAVE_OUTPUT_B:
      case HAVE_INPUT:
      case HAVE_INPUT_B:
	m_state = STOP;
	break;
      case BIND_SENT:
	m_state = BIND_RECV2;
	break;
      case BIND_RECV1:
	ioa::scheduler.unbind (m_this, m_bid, *this);
	m_state = UNBIND_SENT;
	break;
      default:
	break;
      }
    }

    void output_automaton_dne () {
      m_state = ERROR;
    }

    void input_automaton_dne () {
      m_state = ERROR;
    }

    void binding_exists () {
      m_state = ERROR;
    }
    
    void input_action_unavailable () {
      m_state = ERROR;
    }

    void output_action_unavailable () {
      m_state = ERROR;
    }

    void bound (const ioa::bid_t bid) {
      switch (m_state) {
      case BIND_SENT:
	m_bid = bid;
	m_state = BIND_RECV1;
	break;
      case BIND_RECV2:
	ioa::scheduler.unbind (m_this, bid, *this);
	m_state = UNBIND_SENT;
	break;
      default:
	break;
      }
    }

    void unbound () {
      m_bid = -1;
      m_state = STOP;
    }

    void binding_dne () {
      m_state = ERROR;
    }

  };

}

#endif
