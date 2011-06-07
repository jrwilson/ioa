#include <ioa/system.hpp>

#include <algorithm>

namespace ioa {

  shared_mutex system::m_mutex;
  sequential_set<aid_t> system::m_aids;
  std::set<automaton_interface*> system::m_instances;
  std::map<aid_t, automaton_record*> system::m_records;
  std::list<output_executor_interface*> system::m_bindings;

  void system::clear (void) {
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
  
  aid_t system::create (std::auto_ptr<generator_interface> generator)
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
  
  aid_t system::create (const aid_t automaton,
			std::auto_ptr<generator_interface> generator,
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

  int system::bind (bind_executor_interface& bind_exec,
		    const aid_t binder,
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
    
    output_executor_interface& output = bind_exec.get_output ();
    input_executor_interface& input = bind_exec.get_input ();
    
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

  int system::unbind (const aid_t binder,
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

  int system::destroy (const aid_t target)
  {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (target)) {
      return -1;
    }
    
    inner_destroy (m_records[target]);
    return 0;
  }
  
  int system::destroy (const aid_t automaton,
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

  void system::inner_destroy (automaton_record* automaton)
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

  int system::execute (output_executor_interface& exec) {
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
  
  int system::execute (internal_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance ()) {
      // Automaton does not exist.
      return -1;
    }
    
    exec ();
    return 0;
  }
  
  int system::execute (const aid_t from,
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
  
  void system::lock_automaton (const aid_t handle) {
    m_records[handle]->lock ();
  }

  void system::unlock_automaton (const aid_t handle) {
    m_records[handle]->unlock ();
  }

}
