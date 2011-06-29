#ifndef __model_hpp__
#define __model_hpp__

#include <ioa/action.hpp>
#include "sequential_set.hpp"
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
    
    class binding_equal
    {
    private:
      const action_executor_interface& m_output;
      const action_executor_interface& m_input;
      const aid_t m_binder;

    public:
      binding_equal (const action_executor_interface& output,
		     const action_executor_interface& input,
		     const aid_t binder) :
  	m_output (output),
  	m_input (input),
	m_binder (binder)
      { }
      
      bool operator() (const output_executor_interface* c) const {
  	return c->involves_binding (m_output, m_input, m_binder);
      }
    };
    
    class binding_output_equal
    {
    private:
      const action_executor_interface& m_output;
      
    public:
      binding_output_equal (const action_executor_interface& output) :
  	m_output (output)
      { }
      
      bool operator() (const output_executor_interface* c) const {
  	return c->involves_output (m_output);
      }
    };
    
    class binding_input_equal
    {
    private:
      const action_executor_interface& m_input;
      
    public:
      binding_input_equal (const action_executor_interface& input) :
  	m_input (input)
      { }
      
      bool operator() (const output_executor_interface* c) const {
  	return c->involves_input (m_input);
      }
    };

    class binding_aid_key_equal
    {
    private:
      aid_t const m_aid;
      void* const m_key;

    public:
      binding_aid_key_equal (aid_t const aid,
			     void* const key) :
	m_aid (aid),
	m_key (key)
      { }

      bool operator() (const output_executor_interface* c) const {
	return c->involves_aid_key (m_aid, m_key);
      }
    };

    system_scheduler_interface& m_system_scheduler;    
    shared_mutex m_mutex;
    sequential_set<aid_t> m_aids;
    std::set<automaton*> m_instances;
    std::map<aid_t, automaton_record*> m_records;
    std::list<output_executor_interface*> m_bindings;

    void inner_destroy (automaton_record* automaton);
    
  public:
    model (system_scheduler_interface&);
    ~model ();

    void clear (void);
    aid_t create (const_shared_ptr<generator_interface> generator);
    aid_t create (const aid_t automaton,
		  const_shared_ptr<generator_interface> generator,
		  void* const key);
    int bind (const aid_t automaton,
	      shared_ptr<bind_executor_interface> exec,
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
    int execute_sys_bind (const aid_t automaton);
    int execute_sys_unbind (const aid_t automaton);
    int execute_sys_destroy (const aid_t automaton);
    int execute_sys_self_destruct (const aid_t automaton);
    int execute_output_bound (output_executor_interface& exec);
    int execute_input_bound (input_executor_interface& exec);
    int execute_output_unbound (output_executor_interface& exec);
    int execute_input_unbound (input_executor_interface& exec);
    
    size_t binding_count (const action_executor_interface& action) const;
    
    automaton* get_instance (const aid_t aid);
    void lock_automaton (const aid_t handle);
    void unlock_automaton (const aid_t handle);
  };

}

#endif
