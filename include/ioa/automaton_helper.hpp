#ifndef __automaton_helper_hpp__
#define __automaton_helper_hpp__

#include <ioa/automaton_interface.hpp>
#include <ioa/observer.hpp>

namespace ioa {

  template <class I>
  class automaton_helper :
    public automaton_helper_interface,
    public observable
  {
  public:
    typedef I instance;

  private:
    automaton_interface& m_automaton;
    shared_ptr<generator_interface> m_generator;
    automaton_handle<I> m_handle;

  public:
    automaton_helper (automaton_interface& automaton,
		      shared_ptr<generator_interface> generator) :
      m_automaton (automaton),
      m_generator (generator)
    {
      m_automaton.create (this);
    }

  private:
    // Must use new/destroy.
    ~automaton_helper () { }

  public:

    void destroy () {
      m_automaton.destroy (this);
    }

    shared_ptr<generator_interface> get_generator () const {
      return m_generator;
    }

    void instance_exists () {
      delete this;
    }

    void automaton_created (const aid_t aid) {
      m_handle = aid;
      notify_observers ();
    }

    void automaton_destroyed () {
      m_handle = -1;
      delete this;
    }
  
    automaton_handle<I> get_handle () const {
      return m_handle;
    }

  };

}

#endif
