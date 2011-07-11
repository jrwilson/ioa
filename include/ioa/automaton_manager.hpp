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
    automaton* m_automaton;
    const_shared_ptr<typed_generator_interface<I> > m_generator;
    automaton_handle<I> m_handle;

  public:
    automaton_manager (automaton* automaton,
		       const_shared_ptr<typed_generator_interface<I> > generator) :
      m_automaton (automaton),
      m_generator (generator)
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

    const_shared_ptr<generator_interface> get_generator () const {
      return m_generator;
    }

    void instance_exists () {
      delete this;
    }

    void automaton_created (const aid_t aid) {
      m_handle = aid;
      this->notify_observers ();
    }

    void automaton_destroyed () {
      delete this;
    }
  
    automaton_handle<I> get_handle () const {
      return m_handle;
    }

  };

}

#endif
