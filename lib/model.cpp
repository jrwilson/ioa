#include "model.hpp"

#include <ioa/shared_mutex.hpp>
#include "unique_lock.hpp"
#include "shared_lock.hpp"
#include <algorithm>
#include <ioa/generator_interface.hpp>
#include <ioa/automaton.hpp>
#include <ioa/system_scheduler_interface.hpp>
#include <ioa/automaton_handle.hpp>

namespace ioa {

  model::model (system_scheduler_interface& system_scheduler) :
    m_system_scheduler (system_scheduler)
  { }

  model::~model () {
    clear ();
  }

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
    m_system_scheduler.set_current_aid (aid);
    
    // Run the generator.
    automaton* instance = (*generator) ();
    assert (instance != 0);
    
    // Clear the current aid.
    m_system_scheduler.clear_current_aid ();
    
    if (m_instances.count (instance) != 0) {
      // Root automaton instance exists.  Bad news.
      m_aids.replace (aid);
      return -1;
    }
    
    m_instances.insert (instance);
    automaton_record* record = new automaton_record (m_system_scheduler, instance, aid);
    m_records.insert (std::make_pair (aid, record));
        
    return aid;
  }
  
  aid_t model::create (const aid_t creator_aid,
			shared_ptr<generator_interface> generator,
			void* const key)
  {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (creator_aid)) {
      // Creator does not exists.
      return -1;
    }

    if (m_records[creator_aid]->create_key_exists (key)) {
      // Create key already in use.
      m_system_scheduler.create_key_exists (creator_aid, key);
      return -1;
    }
    
    // Take an aid.
    aid_t aid = m_aids.take ();
    
    // Set the current aid.
    m_system_scheduler.set_current_aid (aid);
    
    // Run the generator.
    automaton* instance = (*generator) ();
    assert (instance != 0);
    
    // Clear the current aid.
    m_system_scheduler.clear_current_aid ();
    
    if (m_instances.count (instance) != 0) {
      // Return the aid and inform the automaton that the instance already exists.
      m_aids.replace (aid);
      m_system_scheduler.instance_exists (creator_aid, key);
      return -1;
    }
    
    m_instances.insert (instance);      
    automaton_record* record = new automaton_record (m_system_scheduler, instance, aid);
    m_records.insert (std::make_pair (aid, record));
    automaton_record* parent = m_records[creator_aid];
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
      m_system_scheduler.bind_key_exists (binder, key);
      return -1;
    }
    
    output_executor_interface& output = bind_exec->get_output ();
    input_executor_interface& input = bind_exec->get_input ();

    // Set the parameters in the event that they are auto_parameterized.
    output.set_parameter (input.get_aid ());
    input.set_parameter (output.get_aid ());
    
    if (!output.fetch_instance (*this)) {
      m_system_scheduler.output_automaton_dne (binder, key);
      return -1;
    }
    
    if (!input.fetch_instance (*this)) {
      m_system_scheduler.input_automaton_dne (binder, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									      m_bindings.end (),
									      binding_equal (output, input, binder));
    
    if (pos != m_bindings.end ()) {
      // Bound.
      m_system_scheduler.binding_exists (binder, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
										 m_bindings.end (),
										 binding_input_equal (input));
    
    if (in_pos != m_bindings.end ()) {
      // Input unavailable.
      m_system_scheduler.input_action_unavailable (binder, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
										  m_bindings.end (),
										  binding_output_equal (output));
    
    if (output.get_aid () == input.get_aid () ||
	(out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_aid ()))) {
      // Output unavailable.
      m_system_scheduler.output_action_unavailable (binder, key);
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
    c->bind (m_system_scheduler, input, binder, key);
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
      m_system_scheduler.bind_key_dne (binder, key);
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
      m_system_scheduler.create_key_dne (automaton, key);
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
    
    if (!exec.fetch_instance (*this)) {
      // Automaton does not exist.
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
										  m_bindings.end (),
										  binding_output_equal (exec));
    
    if (out_pos == m_bindings.end ()) {
      // Not bound.
      exec (*this, m_system_scheduler);
    }
    else {
      (*(*out_pos)) (*this, m_system_scheduler);
    }
    
    return 0;
  }
  
  int model::execute (internal_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance (*this)) {
      // Automaton does not exist.
      return -1;
    }
    
    exec (*this, m_system_scheduler);
    return 0;
  }
  
  int model::execute (system_input_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance (*this)) {
      // Automaton does not exist.
      return -1;
    }
    
    exec (*this, m_system_scheduler);
    return 0;
  }

  int model::execute_sys_create (const aid_t aid) {
    shared_lock lock (m_mutex);

    if (!m_aids.contains (aid)) {
      // Automaton does not exists.
      return -1;
    }

    automaton* instance = get_instance (automaton_handle<automaton> (aid));

    lock_automaton (aid);
    m_system_scheduler.set_current_aid (aid);
    if (instance->sys_create.precondition (*instance)) {
      std::pair<shared_ptr<generator_interface>, void*> key = instance->sys_create (*instance);
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
      m_system_scheduler.create (aid, key.first, key.second);
    }
    else {
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
    }

    return 0;
  }

  int model::execute_sys_bind (const aid_t aid) {
    shared_lock lock (m_mutex);

    if (!m_aids.contains (aid)) {
      // Automaton does not exists.
      return -1;
    }

    automaton* instance = get_instance (automaton_handle<automaton> (aid));

    lock_automaton (aid);
    m_system_scheduler.set_current_aid (aid);
    if (instance->sys_bind.precondition (*instance)) {
      std::pair<shared_ptr<bind_executor_interface>, void*> key = instance->sys_bind (*instance);
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
      m_system_scheduler.bind (aid, key.first, key.second);
    }
    else {
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
    }

    return 0;
  }

  int model::execute_sys_unbind (const aid_t aid) {
    shared_lock lock (m_mutex);

    if (!m_aids.contains (aid)) {
      // Automaton does not exists.
      return -1;
    }

    automaton* instance = get_instance (automaton_handle<automaton> (aid));

    lock_automaton (aid);
    m_system_scheduler.set_current_aid (aid);
    if (instance->sys_unbind.precondition (*instance)) {
      void* key = instance->sys_unbind (*instance);
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
      m_system_scheduler.unbind (aid, key);
    }
    else {
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
    }

    return 0;
  }

  int model::execute_sys_destroy (const aid_t aid) {
    shared_lock lock (m_mutex);

    if (!m_aids.contains (aid)) {
      // Automaton does not exists.
      return -1;
    }

    automaton* instance = get_instance (automaton_handle<automaton> (aid));

    lock_automaton (aid);
    m_system_scheduler.set_current_aid (aid);
    if (instance->sys_destroy.precondition (*instance)) {
      void* key = instance->sys_destroy (*instance);
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
      m_system_scheduler.destroy (aid, key);
    }
    else {
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
    }

    return 0;
  }

  int model::execute_output_bound (output_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance (*this)) {
      // Automaton does not exist.
      return -1;
    }
    
    exec.bound (*this, m_system_scheduler);
    return 0;
  }

  int model::execute_input_bound (input_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance (*this)) {
      // Automaton does not exist.
      return -1;
    }

    exec.bound (*this, m_system_scheduler);
    return 0;
  }

  int model::execute_output_unbound (output_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance (*this)) {
      // Automaton does not exist.
      return -1;
    }
    
    exec.unbound (*this, m_system_scheduler);
    return 0;
  }

  int model::execute_input_unbound (input_executor_interface& exec) {
    shared_lock lock (m_mutex);
    
    if (!exec.fetch_instance (*this)) {
      // Automaton does not exist.
      return -1;
    }

    exec.unbound (*this, m_system_scheduler);
    return 0;
  }

  automaton* model::get_instance (const aid_t aid) {
    std::map<aid_t, automaton_record*>::const_iterator pos = m_records.find (aid);
    if (pos != m_records.end ()) {
      return pos->second->get_instance ();
    }
    else {
      return 0;
    }
  }

  void model::lock_automaton (const aid_t handle) {
    m_records[handle]->lock ();
  }

  void model::unlock_automaton (const aid_t handle) {
    m_records[handle]->unlock ();
  }

  // This should only be called from user code because we don't get a lock.
  size_t model::bind_count (const action_executor_interface& action) const {
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
