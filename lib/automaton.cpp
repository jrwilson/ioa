#include <ioa/automaton.hpp>
#include <ioa/scheduler.hpp>
#include <ioa/const_shared_ptr.hpp>

namespace ioa {

  automaton::automaton () { }

  automaton::~automaton () {
    // Send the helpers a destroyed signal.
    for (std::set<system_automaton_manager_interface*>::const_iterator pos = m_create_send.begin ();
	 pos != m_create_send.end ();
	 ++pos) {
      (*pos)->destroyed (AUTOMATON_DESTROYED_RESULT);
    }
    for (std::set<system_automaton_manager_interface*>::const_iterator pos = m_create_recv.begin ();
	 pos != m_create_recv.end ();
	 ++pos) {
      (*pos)->destroyed (AUTOMATON_DESTROYED_RESULT);
    }
    for (std::set<system_automaton_manager_interface*>::const_iterator pos = m_create_done.begin ();
	 pos != m_create_done.end ();
	 ++pos) {
      (*pos)->destroyed (AUTOMATON_DESTROYED_RESULT);
    }
    for (std::set<system_automaton_manager_interface*>::const_iterator pos = m_destroy_send.begin ();
	 pos != m_destroy_send.end ();
	 ++pos) {
      (*pos)->destroyed (AUTOMATON_DESTROYED_RESULT);
    }
    for (std::set<system_automaton_manager_interface*>::const_iterator pos = m_destroy_recv.begin ();
	 pos != m_destroy_recv.end ();
	 ++pos) {
      (*pos)->destroyed (AUTOMATON_DESTROYED_RESULT);
    }
    for (std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_send.begin ();
	 pos != m_bind_send.end ();
	 ++pos) {
      (*pos)->unbound (UNBOUND_RESULT);
    }
    for (std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_recv.begin ();
	 pos != m_bind_recv.end ();
	 ++pos) {
      (*pos)->unbound (UNBOUND_RESULT);
    }
    for (std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_done.begin ();
	 pos != m_bind_done.end ();
	 ++pos) {
      (*pos)->unbound (UNBOUND_RESULT);
    }
    for (std::set<system_binding_manager_interface*>::const_iterator pos = m_unbind_send.begin ();
	 pos != m_unbind_send.end ();
	 ++pos) {
      (*pos)->unbound (UNBOUND_RESULT);
    }
    for (std::set<system_binding_manager_interface*>::const_iterator pos = m_unbind_recv.begin ();
	 pos != m_unbind_recv.end ();
	 ++pos) {
      (*pos)->unbound (UNBOUND_RESULT);
    }
  }

  void automaton::create (system_automaton_manager_interface* helper) {
    assert (helper != 0);
    // We should not have seen this helper before.  Otherwise, we will get a create_key_exists error.
    assert (m_create_send.count (helper) == 0 &&
	    m_create_recv.count (helper) == 0 &&
	    m_create_done.count (helper) == 0 &&
	    m_destroy_send.count (helper) == 0 &&
	    m_destroy_recv.count (helper) == 0);
    // Add to the send set and schedule.
    m_create_send.insert (helper);
    schedule ();
  }
  
  void automaton::bind (system_binding_manager_interface* helper) {
    assert (helper != 0);
    // We should not have seen this helper before.  Otherwise, we will get a bind_key_exists error.
    assert (m_bind_send.count (helper) == 0 &&
	    m_bind_recv.count (helper) == 0 &&
	    m_bind_done.count (helper) == 0 &&
	    m_unbind_send.count (helper) == 0 &&
	    m_unbind_recv.count (helper) == 0);
    // Add to the send set and schedule.
    m_bind_send.insert (helper);
    schedule ();
  }
  
  void automaton::unbind (system_binding_manager_interface* helper) {
    assert (helper != 0);

    std::set<system_binding_manager_interface*>::const_iterator send_iter = m_bind_send.find (helper);
    std::set<system_binding_manager_interface*>::const_iterator recv_iter = m_bind_recv.find (helper);
    std::set<system_binding_manager_interface*>::const_iterator done_iter = m_bind_done.find (helper);

    // Helper is somewhere in bind.
    assert ((send_iter != m_bind_send.end () &&
	     recv_iter == m_bind_recv.end () &&
	     done_iter == m_bind_done.end ()) ||
	    (send_iter == m_bind_send.end () &&
	     recv_iter != m_bind_recv.end () &&
	     done_iter == m_bind_done.end ()) ||
	    (send_iter == m_bind_send.end () &&
	     recv_iter == m_bind_recv.end () &&
	     done_iter != m_bind_done.end ()));
    
    // Error to unbind again.
    assert (m_unbind_send.count (helper) == 0 &&
	    m_unbind_recv.count (helper) == 0);
    
    if (send_iter != m_bind_send.end ()) {
      // We haven't sent the bind request yet.  We can just remove and send it unbound.
      m_bind_send.erase (send_iter);
      helper->unbound (UNBOUND_RESULT);
    }
    else {
      // Unbind it.
      m_unbind_send.insert (helper);
    }

    schedule ();
  }
  
  void automaton::destroy (system_automaton_manager_interface* helper) {
    assert (helper != 0);

    std::set<system_automaton_manager_interface*>::const_iterator send_iter = m_create_send.find (helper);
    std::set<system_automaton_manager_interface*>::const_iterator recv_iter = m_create_recv.find (helper);
    std::set<system_automaton_manager_interface*>::const_iterator done_iter = m_create_done.find (helper);
    
    // Helper is somewhere in create.
    assert ((send_iter != m_create_send.end () &&
	     recv_iter == m_create_recv.end () &&
	     done_iter == m_create_done.end ()) ||
	    (send_iter == m_create_send.end () &&
	     recv_iter != m_create_recv.end () &&
	     done_iter == m_create_done.end ()) ||
	    (send_iter == m_create_send.end () &&
	     recv_iter == m_create_recv.end () &&
	     done_iter != m_create_done.end ()));

    // Error to destroy again.
    assert (m_destroy_send.count (helper) == 0 &&
	    m_destroy_recv.count (helper) == 0);

    if (send_iter != m_create_send.end ()) {
      // We haven't sent the create request yet.  We can just remove and send it destroyed.
      m_create_send.erase (send_iter);
      helper->destroyed (AUTOMATON_DESTROYED_RESULT);
    }
    else {
      // Destroy it.
      m_destroy_send.insert (helper);
    }

    schedule ();
  }

  bool automaton::sys_create_precondition () const {
    return !m_create_send.empty ();
  }

  std::pair<generator_interface*, void*> automaton::sys_create_effect () {
    std::set<system_automaton_manager_interface*>::iterator pos = m_create_send.begin ();
    system_automaton_manager_interface* helper = *pos;
    m_create_send.erase (pos);
    m_create_recv.insert (helper);
    // I really don't like return the raw pointer but I haven't found a better solution.
    // Some type of smart pointer would be nice.
    return std::make_pair (helper->get_generator ().release (), helper);
  }

  bool automaton::sys_bind_precondition () const {
    return !m_bind_send.empty ();
  }
  
  std::pair<shared_ptr<bind_executor_interface> , void*> automaton::sys_bind_effect () {
    std::set<system_binding_manager_interface*>::iterator pos = m_bind_send.begin ();
    system_binding_manager_interface* helper = *pos;
    m_bind_send.erase (pos);
    m_bind_recv.insert (helper);
    return std::make_pair (helper->get_executor (), helper);
  }

  bool automaton::sys_unbind_precondition () const {
    for (std::set<system_binding_manager_interface*>::const_iterator pos = m_unbind_send.begin ();
	 pos != m_unbind_send.end ();
	 ++pos) {
      if (m_bind_done.find (*pos) != m_bind_done.end ()) {
	// Found one that was fully created and needs to be unbound.
	return true;
      }
    }
    return false;
  }
  
  void* automaton::sys_unbind_effect () {
    std::set<system_binding_manager_interface*>::iterator pos;

    for (pos = m_unbind_send.begin ();
	 pos != m_unbind_send.end ();
	 ++pos) {
      if (m_bind_done.find (*pos) != m_bind_done.end ()) {
	// Found one that was fully created and needs to be destroyed.
	break;
      }
    }
    assert (pos != m_unbind_send.end ());
    system_binding_manager_interface* helper = *pos;
    m_unbind_send.erase (pos);
    m_unbind_recv.insert (helper);
    return helper;
  }

  bool automaton::sys_destroy_precondition () const {
    for (std::set<system_automaton_manager_interface*>::const_iterator pos = m_destroy_send.begin ();
	 pos != m_destroy_send.end ();
	 ++pos) {
      if (m_create_done.find (*pos) != m_create_done.end ()) {
	// Found one that was fully created and needs to be destroyed.
	return true;
      }
    }
    return false;
  }
  
  void* automaton::sys_destroy_effect () {
    std::set<system_automaton_manager_interface*>::iterator pos;

    for (pos = m_destroy_send.begin ();
	 pos != m_destroy_send.end ();
	 ++pos) {
      if (m_create_done.find (*pos) != m_create_done.end ()) {
	// Found one that was fully created and needs to be destroyed.
	break;
      }
    }
    assert (pos != m_destroy_send.end ());
    system_automaton_manager_interface* helper = *pos;
    m_destroy_send.erase (pos);
    m_destroy_recv.insert (helper);
    return helper;
  }

  void automaton::sys_created_effect (const created_arg_t& arg) {
    switch (arg.type) {
    case CREATE_KEY_EXISTS_RESULT:
      // We prevent this in create.
      assert (false);
      break;
    case INSTANCE_EXISTS_RESULT:
      {
	// Find the helper (sanity check).
	std::set<system_automaton_manager_interface*>::const_iterator pos = m_create_recv.find (static_cast<system_automaton_manager_interface*> (arg.key));
	assert (pos != m_create_recv.end ());
	(*pos)->created (INSTANCE_EXISTS_RESULT, -1);
	// The creation failed so erase.
	m_destroy_send.erase (*pos);
	m_create_recv.erase (pos);
      }
      break;
    case AUTOMATON_CREATED_RESULT:
      // Find the helper (sanity check).
      std::set<system_automaton_manager_interface*>::const_iterator pos = m_create_recv.find (static_cast<system_automaton_manager_interface*> (arg.key));
      assert (pos != m_create_recv.end ());
      (*pos)->created (AUTOMATON_CREATED_RESULT, arg.aid);
      // The create succeeded.  Move to done.
      m_create_done.insert (*pos);
      m_create_recv.erase (pos);
      break;
    }
  }
  
  void automaton::sys_bound_effect (std::pair<bound_t, void*> const & t) {
    switch (t.first) {
    case BIND_KEY_EXISTS_RESULT:
      // We prevent this in bind.
      assert (false);
      break;
    case OUTPUT_AUTOMATON_DNE_RESULT:
      {
	// Find the helper (sanity check).
	std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_binding_manager_interface*> (t.second));
	assert (pos != m_bind_recv.end ());
	(*pos)->bound (OUTPUT_AUTOMATON_DNE_RESULT);
	// The bind failed so erase.
	m_unbind_send.erase (*pos);
	m_bind_recv.erase (pos);
      }
      break;
    case INPUT_AUTOMATON_DNE_RESULT:
      {
	// Find the helper (sanity check).
	std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_binding_manager_interface*> (t.second));
	assert (pos != m_bind_recv.end ());
	(*pos)->bound (INPUT_AUTOMATON_DNE_RESULT);
	// The bind failed so erase.
	m_unbind_send.erase (*pos);
	m_bind_recv.erase (pos);
      }
      break;
    case BINDING_EXISTS_RESULT:
      {
	// Find the helper (sanity check).
	std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_binding_manager_interface*> (t.second));
	assert (pos != m_bind_recv.end ());
	(*pos)->bound (BINDING_EXISTS_RESULT);
	// The bind failed so erase.
	m_unbind_send.erase (*pos);
	m_bind_recv.erase (pos);
      }
      break;
    case OUTPUT_ACTION_UNAVAILABLE_RESULT:
      {
	// Find the helper (sanity check).
	std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_binding_manager_interface*> (t.second));
	assert (pos != m_bind_recv.end ());
	(*pos)->bound (OUTPUT_ACTION_UNAVAILABLE_RESULT);
	// The bind failed so erase.
	m_unbind_send.erase (*pos);
	m_bind_recv.erase (pos);
      }
      break;
    case INPUT_ACTION_UNAVAILABLE_RESULT:
      {
	// Find the helper (sanity check).
	std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_binding_manager_interface*> (t.second));
	assert (pos != m_bind_recv.end ());
	(*pos)->bound (INPUT_ACTION_UNAVAILABLE_RESULT);
	// The bind failed so erase.
	m_unbind_send.erase (*pos);
	m_bind_recv.erase (pos);
      }
      break;
    case BOUND_RESULT:
      // Find the helper (sanity check).
      std::set<system_binding_manager_interface*>::const_iterator pos = m_bind_recv.find (static_cast<system_binding_manager_interface*> (t.second));
      assert (pos != m_bind_recv.end ());
      (*pos)->bound (BOUND_RESULT);
      // The bind succeeded.  Move to done.
      m_bind_done.insert (*pos);
      m_bind_recv.erase (pos);
      break;
    }
  }
  
  void automaton::sys_unbound_effect (std::pair<unbound_t, void*> const & t) {
    switch (t.first) {
    case BIND_KEY_DNE_RESULT:
      // We prevent this in unbind.
      assert (false);
      break;
    case UNBOUND_RESULT:
      // Something was unbound.
      // It can be in m_bind_recv or m_bind_done and also m_unbind_send or m_unbind_recv.
      
      system_binding_manager_interface* helper = static_cast<system_binding_manager_interface*> (t.second);
      
      assert ((m_bind_recv.count (helper) == 1 &&
	       m_bind_done.count (helper) == 0) ||
	      (m_bind_recv.count (helper) == 0 &&
	       m_bind_done.count (helper) == 1));
      
      if (m_bind_recv.count (helper) != 0) {
	m_bind_recv.erase (helper);
      }
      else if (m_bind_done.count (helper) != 0) {
	m_bind_done.erase (helper);
      }
      
      if (m_unbind_send.count (helper) != 0) {
	m_unbind_send.erase (helper);
      }
      else if (m_unbind_recv.count (helper) != 0) {
	m_unbind_recv.erase (helper);
      }
      
      helper->unbound (UNBOUND_RESULT);
      break;
    }
  }
  
  void automaton::sys_destroyed_effect (std::pair<destroyed_t, void*> const & t) {
    switch (t.first) {
    case CREATE_KEY_DNE_RESULT:
      // We prevent this in destroy.
      assert (false);
      break;
    case AUTOMATON_DESTROYED_RESULT:
      // An automaton was destroyed.
      // It can be in m_create_recv or m_create_done and also m_destroy_send or m_destroy_recv.
      
      system_automaton_manager_interface* helper = static_cast<system_automaton_manager_interface*> (t.second);
      
      assert ((m_create_recv.count (helper) == 1 &&
	       m_create_done.count (helper) == 0) ||
	      (m_create_recv.count (helper) == 0 &&
	       m_create_done.count (helper) == 1));
      
      if (m_create_recv.count (helper) != 0) {
	m_create_recv.erase (helper);
      }
      else if (m_create_done.count (helper) != 0) {
	m_create_done.erase (helper);
      }
      
      if (m_destroy_send.count (helper) != 0) {
	m_destroy_send.erase (helper);
      }
      else if (m_destroy_recv.count (helper) != 0) {
	m_destroy_recv.erase (helper);
      }
      
      helper->destroyed (AUTOMATON_DESTROYED_RESULT);
      
      break;
    }
  }

  void automaton::schedule () const {
    if (sys_create_precondition ()) {
      ioa::schedule (&automaton::sys_create);
    }
    if (sys_bind_precondition ()) {
      ioa::schedule (&automaton::sys_bind);
    }
    if (sys_unbind_precondition ()) {
      ioa::schedule (&automaton::sys_unbind);
    }
    if (sys_destroy_precondition ()) {
      ioa::schedule (&automaton::sys_destroy);
    }
  }

}
