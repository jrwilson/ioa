#ifndef __model_hpp__
#define __model_hpp__

#include "system_automaton.hpp"
#include <ioa/action.hpp>
#include <map>
#include <list>
#include "automaton_record.hpp"
#include <ioa/shared_mutex.hpp>
#include <ioa/generator_interface.hpp>
#include <ioa/executor_interface.hpp>
#include <ioa/model_interface.hpp>

// TODO:  Cleanup redundancy.

namespace ioa {

  // TODO:  Memory allocation.

  class model :
    public model_interface
  {
  private:    
    shared_mutex m_mutex;
    system_automaton m_system_automaton;
    sequential_set<aid_t> m_aids;
    std::set<automaton_base*> m_instances;
    std::map<aid_t, automaton_record*> m_records;
    std::list<output_executor_interface*> m_bindings;

    void inner_destroy (automaton_record* automaton);
    
  public:
    ~model ();

    void add_bind_key (const aid_t,
		       void* const key);
    void remove_bind_key (const aid_t,
			  void* const key);

    void clear (void);
    aid_t create (std::auto_ptr<generator_interface> generator);
    aid_t create (const aid_t automaton,
		  std::auto_ptr<generator_interface> generator,
		  void* const key);
    int bind (const aid_t automaton,
	      std::auto_ptr<bind_executor_interface> exec,
	      void* const key);
    int unbind (const aid_t automaton,
		void* const key);
    int destroy (const aid_t target);
    int destroy (const aid_t automaton,
		 void* const key);
    int execute (output_executor_interface& exec);
    int execute (internal_executor_interface& exec);
    int execute (system_input_executor_interface& exec);
    int execute_sys_create (const aid_t automaton);
    int execute_sys_unbind (const aid_t automaton);
    int execute_sys_destroy (const aid_t automaton);
    int execute_output_bound (output_executor_interface& exec);
    int execute_input_bound (input_executor_interface& exec);
    int execute_output_unbound (output_executor_interface& exec);
    int execute_input_unbound (input_executor_interface& exec);
    
    size_t binding_count (const action_executor_interface& action) const;
    
    automaton_base* get_instance (const aid_t aid);
    void lock_automaton (const aid_t handle);
    void unlock_automaton (const aid_t handle);
  };

}

#endif
