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
    std::auto_ptr<generator_interface> m_generator;
    void* m_aux;
  public:
    create_runnable (const aid_t automaton,
		     std::auto_ptr<generator_interface> generator,
		     void* aux) :
      m_automaton (automaton),
      m_generator (generator),
      m_aux (aux)
    { }
    
    void operator() () {
      system::create (m_automaton, m_generator, m_aux);
    }
  };
  
  create_runnable* make_create_runnable (const aid_t automaton,
					 std::auto_ptr<generator_interface> generator,
					 void* aux) {
    return new create_runnable (automaton, generator, aux);
  }
  
}

#endif
