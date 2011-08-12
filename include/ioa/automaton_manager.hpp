#ifndef __automaton_manager_hpp__
#define __automaton_manager_hpp__

#include <ioa/automaton_manager_interface.hpp>

namespace ioa {
  
  template <class I>
  class automaton_manager :
    public system_automaton_manager_interface,
    public automaton_manager_interface,
    public automaton_handle_interface<I>
  {
  public:
    typedef I instance;

  private:
    enum state_t {
      START,
      INSTANCE_EXISTS,
      CREATED,
      DESTROYED
    };
    automaton* m_automaton;
    std::auto_ptr<typed_generator_interface<I> > m_generator;
    state_t m_state;
    automaton_handle<I> m_handle;

  public:
    automaton_manager (automaton* automaton,
		       std::auto_ptr<typed_generator_interface<I> > generator) :
      m_automaton (automaton),
      m_generator (generator),
      m_state (START)
    {
      m_automaton->create (this);
    }

  private:
    // Must use new/destroy.
    ~automaton_manager () { }

  public:

    void destroy () {
      m_automaton->destroy (this);
    }

    std::auto_ptr<generator_interface> get_generator () {
      return std::auto_ptr<generator_interface> (m_generator);
    }

    void created (const created_t result,
		  const aid_t aid) {
      switch (result) {
      case CREATE_KEY_EXISTS_RESULT:
	// System is not correct.
	assert (false);
	break;
      case INSTANCE_EXISTS_RESULT:
	m_state = INSTANCE_EXISTS;
	m_handle = -1;
	this->notify_observers ();
	delete this;
	break;
      case AUTOMATON_CREATED_RESULT:
	m_state = CREATED;
	m_handle = aid;
	this->notify_observers ();
	break;
      }
    }

    void destroyed (const destroyed_t result) {
      switch (result) {
      case CREATE_KEY_DNE_RESULT:
	// System is not correct.
	assert (false);
	break;	
      case AUTOMATON_DESTROYED_RESULT:
	m_state = DESTROYED;
	m_handle = -1;
	this->notify_observers ();
	delete this;
      }
    }
  
    state_t get_state () const {
      return m_state;
    }

    automaton_handle<I> get_handle () const {
      return m_handle;
    }

  };

  template <typename I>
  automaton_manager<I>* make_automaton_manager (automaton* automaton,
						std::auto_ptr<typed_generator_interface<I> > generator) {
    return new automaton_manager<I> (automaton, generator);
  }

}

#endif
