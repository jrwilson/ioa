#ifndef __create_runnable_hpp__
#define __create_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/system.hpp>

namespace ioa {
  
  class create_runnable :
    public runnable_interface
  {
  private:
    const aid_t m_automaton;
    shared_ptr<generator_interface> m_generator;
    void* const m_key;
  public:
    create_runnable (const aid_t automaton,
		     shared_ptr<generator_interface> generator,
		     void* const key) :
      m_automaton (automaton),
      m_generator (generator),
      m_key (key)
    { }
    
    void operator() () {
      system::create (m_automaton, m_generator, m_key);
    }
  };
  
}

#endif
