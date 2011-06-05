#ifndef __create_runnable_hpp__
#define __create_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/system.hpp>

namespace ioa {
  
  template <class C, class I, class D>
  class create_runnable :
    public runnable_interface
  {
  private:
    const automaton_handle<C> m_automaton;
    std::auto_ptr<generator_interface> m_generator;
    D& m_d;
  public:
    create_runnable (const automaton_handle<C>& automaton,
		     std::auto_ptr<generator_interface> generator,
		     D& d) :
      m_automaton (automaton),
      m_generator (generator),
      m_d (d)
    { }
    
    void operator() () {
      system::create (m_automaton, m_generator, m_d);
    }
  };
  
  template <class C, class I, class D>
  create_runnable<C, I, D>* make_create_runnable (const automaton_handle<C>& automaton,
						  std::auto_ptr<generator_interface> generator,
						  D& d) {
    return new create_runnable<C, I, D> (automaton, generator, d);
  }
  
}

#endif
