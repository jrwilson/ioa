#ifndef __bind_helper_hpp__
#define __bind_helper_hpp__

#include "observer.hpp"
#include "automaton_handle.hpp"
#include "binding_handle.hpp"
#include "scheduler.hpp"

namespace ioa {

  template <class T, class OH, class OM, class OPS, class IH, class IM, class IPS> class bind_helper_impl;

  template <class T, class OH, class OM, class IH, class IM>
  class bind_helper_core :
    public observable
  {
  private:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

    struct output_observer :
      public observer
    {
      bind_helper_core* m_bind_helper;
      OH* m_output;

      output_observer (bind_helper_core* bind_helper,
		       OH* output) :
	m_bind_helper (bind_helper),
	m_output (output)
      {
	BOOST_ASSERT (m_output != 0);
	m_output->add_observer (this);

	automaton_handle<OI> h = m_output->get_handle ();
	if (h.aid () != -1) {
	  m_bind_helper->set_output_handle (h);
	}
      }

      ~output_observer () {
	if (m_output != 0) {
	  m_output->remove_observer (this);
	}
      }

      void observe () {
	m_bind_helper->set_output_handle (m_output->get_handle ());
      }

      void stop_observing () {
	m_output = 0;
	delete m_bind_helper;
      }

    };

    struct input_observer :
      public observer
    {
      bind_helper_core* m_bind_helper;
      IH* m_input;

      input_observer (bind_helper_core* bind_helper,
		      IH* input) :
	m_bind_helper (bind_helper),
	m_input (input)
      {
	BOOST_ASSERT (m_input != 0);
	m_input->add_observer (this);

	automaton_handle<II> h = m_input->get_handle ();
	if (h.aid () != -1) {
	  m_bind_helper->set_input_handle (h);
	}
      }

      ~input_observer () {
	if (m_input != 0) {
	  m_input->remove_observer (this);
	}
      }

      void observe () {
	m_bind_helper->set_input_handle (m_input->get_handle ());
      }

      void stop_observing () {
	m_input = 0;
	delete m_bind_helper;
      }

    };

    typedef enum {
      START,
      HAVE_OUTPUT,
      HAVE_INPUT,
      BIND_SENT,
      BIND_RECV1,
      BIND_RECV2,
      UNBIND_SENT,
    } state_type;

    state_type m_state;

  protected:
    const T* m_this;

  protected:
    automaton_handle<OI> m_output_handle;
    OM OI::*m_output_member_ptr;
    automaton_handle<II> m_input_handle;
    IM II::*m_input_member_ptr;

  private:
    output_observer m_output_observer;
    input_observer m_input_observer;

  private:
    bid_t m_bid;

    void set_output_handle (const automaton_handle<OI>& output_handle) {
      switch (m_state) {
      case START:
	m_output_handle = output_handle;
	m_state = HAVE_OUTPUT;
	break;
      case HAVE_INPUT:
	m_output_handle = output_handle;
	bind_dispatch ();
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
      case HAVE_OUTPUT:
	m_input_handle = input_handle;
	bind_dispatch ();
	m_state = BIND_SENT;
	break;
      default:
	break;
      }
    }

    virtual void bind_dispatch () = 0;

  public:
    bind_helper_core (const T* t,
		      OH* output_helper,
		      OM OI::*output_member_ptr,
		      IH* input_helper,
		      IM II::*input_member_ptr) :
      m_state (START),
      m_this (t),
      m_output_handle (),
      m_output_member_ptr (output_member_ptr),
      m_input_handle (),
      m_input_member_ptr (input_member_ptr),
      // This need to be initialized after the handles because they might set the handle.
      m_output_observer (this, output_helper),
      m_input_observer (this, input_helper),
      m_bid (-1)
    { }

  protected:

    virtual ~bind_helper_core () { }

  public:

    void unbind () {
      switch (m_state) {
      case START:
      case HAVE_OUTPUT:
      case HAVE_INPUT:
	delete this;
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

    void bound (const ioa::bid_t bid) {
      switch (m_state) {
      case BIND_SENT:
	m_bid = bid;
	m_state = BIND_RECV1;
	// Notify the observers.
	notify_observers ();
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
      delete this;
    }

    void binding_dne () {
      delete this;
    }

    bid_t get_handle () const {
      return m_bid;
    }
  };

  // No parameters.
  template <class T, class OH, class OM, class IH, class IM>
  class bind_helper_impl<T, OH, OM, unparameterized, IH, IM, unparameterized> :
    public bind_helper_core<T, OH, OM, IH, IM>
  {
  private:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;
    

    void bind_dispatch () {
      scheduler.bind (this->m_this,
		      ioa::make_action (this->m_output_handle, this->m_output_member_ptr),
		      ioa::make_action (this->m_input_handle, this->m_input_member_ptr),
		      *this);
    }

  public:
    bind_helper_impl (const T* t,
		      OH* output_helper,
		      OM OI::*output_member_ptr,
		      IH* input_helper,
		      IM II::*input_member_ptr) :
      bind_helper_core<T, OH, OM, IH, IM> (t, output_helper, output_member_ptr, input_helper, input_member_ptr)
    { }

  };

  // Parameterized output.
  template <class T, class OH, class OM, class IH, class IM>
  class bind_helper_impl<T, OH, OM, parameterized, IH, IM, unparameterized> :
    public bind_helper_core<T, OH, OM, IH, IM>
  {
  private:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;
    typedef typename OM::parameter_type OP;

    OP m_output_parameter;

    void bind_dispatch () {
      scheduler.bind (this->m_this,
		      ioa::make_action (this->m_output_handle, this->m_output_member_ptr, m_output_parameter),
		      ioa::make_action (this->m_input_handle, this->m_input_member_ptr),
		      *this);
    }

  public:
    bind_helper_impl (const T* t,
		      OH* output_helper,
		      OM OI::*output_member_ptr,
		      const OP& output_parameter,
		      IH* input_helper,
		      IM II::*input_member_ptr) :
      bind_helper_core<T, OH, OM, IH, IM> (t, output_helper, output_member_ptr, input_helper, input_member_ptr),
      m_output_parameter (output_parameter)
    { }

  };

  // Parameterized input.
  template <class T, class OH, class OM, class IH, class IM>
  class bind_helper_impl<T, OH, OM, unparameterized, IH, IM, parameterized> :
    public bind_helper_core<T, OH, OM, IH, IM>
  {
  private:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;
    typedef typename IM::parameter_type IP;

    IP m_input_parameter;

    void bind_dispatch () {
      scheduler.bind (this->m_this,
		      ioa::make_action (this->m_output_handle, this->m_output_member_ptr),
		      ioa::make_action (this->m_input_handle, this->m_input_member_ptr, m_input_parameter),
		      *this);
    }

  public:
    bind_helper_impl (const T* t,
		      OH* output_helper,
		      OM OI::*output_member_ptr,
		      IH* input_helper,
		      IM II::*input_member_ptr,
		      const IP& input_parameter) :
      bind_helper_core<T, OH, OM, IH, IM> (t, output_helper, output_member_ptr, input_helper, input_member_ptr),
      m_input_parameter (input_parameter)
    { }

  };

  // Parameterized output and input.
  template <class T, class OH, class OM, class IH, class IM>
  class bind_helper_impl<T, OH, OM, parameterized, IH, IM, parameterized> :
    public bind_helper_core<T, OH, OM, IH, IM>
  {
  private:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;
    typedef typename OM::parameter_type OP;
    typedef typename IM::parameter_type IP;

    OP m_output_parameter;
    IP m_input_parameter;

    void bind_dispatch () {
      scheduler.bind (this->m_this,
		      ioa::make_action (this->m_output_handle, this->m_output_member_ptr, m_output_parameter),
		      ioa::make_action (this->m_input_handle, this->m_input_member_ptr, m_input_parameter),
		      *this);
    }

  public:
    bind_helper_impl (const T* t,
		      OH* output_helper,
		      OM OI::*output_member_ptr,
		      const OP& output_parameter,
		      IH* input_helper,
		      IM II::*input_member_ptr,
		      const IP& input_parameter) :
      bind_helper_core<T, OH, OM, IH, IM> (t, output_helper, output_member_ptr, input_helper, input_member_ptr),
      m_output_parameter (output_parameter),
      m_input_parameter (input_parameter)
    { }

  };

  template <class T, class OH, class OM, class IH, class IM>
  class bind_helper :
    public bind_helper_impl<T, OH, OM, typename OM::parameter_status, IH, IM, typename IM::parameter_status>
  {
  private:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;
    typedef typename OM::parameter_type OP;
    typedef typename IM::parameter_type IP;
    
  public:
    bind_helper (const T* t,
		 OH* output_helper,
		 OM OI::*output_member_ptr,
		 IH* input_helper,
		 IM II::*input_member_ptr) :
      bind_helper_impl<T, OH, OM, typename OM::parameter_status, IH, IM, typename IM::parameter_status> (t, output_helper, output_member_ptr, input_helper, input_member_ptr)
    { }

    bind_helper (const T* t,
		 OH* output_helper,
		 OM OI::*output_member_ptr,
		 const OP& output_parameter,
		 IH* input_helper,
		 IM II::*input_member_ptr) :
      bind_helper_impl<T, OH, OM, typename OM::parameter_status, IH, IM, typename IM::parameter_status> (t, output_helper, output_member_ptr, output_parameter, input_helper, input_member_ptr)
    { }

    bind_helper (const T* t,
		 OH* output_helper,
		 OM OI::*output_member_ptr,
		 IH* input_helper,
		 IM II::*input_member_ptr,
		 const IP& input_parameter) :
      bind_helper_impl<T, OH, OM, typename OM::parameter_status, IH, IM, typename IM::parameter_status> (t, output_helper, output_member_ptr, input_helper, input_member_ptr, input_parameter)
    { }

    bind_helper (const T* t,
		 OH* output_helper,
		 OM OI::*output_member_ptr,
		 const OP& output_parameter,
		 IH* input_helper,
		 IM II::*input_member_ptr,
		 const IP& input_parameter) :
      bind_helper_impl<T, OH, OM, typename OM::parameter_status, IH, IM, typename IM::parameter_status> (t, output_helper, output_member_ptr, output_parameter, input_helper, input_member_ptr, input_parameter)
    { }

  };

}

#endif
