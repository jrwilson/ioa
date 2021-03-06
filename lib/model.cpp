/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "model.hpp"

#include <ioa/shared_mutex.hpp>
#include "unique_lock.hpp"
#include "shared_lock.hpp"
#include <algorithm>
#include <ioa/allocator_interface.hpp>
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

  void model::add_bind_key (const aid_t binder,
			    void* const key) {
    m_records[binder]->add_bind_key (key);
  }

  void model::remove_bind_key (const aid_t binder,
			       void* const key) {
    m_records[binder]->remove_bind_key (key);
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
  
  aid_t model::create (std::auto_ptr<allocator_interface> allocator)
  {
    unique_lock lock (m_mutex);
    
    // Take an aid.
    aid_t aid = m_aids.take ();
    
    // Set the current aid.
    m_system_scheduler.set_current_aid (aid);
    
    // Run the allocator.
    automaton* instance = (*allocator) ();
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
		       std::auto_ptr<allocator_interface> allocator,
		       void* const key)
  {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (creator_aid)) {
      // Creator does not exists.
      return -1;
    }

    if (m_records[creator_aid]->create_key_exists (key)) {
      // Create key already in use.
      m_system_scheduler.created (creator_aid, CREATE_KEY_EXISTS_RESULT, key, -1);
      return -1;
    }
    
    // Take an aid.
    aid_t aid = m_aids.take ();
    
    // Set the current aid.
    m_system_scheduler.set_current_aid (aid);
    
    // Run the allocator.
    automaton* instance = (*allocator) ();
    assert (instance != 0);
    
    // Clear the current aid.
    m_system_scheduler.clear_current_aid ();
    
    if (m_instances.count (instance) != 0) {
      // Return the aid and inform the automaton that the instance already exists.
      m_aids.replace (aid);
      m_system_scheduler.created (creator_aid, INSTANCE_EXISTS_RESULT, key, -1);
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
		   std::auto_ptr<bind_executor_interface> bind_exec,
		   void* const key) {
    unique_lock lock (m_mutex);
    
    if (!m_aids.contains (binder)) {
      // Binder DNE.
      return -1;
    }
    
    if (m_records[binder]->bind_key_exists (key)) {
      // Bind key already in use.
      m_system_scheduler.bound (binder, BIND_KEY_EXISTS_RESULT, key);
      return -1;
    }
    
    output_executor_interface& output = bind_exec->get_output ();
    input_executor_interface& input = bind_exec->get_input ();

    // Set the parameters in the event that they are auto_parameterized.
    output.set_parameter (input.get_aid ());
    input.set_parameter (output.get_aid ());
    
    if (!output.fetch_instance (*this)) {
      m_system_scheduler.bound (binder, OUTPUT_AUTOMATON_DNE_RESULT, key);
      return -1;
    }
    
    if (!input.fetch_instance (*this)) {
      m_system_scheduler.bound (binder, INPUT_AUTOMATON_DNE_RESULT, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									      m_bindings.end (),
									      binding_equal (output, input, binder));
    
    if (pos != m_bindings.end ()) {
      // Bound.
      m_system_scheduler.bound (binder, BINDING_EXISTS_RESULT, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
										 m_bindings.end (),
										 binding_input_equal (input));
    
    if (in_pos != m_bindings.end ()) {
      // Input unavailable.
      m_system_scheduler.bound (binder, INPUT_ACTION_UNAVAILABLE_RESULT, key);
      return -1;
    }
    
    std::list<output_executor_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
										  m_bindings.end (),
										  binding_output_equal (output));
    
    if (output.get_aid () == input.get_aid () ||
	(out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_aid ()))) {
      // Output unavailable.
      m_system_scheduler.bound (binder, OUTPUT_ACTION_UNAVAILABLE_RESULT, key);
      return -1;
    }
    
    output_executor_interface* c;
    
    if (out_pos != m_bindings.end ()) {
      c = *out_pos;
    }
    else {
      c = output.clone ().release ();
      m_bindings.push_front (c);
    }
    
    // Bind.
    c->bind (m_system_scheduler, *this, input, binder, key);
    
    return 0;
  }    

  int model::unbind (const aid_t binder,
		      void* const key)
  {
    unique_lock lock (m_mutex);

    if (!m_aids.contains (binder)) {
      // Binder does not exist.
      return -1;
    }
    
    std::list<output_executor_interface*>::iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_aid_key_equal (binder, key));
    
    if (pos == m_bindings.end ()) {
      // Not bound.
      m_system_scheduler.unbound (binder, BIND_KEY_DNE_RESULT, key);
      return -1;
    }
    
    output_executor_interface* c = *pos;
    
    // Unbind.
    c->unbind (binder, key);
    
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
      m_system_scheduler.destroyed (automaton, CREATE_KEY_DNE_RESULT, key);
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
    if (instance->sys_create.precondition (const_cast<const automaton&> (*instance))) {
      std::pair<allocator_interface*, void*> key = instance->sys_create.effect (*instance);
      instance->sys_create.schedule (const_cast<const automaton&> (*instance));
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
      m_system_scheduler.create (aid, std::auto_ptr<allocator_interface> (key.first), key.second);
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
    if (instance->sys_bind.precondition (const_cast<const automaton&> (*instance))) {
      std::pair<bind_executor_interface*, void*> key = instance->sys_bind.effect (*instance);
      instance->sys_bind.schedule (const_cast<const automaton&> (*instance));
      m_system_scheduler.clear_current_aid ();
      unlock_automaton (aid);
      m_system_scheduler.bind (aid, std::auto_ptr<bind_executor_interface> (key.first), key.second);
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
    if (instance->sys_unbind.precondition (const_cast<const automaton&> (*instance))) {
      void* key = instance->sys_unbind.effect (*instance);
      instance->sys_unbind.schedule (const_cast<const automaton&> (*instance));
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
    if (instance->sys_destroy.precondition (const_cast<const automaton&> (*instance))) {
      void* key = instance->sys_destroy.effect (*instance);
      instance->sys_destroy.schedule (const_cast<const automaton&> (*instance));
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
  size_t model::binding_count (const action_executor_interface& action) const {
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
