#ifndef __test_system_scheduler_hpp__
#define __test_system_scheduler_hpp__

#include <ioa/action.hpp>
#include <ioa/model.hpp>
#include <set>

struct unbound_record
{
  ioa::aid_t m_aid;
  const void* m_member_ptr;
  size_t m_pid;

  unbound_record (const ioa::action_interface& ac) :
    m_aid (ac.get_aid ()),
    m_member_ptr (ac.get_member_ptr ()),
    m_pid (ac.get_pid ())
  { }

  bool operator< (const unbound_record& other) const {
    if (m_aid != other.m_aid) {
      return m_aid < other.m_aid;
    }
    if (m_member_ptr != other.m_member_ptr) {
      return m_member_ptr < other.m_member_ptr;
    }
    return m_pid < other.m_pid;
  }
};

struct test_system_scheduler :
  public ioa::system_scheduler_interface
{
  ioa::aid_t m_create_key_exists_automaton;
  void* m_create_key_exists_key;

  ioa::aid_t m_instance_exists_automaton;
  void* m_instance_exists_key;

  ioa::aid_t m_automaton_created_automaton;
  void* m_automaton_created_key;
  ioa::aid_t m_automaton_created_child;

  ioa::aid_t m_bind_key_exists_automaton;
  void* m_bind_key_exists_key;

  ioa::aid_t m_output_automaton_dne_automaton;
  void* m_output_automaton_dne_key;

  ioa::aid_t m_input_automaton_dne_automaton;
  void* m_input_automaton_dne_key;

  ioa::aid_t m_binding_exists_automaton;
  void* m_binding_exists_key;

  ioa::aid_t m_input_action_unavailable_automaton;
  void* m_input_action_unavailable_key;

  ioa::aid_t m_output_action_unavailable_automaton;
  void* m_output_action_unavailable_key;

  ioa::aid_t m_bound_automaton;
  void* m_bound_key;

  ioa::aid_t m_output_bound_aid;
  const void* m_output_bound_member_ptr;
  size_t m_output_bound_pid;

  ioa::aid_t m_input_bound_aid;
  const void* m_input_bound_member_ptr;
  size_t m_input_bound_pid;

  ioa::aid_t m_bind_key_dne_automaton;
  void* m_bind_key_dne_key;

  std::set<std::pair<ioa::aid_t, void*> > m_unbound;

  std::set<unbound_record> m_output_unbound;

  std::set<unbound_record> m_input_unbound;

  ioa::aid_t m_create_key_dne_automaton;
  void* m_create_key_dne_key;

  std::set<std::pair<ioa::aid_t, void*> > m_automaton_destroyed;

  ioa::aid_t m_recipient_dne_automaton;
  void* m_recipient_dne_key;

  ioa::aid_t m_event_delivered_automaton;
  void* m_event_delivered_key;

  test_system_scheduler ();
  void reset ();
  
  void set_current_aid (const ioa::aid_t aid);
  void clear_current_aid ();
    
  void create (const ioa::aid_t automaton,
	       ioa::shared_ptr<ioa::generator_interface> generator,
	       void* const key);
    
  void bind (const ioa::aid_t automaton,
	     ioa::shared_ptr<ioa::bind_executor_interface> exec,
	     void* const key);
    
  void unbind (const ioa::aid_t automaton,
	       void* const key);
    
  void destroy (const ioa::aid_t automaton,
		void* const key);
    
  void create_key_exists (const ioa::aid_t automaton,
			  void* const key);
    
  void instance_exists (const ioa::aid_t automaton,
			void* const key);
    
  void automaton_created (const ioa::aid_t automaton,
			  void* const key,
			  const ioa::aid_t child);
    
  void bind_key_exists (const ioa::aid_t automaton,
			void* const key);
    
  void output_automaton_dne (const ioa::aid_t automaton,
			     void* const key);
    
  void input_automaton_dne (const ioa::aid_t automaton,
			    void* const key);
    
  void binding_exists (const ioa::aid_t automaton,
		       void* const key);
    
  void input_action_unavailable (const ioa::aid_t automaton,
				 void* const key);
    
  void output_action_unavailable (const ioa::aid_t automaton,
				  void* const key);
    
  void bound (const ioa::aid_t automaton,
	      void* const key);
    
  void output_bound (const ioa::output_executor_interface&);
    
  void input_bound (const ioa::input_executor_interface&);
    
  void bind_key_dne (const ioa::aid_t automaton,
		     void* const key);
    
  void unbound (const ioa::aid_t automaton,
		void* const key);
    
  void output_unbound (const ioa::output_executor_interface&);
    
  void input_unbound (const ioa::input_executor_interface&);
    
  void create_key_dne (const ioa::aid_t automaton,
		       void* const key);
    
  void automaton_destroyed (const ioa::aid_t automaton,
			    void* const key);
    
  void recipient_dne (const ioa::aid_t automaton,
		      void* const key);
    
  void event_delivered (const ioa::aid_t automaton,
			void* const key);

};

#endif
