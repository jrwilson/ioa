#include <ioa/system.hpp>
#include <ioa/automaton_locker.hpp>

namespace ioa {

  shared_mutex system::m_mutex;
  sequential_set<aid_t> system::m_aids;
  std::set<automaton_interface*> system::m_instances;
  std::map<aid_t, automaton_record*> system::m_records;
  std::list<binding_interface*> system::m_bindings;

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

  bool system::unbind (const bid_t bid,
		       const aid_t binder,
		       void* aux)
  {
    if (!m_aids.contains (binder)) {
      // Binder does not exist.
      return false;
    }
    
    std::list<binding_interface*>::iterator pos = std::find_if (m_bindings.begin (),
								m_bindings.end (),
								binding_aid_aux_equal (binder, aux));
    
    if (pos == m_bindings.end ()) {
      // Not bound.
      system_scheduler::bind_key_dne (binder, aux);
      return false;
    }
    
    binding_interface* c = *pos;
    
    // Unbind.
    c->unbind (binder, aux);
    m_records[binder]->replace_bid (bid);
    
    if (c->empty ()) {
      delete c;
      m_bindings.erase (pos);
    }
    
    return true;
  }

  bool system::destroy (const aid_t target)
  {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (target)) {
      return false;
    }
    
    inner_destroy (m_records[target]);
    return true;
  }
  
  bool system::destroy (const aid_t automaton,
			const aid_t target,
			void* aux)
  {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (automaton)) {
      // Destroyer does not exist.
      return false;
    }
    
    if (!m_aids.contains (target)) {
      //system_scheduler::target_automaton_dne (automaton, aux);
      return false;
    }
    
    automaton_record* parent = m_records[automaton];
    automaton_record* child = m_records[target];
    
    if (parent != child->get_parent ()) {
      //system_scheduler::destroyer_not_creator (automaton, aux);
      return false;
    }
    
    inner_destroy (child);
    return true;
  }

  void system::inner_destroy (automaton_record* automaton)
  {
    
    for (std::pair<void*, automaton_record*> p = automaton->get_first_child ();
	 p.second != 0;
	 p = automaton->get_first_child ()) {
      inner_destroy (p.second);
    }
    
    // Update bindings.
    for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
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

  void system::lock_automaton (const aid_t handle) {
    m_records[handle]->lock ();
  }

  void system::unlock_automaton (const aid_t handle) {
    m_records[handle]->unlock ();
  }
  
  // Implement automaton_locker.

  void automaton_locker::lock_automaton (const aid_t handle) {
    system::lock_automaton (handle);
  }

  void automaton_locker::unlock_automaton (const aid_t handle) {
    system::unlock_automaton (handle);
  }

}
