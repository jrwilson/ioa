#include <ioa/model.hpp>

#include <ioa/shared_mutex.hpp>
#include <ioa/unique_lock.hpp>
#include <ioa/shared_lock.hpp>
#include <algorithm>
#include <ioa/generator_interface.hpp>
#include <ioa/automaton_interface.hpp>

namespace ioa {

  shared_mutex model::m_mutex;
  sequential_set<aid_t> model::m_aids;
  std::set<automaton_interface*> model::m_instances;
  std::map<aid_t, automaton_record*> model::m_records;
  std::list<output_executor_interface*> model::m_bindings;

  void model::clear (void) {
   // Delete all root automata.
    while (!m_records.empty ()) {
      for (std::map<aid_t, automaton_record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (pos->second->get_parent () == 0) {
	  inner_destroy (pos->second);
	  break;
	}
      }
    }
    
    assert (m_aids.empty ());
    assert (m_instances.empty ());
    assert (m_records.empty ());
    assert (m_bindings.empty ());
  }
  
  aid_t model::create (shared_ptr<generator_interface> generator)
  {
    unique_lock lock (m_mutex);
    
    // Take an aid.
    aid_t aid = m_aids.take ();
    
    // Set the current aid.
    system_scheduler::set_current_aid (aid);
    
    // Run the generator.
    automaton_interface* instance = (*generator) ();
    assert (instance != 0);
    
    // Clear the current aid.
    system_scheduler::clear_current_aid ();
    
    if (m_instances.count (instance) != 0) {
      // Root automaton instance exists.  Bad news.
      m_aids.replace (aid);
      return -1;
    }
    
    m_instances.insert (instance);
    automaton_record* record = new automaton_record (instance, aid);
    m_records.insert (std::make_pair (aid, record));
        
    return aid;
  }
  
  aid_t model::create (const aid_t automaton,
			shared_ptr<generator_interface> generator,
			void* const key)
  {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (automaton)) {
      // Creator does not exists.
      return -1;
    }

    if (m_records[automaton]->create_key_exists (key)) {
      // Create key already in use.
      system_scheduler::create_key_exists (automaton, key);
      return -1;
    }
    
    // Take an aid.
    aid_t aid = m_aids.take ();
    
    // Set the current aid.
    system_scheduler::set_current_aid (aid);
    
    // Run the generator.
    automaton_interface* instance = (*generator) ();
    assert (instance != 0);
    
    // Clear the current aid.
    system_scheduler::clear_current_aid ();
    
    if (m_instances.count (instance) != 0) {
      // Return the aid and inform the automaton that the instance already exists.
      m_aids.replace (aid);
      system_scheduler::instance_exists (automaton, key);
      return -1;
    }
    
    m_instances.insert (instance);      
    automaton_record* record = new automaton_record (instance, aid);
    m_records.insert (std::make_pair (aid, record));
    automaton_record* parent = m_records[automaton];
    record->set_parent (key, parent);
    parent->add_child (key, record);
    
    return aid;
  }

  int model::bind (const aid_t binder,
		    shared_ptr<bind_executor_interface> bind_exec,
		    void* const key) {
    
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (binder)) {
      // Binder DNE.
      return -1;
    }
    
    if (m_records[binder]->bind_key_exists (key)) {
      // Bind key already in use.
      system_scheduler::bind_key_exists (binder, key);
      return -1;
    }
    
    output_executor_interface& output = bind_exec->get_output ();
    input_executor_interface& input = bind_exec->get_input ();
    
    if (!output.fetch_instance ()) {
      system_scheduler::output_automaton_dne (binder, key);
      return -1;
    }
    
    if (!input.fetch_instance ()) {
      system_scheduler::input_automaton_dne (binder, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									      m_bindings.end (),
									      binding_equal (output.get_action (), input.get_action (), binder));
    
    if (pos != m_bindings.end ()) {
      // Bound.
      system_scheduler::binding_exists (binder, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
										 m_bindings.end (),
										 binding_input_equal (input.get_action ()));
    
    if (in_pos != m_bindings.end ()) {
      // Input unavailable.
      system_scheduler::input_action_unavailable (binder, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
										  m_bindings.end (),
										  binding_output_equal (output.get_action ()));
    
    if (output.get_action ().get_aid () == input.get_action ().get_aid () ||
	(out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_action ().get_aid ()))) {
      // Output unavailable.
      system_scheduler::output_action_unavailable (binder, key);
      return -1;
    }
    
    output_executor_interface* c;
    
    if (out_pos != m_bindings.end ()) {
      c = *out_pos;
    }
    else {
      c = output.clone ();
      m_bindings.push_front (c);
    }
    
    // Bind.
    c->bind (input, binder, key);
    m_records[binder]->add_bind_key (key);
    
    return 0;
  }    

  int model::unbind (const aid_t binder,
		      void* const key)
  {
    if (!m_aids.contains (binder)) {
      // Binder does not exist.
      return -1;
    }
    
    std::list<output_executor_interface*>::iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_aid_key_equal (binder, key));
    
    if (pos == m_bindings.end ()) {
      // Not bound.
      system_scheduler::bind_key_dne (binder, key);
      return -1;
    }
    
    output_executor_interface* c = *pos;
    
    // Unbind.
    c->unbind (binder, key);
    m_records[binder]->remove_bind_key (key);
    
    if (c->empty ()) {
      delete c;
      m_bindings.erase (pos);
    }
    
    return 0;
  }

  int model::destroy (const aid_t target)
  {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (target)) {
      return -1;
    }
    
    inner_destroy (m_records[target]);
    return 0;
  }
  
  int model::destroy (const aid_t automaton,
		       void* const key)
  {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (automaton)) {
      // Destroyer does not exist.
      return -1;
    }

    if (!m_records[automaton]->create_key_exists (key)) {
      system_scheduler::create_key_dne (automaton, key);
      return -1;
    }

    inner_destroy (m_records[automaton]->get_child (key));
    return 0;
  }

  void model::inner_destroy (automaton_record* automaton)
  {
    for (std::pair<void*, automaton_record*> p = automaton->get_first_child ();
	 p.second != 0;
	 p = automaton->get_first_child ()) {
      inner_destroy (p.second);
    }
    
    // Update bindings.
    for (std::list<output_executor_interface*>::iterator pos = m_bindings.begin ();
	 pos != m_bindings.end ();
	 ) {
      (*pos)->unbind_automaton (automaton->get_aid ());
      if ((*pos)->empty ()) {
	delete *pos;
	pos = m_bindings.erase (pos);
      }
      else {
	++pos;
      }
    }

    // Update parent-child relationships.
    automaton_record* parent = automaton->get_parent ();
    if (parent != 0) {
      parent->remove_child (automaton->get_key ());
    }
    
    m_aids.replace (automaton->get_aid ());
    m_instances.erase (automaton->get_instance ());
    m_records.erase (automaton->get_aid ());
    delete automaton;
  }

  int model::execute (output_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance ()) {
      // Automaton does not exist.
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
										  m_bindings.end (),
										  binding_output_equal (exec.get_action ()));
    
    if (out_pos == m_bindings.end ()) {
      // Not bound.
      exec ();
    }
    else {
      (*(*out_pos)) ();
    }
    
    return 0;
  }
  
  int model::execute (internal_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance ()) {
      // Automaton does not exist.
      return -1;
    }
    
    exec ();
    return 0;
  }
  
  int model::execute (const aid_t from,
		       event_executor_interface& exec,
		       void* const key) {
    shared_lock lock (m_mutex);
    
    if (!m_aids.contains (from)) {
      // Deliverer does not exists.
      return -1;
    }
    
    if (!exec.fetch_instance ()) {
      // Recipient does not exist.
      system_scheduler::recipient_dne (from, key);
      return -1;
    }
    
    exec ();
    
    system_scheduler::event_delivered (from, key);
    return 0;
  }

  int model::execute (system_input_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance ()) {
      // Automaton does not exist.
      return -1;
    }
    
    exec ();
    return 0;
  }

  int model::execute_sys_create (const aid_t automaton) {
    shared_lock lock (m_mutex);

    if (!m_aids.contains (automaton)) {
      // Automaton does not exists.
      return -1;
    }

    action<automaton_interface, automaton_interface::sys_create_type> ac (automaton, &automaton_interface::sys_create);

    automaton_interface* instance = automaton_handle_to_instance (automaton_handle<automaton_interface> (automaton));

    lock_automaton (automaton);
    system_scheduler::set_current_aid (automaton);
    bool t = ac.precondition (*instance);
    std::pair<shared_ptr<generator_interface>, void*> key;
    if (t) {
      key = ac (*instance);
    }
    system_scheduler::clear_current_aid ();
    unlock_automaton (automaton);

    if (t) {
      system_scheduler::create (automaton, key.first, key.second);
    }

    return 0;
  }

  int model::execute_sys_bind (const aid_t automaton) {
    shared_lock lock (m_mutex);

    if (!m_aids.contains (automaton)) {
      // Automaton does not exists.
      return -1;
    }

    action<automaton_interface, automaton_interface::sys_bind_type> ac (automaton, &automaton_interface::sys_bind);

    automaton_interface* instance = automaton_handle_to_instance (automaton_handle<automaton_interface> (automaton));

    lock_automaton (automaton);
    system_scheduler::set_current_aid (automaton);
    bool t = ac.precondition (*instance);
    std::pair<shared_ptr<bind_executor_interface>, void*> key;
    if (t) {
      key = ac (*instance);
    }

    system_scheduler::clear_current_aid ();
    unlock_automaton (automaton);

    if (t) {
      system_scheduler::bind (automaton, key.first, key.second);
    }

    return 0;
  }

  int model::execute_sys_unbind (const aid_t automaton) {
    shared_lock lock (m_mutex);

    if (!m_aids.contains (automaton)) {
      // Automaton does not exists.
      return -1;
    }

    action<automaton_interface, automaton_interface::sys_unbind_type> ac (automaton, &automaton_interface::sys_unbind);

    automaton_interface* instance = automaton_handle_to_instance (automaton_handle<automaton_interface> (automaton));

    lock_automaton (automaton);
    system_scheduler::set_current_aid (automaton);
    bool t = ac.precondition (*instance);
    void* key = 0;
    if (t) {
      key = ac (*instance);
    }
    system_scheduler::clear_current_aid ();
    unlock_automaton (automaton);

    if (t) {
      system_scheduler::unbind (automaton, key);
    }

    return 0;
  }

  int model::execute_sys_destroy (const aid_t automaton) {
    shared_lock lock (m_mutex);

    if (!m_aids.contains (automaton)) {
      // Automaton does not exists.
      return -1;
    }

    action<automaton_interface, automaton_interface::sys_destroy_type> ac (automaton, &automaton_interface::sys_destroy);

    automaton_interface* instance = automaton_handle_to_instance (automaton_handle<automaton_interface> (automaton));

    lock_automaton (automaton);
    system_scheduler::set_current_aid (automaton);
    bool t = ac.precondition (*instance);
    void* key = 0;
    if (t) {
      key = ac (*instance);
    }
    system_scheduler::clear_current_aid ();
    unlock_automaton (automaton);

    if (t) {
      system_scheduler::destroy (automaton, key);
    }

    return 0;
  }

  int model::execute_output_bound (output_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance ()) {
      // Automaton does not exist.
      return -1;
    }
    
    exec.bound ();
    return 0;
  }

  int model::execute_input_bound (input_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance ()) {
      // Automaton does not exist.
      return -1;
    }

    exec.bound ();
    return 0;
  }

  int model::execute_output_unbound (output_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance ()) {
      // Automaton does not exist.
      return -1;
    }
    
    exec.unbound ();
    return 0;
  }

  int model::execute_input_unbound (input_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance ()) {
      // Automaton does not exist.
      return -1;
    }

    exec.unbound ();
    return 0;
  }

  void model::lock_automaton (const aid_t handle) {
    m_records[handle]->lock ();
  }

  void model::unlock_automaton (const aid_t handle) {
    m_records[handle]->unlock ();
  }

  // This should only be called from user code because we don't get a lock.
  size_t model::bind_count (const action_interface& action) {
    std::list<output_executor_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
										 m_bindings.end (),
										 binding_input_equal (action));
    
    if (in_pos != m_bindings.end ()) {
      // Input is bound.
      return 1;
    }
    
    std::list<output_executor_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
										  m_bindings.end (),
										  binding_output_equal (action));
    
    if (out_pos != m_bindings.end ()) {
      // Output is bound.
      return (*out_pos)->size ();
    }
    
    return 0;
  }
}
