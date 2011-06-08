#include <ioa/automaton_interface.hpp>
#include <ioa/scheduler.hpp>

namespace ioa {

  automaton_interface::~automaton_interface () {
    // Send the helpers a destroyed signal.
    for (std::set<system_automaton_helper_interface*>::const_iterator pos = m_create_send.begin ();
	 pos != m_create_send.end ();
	 ++pos) {
      (*pos)->automaton_destroyed ();
    }
    for (std::set<system_automaton_helper_interface*>::const_iterator pos = m_create_recv.begin ();
	 pos != m_create_recv.end ();
	 ++pos) {
      (*pos)->automaton_destroyed ();
    }
    for (std::set<system_automaton_helper_interface*>::const_iterator pos = m_destroy_send.begin ();
	 pos != m_destroy_send.end ();
	 ++pos) {
      (*pos)->automaton_destroyed ();
    }
    for (std::set<system_automaton_helper_interface*>::const_iterator pos = m_destroy_recv.begin ();
	 pos != m_destroy_recv.end ();
	 ++pos) {
      (*pos)->automaton_destroyed ();
    }
    for (std::set<system_bind_helper_interface*>::const_iterator pos = m_bind_send.begin ();
	 pos != m_bind_send.end ();
	 ++pos) {
      (*pos)->unbound ();
    }
    for (std::set<system_bind_helper_interface*>::const_iterator pos = m_bind_recv.begin ();
	 pos != m_bind_recv.end ();
	 ++pos) {
      (*pos)->unbound ();
    }
    for (std::set<system_bind_helper_interface*>::const_iterator pos = m_unbind_send.begin ();
	 pos != m_unbind_send.end ();
	 ++pos) {
      (*pos)->unbound ();
    }
    for (std::set<system_bind_helper_interface*>::const_iterator pos = m_unbind_recv.begin ();
	 pos != m_unbind_recv.end ();
	 ++pos) {
      (*pos)->unbound ();
    }
  }

  void automaton_interface::create (system_automaton_helper_interface* helper) {
    assert (helper != 0);
    // We should not have seen this helper before.  Otherwise, we will get a create_key_exists error.
    assert (m_create_send.count (helper) == 0 &&
	    m_create_recv.count (helper) == 0 &&
	    m_destroy_send.count (helper) == 0 &&
	    m_destroy_recv.count (helper) == 0);
    // Add to the send set and schedule.
    m_create_send.insert (helper);
    schedule ();
  }
  
  void automaton_interface::bind (system_bind_helper_interface* helper) {
    assert (helper != 0);
    // We should not have seen this helper before.  Otherwise, we will get a bind_key_exists error.
    assert (m_bind_send.count (helper) == 0 &&
	    m_bind_recv.count (helper) == 0 &&
	    m_unbind_send.count (helper) == 0 &&
	    m_unbind_recv.count (helper) == 0);
    // Add to the send set and schedule.
    m_bind_send.insert (helper);
    schedule ();
  }
  
  void automaton_interface::unbind (system_bind_helper_interface* helper) {
    assert (helper != 0);
    
    // Error to unbind again.
    assert (m_unbind_send.count (helper) == 0 &&
	    m_unbind_recv.count (helper) == 0);
    
    std::set<system_bind_helper_interface*>::const_iterator send_iter = m_bind_send.find (helper);
    std::set<system_bind_helper_interface*>::const_iterator recv_iter = m_bind_recv.find (helper);
    
    if (send_iter != m_bind_send.end () && recv_iter != m_bind_recv.end ()) {
      // Invariant is violated.
      assert (false);
    }
    else if (send_iter != m_bind_send.end () && recv_iter == m_bind_recv.end ()) {
      // We haven't sent the bind request yet.  We can just remove and send it unbound.
      m_bind_send.erase (send_iter);
      helper->unbound ();
    }
    else if (send_iter == m_bind_send.end () && recv_iter != m_bind_recv.end ()) {
      // We have sent the bind request.  Move from bind receive to unbind send.
      m_bind_recv.erase (recv_iter);
      m_unbind_send.insert (helper);
    }
    else {
      // We should have seen this helper before.  Otherwise, we will get a bind_key_dne error.
      assert (false);
    }
    
    schedule ();
  }
  
  void automaton_interface::destroy (system_automaton_helper_interface* helper) {
    assert (helper != 0);
    
    // Error to destroy again.
    assert (m_destroy_send.count (helper) == 0 &&
	    m_destroy_recv.count (helper) == 0);
    
    std::set<system_automaton_helper_interface*>::const_iterator send_iter = m_create_send.find (helper);
    std::set<system_automaton_helper_interface*>::const_iterator recv_iter = m_create_recv.find (helper);
    
    if (send_iter != m_create_send.end () && recv_iter != m_create_recv.end ()) {
      // Invariant is violated.
      assert (false);
    }
    else if (send_iter != m_create_send.end () && recv_iter == m_create_recv.end ()) {
      // We haven't sent the create request yet.  We can just remove and send it destroyed.
      m_create_send.erase (send_iter);
      helper->automaton_destroyed ();
    }
    else if (send_iter == m_create_send.end () && recv_iter != m_create_recv.end ()) {
      // We have sent the create request.  Move from create receive to destroy send.
      m_create_recv.erase (recv_iter);
      m_destroy_send.insert (helper);
    }
    else {
      // We should have seen this helper before.  Otherwise, we will get a create_key_dne error.
      assert (false);
    }
    
    schedule ();
  }

  bool automaton_interface::sys_create_precondition () const {
    return !m_create_send.empty ();
  }

  DEFINE_SYSTEM_OUTPUT (automaton_interface, sys_create, std::pair<shared_ptr<generator_interface> COMMA void*>) {
    std::pair<bool, std::pair<shared_ptr<generator_interface>, void*> > retval;
    
    if (sys_create_precondition ()) {
      std::set<system_automaton_helper_interface*>::iterator pos = m_create_send.begin ();
      system_automaton_helper_interface* helper = *pos;
      m_create_send.erase (pos);
      m_create_recv.insert (helper);
      retval = std::make_pair (true, std::make_pair (helper->get_generator (), helper));
    }
    
    schedule ();
    return retval;
  }

  bool automaton_interface::sys_bind_precondition () const {
    return !m_bind_send.empty ();
  }
  
  DEFINE_SYSTEM_OUTPUT (automaton_interface, sys_bind, std::pair<shared_ptr<bind_executor_interface> COMMA void*>) {
    std::pair<bool, std::pair<shared_ptr<bind_executor_interface>, void*> > retval;
    
    if (sys_bind_precondition ()) {
      std::set<system_bind_helper_interface*>::iterator pos = m_bind_send.begin ();
      system_bind_helper_interface* helper = *pos;
      m_bind_send.erase (pos);
      m_bind_recv.insert (helper);
      retval = std::make_pair (true, std::make_pair (helper->get_executor (), helper));
    }
    
    schedule ();
    return retval;
  }

  bool automaton_interface::sys_unbind_precondition () const {
    return !m_unbind_send.empty ();
  }
  
  DEFINE_SYSTEM_OUTPUT (automaton_interface, sys_unbind, void*) {
    std::pair<bool, void*> retval;
    
    if (sys_unbind_precondition ()) {
      std::set<system_bind_helper_interface*>::iterator pos = m_unbind_send.begin ();
      system_bind_helper_interface* helper = *pos;
      m_unbind_send.erase (pos);
      m_unbind_recv.insert (helper);
      retval = std::make_pair (true, helper);
    }
    
    schedule ();
    return retval;
  }

  bool automaton_interface::sys_destroy_precondition () const {
    return !m_destroy_send.empty ();
  }
  
  DEFINE_SYSTEM_OUTPUT (automaton_interface, sys_destroy, void*) {
    std::pair<bool, void*> retval;
    
    if (sys_destroy_precondition ()) {
      std::set<system_automaton_helper_interface*>::iterator pos = m_destroy_send.begin ();
      system_automaton_helper_interface* helper = *pos;
      m_destroy_send.erase (pos);
      m_destroy_recv.insert (helper);
      retval = std::make_pair (true, helper);
    }
    
    schedule ();
    return retval;
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_create_key_exists, void*, t) {
    // We prevent this in create.
    assert (false);
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_instance_exists, void*, t) {
    // Find the helper (sanity check).
    std::set<system_automaton_helper_interface*>::const_iterator pos = m_create_recv.find (static_cast<system_automaton_helper_interface*> (t));
    assert (pos != m_create_recv.end ());
    (*pos)->instance_exists ();
    // The creation failed so erase.
    m_create_recv.erase (pos);
    schedule ();
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_automaton_created, std::pair<void* COMMA aid_t>, t) {
    // Find the helper (sanity check).
    std::set<system_automaton_helper_interface*>::const_iterator pos = m_create_recv.find (static_cast<system_automaton_helper_interface*> (t.first));
    assert (pos != m_create_recv.end ());
    (*pos)->automaton_created (t.second);
    // The create succeeded.  Leave in set.
    schedule ();
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_bind_key_exists, void*, t) {
    // We prevent this in bind.
    assert (false);
  }
  
  DEFINE_SYSTEM_INPUT (automaton_interface, sys_output_automaton_dne, void*, t) {
    // Find the helper (sanity check).
    std::set<system_bind_helper_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_bind_helper_interface*> (t));
    assert (pos != m_bind_recv.end ());
    (*pos)->output_automaton_dne ();
    // The bind failed so erase.
    m_bind_recv.erase (pos);
    schedule ();
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_input_automaton_dne, void*, t) {
    // Find the helper (sanity check).
    std::set<system_bind_helper_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_bind_helper_interface*> (t));
    assert (pos != m_bind_recv.end ());
    (*pos)->input_automaton_dne ();
    // The bind failed so erase.
    m_bind_recv.erase (pos);
    schedule ();
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_binding_exists, void*, t) {
    // Find the helper (sanity check).
    std::set<system_bind_helper_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_bind_helper_interface*> (t));
    assert (pos != m_bind_recv.end ());
    (*pos)->binding_exists ();
    // The bind failed so erase.
    m_bind_recv.erase (pos);
    schedule ();
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_output_action_unavailable, void*, t) {
    // Find the helper (sanity check).
    std::set<system_bind_helper_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_bind_helper_interface*> (t));
    assert (pos != m_bind_recv.end ());
    (*pos)->output_action_unavailable ();
    // The bind failed so erase.
    m_bind_recv.erase (pos);
    schedule ();
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_input_action_unavailable, void*, t) {
    // Find the helper (sanity check).
    std::set<system_bind_helper_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_bind_helper_interface*> (t));
    assert (pos != m_bind_recv.end ());
    (*pos)->input_action_unavailable ();
    // The bind failed so erase.
    m_bind_recv.erase (pos);
    schedule ();
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_bound, void*, t) {
    // Find the helper (sanity check).
    std::set<system_bind_helper_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_bind_helper_interface*> (t));
    assert (pos != m_bind_recv.end ());
    (*pos)->bound ();
    // The bind succeeded.  Leave in set.
    schedule ();
  }
  
  DEFINE_SYSTEM_INPUT (automaton_interface, sys_bind_key_dne, void*, t) {
    // We prevent this in unbind.
    assert (false);
  }
  
  DEFINE_SYSTEM_INPUT (automaton_interface, sys_unbound, void*, t) {
    // Something was unbound.
    // It can be in m_bind_recv, m_unbind_send, or m_unbind_recv.
    
    system_bind_helper_interface* helper = static_cast<system_bind_helper_interface*> (t);
    
    if (m_bind_recv.count (helper) != 0) {
      m_bind_recv.erase (helper);
    }
    else if (m_unbind_send.count (helper) != 0) {
      m_unbind_send.erase (helper);
    }
    else if (m_unbind_recv.count (helper) != 0) {
      m_unbind_recv.erase (helper);
    }
    else {
      // Didn't find the helper.
      assert (false);
    }
    
    helper->unbound ();
    schedule ();
  }
  
  DEFINE_SYSTEM_INPUT (automaton_interface, sys_create_key_dne, void*, t) {
    // We prevent this in destroy.
    assert (false);
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_automaton_destroyed, void*, t) {
    // An automaton was destroyed.
    // It can be in m_create_recv, m_destroy_send, or m_destroy_recv.
    
    system_automaton_helper_interface* helper = static_cast<system_automaton_helper_interface*> (t);
    
    if (m_create_recv.count (helper) != 0) {
      m_create_recv.erase (helper);
    }
    else if (m_destroy_send.count (helper) != 0) {
      m_destroy_send.erase (helper);
    }
    else if (m_destroy_recv.count (helper) != 0) {
      m_destroy_recv.erase (helper);
    }
    else {
      // Didn't find the helper.
      assert (false);
    }
    
    helper->automaton_destroyed ();
    schedule ();
  }
  
  DEFINE_SYSTEM_INPUT (automaton_interface, sys_recipient_dne, void*, t) {
    // TODO
    assert (false);
  }

  DEFINE_SYSTEM_INPUT (automaton_interface, sys_event_delivered, void*, t) {
    // TODO
    assert (false);
  }

  void automaton_interface::schedule () {
    if (sys_create_precondition ()) {
      scheduler::schedule (&automaton_interface::sys_create);
    }
    if (sys_bind_precondition ()) {
      scheduler::schedule (&automaton_interface::sys_bind);
    }
    if (sys_unbind_precondition ()) {
      scheduler::schedule (&automaton_interface::sys_unbind);
    }
    if (sys_destroy_precondition ()) {
      scheduler::schedule (&automaton_interface::sys_destroy);
    }
  }

}
