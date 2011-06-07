#ifndef __automaton_interface_hpp__
#define __automaton_interface_hpp__

#include <ioa/shared_ptr.hpp>
#include <cassert>
#include <queue>
#include <ioa/action.hpp>
#include <set>
#include <ioa/generator_interface.hpp>

#define COMMA ,

#define SYSTEM_INPUT(c, name, type, var)		\
  public: \
  typedef ioa::system_input_wrapper<c, type> name##_type;	\
  name##_type name; \
  private:	    \
  void _##name (type const & var)

#define SYSTEM_OUTPUT(c, name, type)			\
  public: \
  typedef ioa::system_output_wrapper<c, type> name##_type;	\
  name##_type name; \
  private:	    \
  std::pair<bool, type> _##name ()

#define ACTION(c, name) name (*this, &c::_##name, &c::name)

namespace ioa {

  template <class C, class T>
  class system_input_wrapper :
    public system_input,
    public value<T>
  {
  private:
    C& m_c;
    void (C::*m_member_function_ptr)(T const &);
    
  public:
    system_input_wrapper (C& c,
			  void (C::*member_function_ptr)(T const &),
			  system_input_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }
    
    void operator() (T const & t) {
      (m_c.*m_member_function_ptr) (t);
    }
  };

  template <class C, class T>
  class system_output_wrapper :
    public system_output,
    public value<T>
  {
  private:
    C& m_c;
    std::pair<bool, T> (C::*m_member_function_ptr)(void);
    
  public:
    system_output_wrapper (C& c,
			   std::pair<bool, T> (C::*member_function_ptr)(void),
			   system_output_wrapper C::*member_object_ptr) :
      m_c (c),
      m_member_function_ptr (member_function_ptr)
    { }
    
    std::pair<bool, T> operator() () {
      return (m_c.*m_member_function_ptr) ();
    }
  };

  class automaton_helper_interface
  {
  public:
    virtual ~automaton_helper_interface () { }
    virtual shared_ptr<generator_interface> get_generator () const = 0;
    virtual void instance_exists () = 0;
    virtual void automaton_created (const aid_t aid) = 0;
    virtual void automaton_destroyed () = 0;
  };

  class automaton_interface {
  private:
    // Invariant:  m_create_send INTERSECT m_create_recv INTERSECT m_destroy_send = empty set
    // Basically, a helper is in one place or another.
    // The send sets are waiting to send the command.
    // The receive set is waiting for a response.
    std::set<automaton_helper_interface*> m_create_send;
    std::set<automaton_helper_interface*> m_create_recv;
    std::set<automaton_helper_interface*> m_destroy_send;
    std::set<automaton_helper_interface*> m_destroy_recv;

  protected:
    automaton_interface () :
      ACTION (automaton_interface, sys_create),
      ACTION (automaton_interface, sys_bind),
      ACTION (automaton_interface, sys_unbind),
      ACTION (automaton_interface, sys_destroy),

      ACTION (automaton_interface, sys_create_key_exists),
      ACTION (automaton_interface, sys_instance_exists),
      ACTION (automaton_interface, sys_automaton_created),
      ACTION (automaton_interface, sys_bind_key_exists),
      ACTION (automaton_interface, sys_output_automaton_dne),
      ACTION (automaton_interface, sys_input_automaton_dne),
      ACTION (automaton_interface, sys_binding_exists),
      ACTION (automaton_interface, sys_output_action_unavailable),
      ACTION (automaton_interface, sys_input_action_unavailable),
      ACTION (automaton_interface, sys_bound),
      ACTION (automaton_interface, sys_bind_key_dne),
      ACTION (automaton_interface, sys_unbound),
      ACTION (automaton_interface, sys_create_key_dne),
      ACTION (automaton_interface, sys_automaton_destroyed),
      ACTION (automaton_interface, sys_recipient_dne),
      ACTION (automaton_interface, sys_event_delivered)
    { }

  public:
    virtual ~automaton_interface () {
      // Send the helpers a destroyed signal.
      for (std::set<automaton_helper_interface*>::const_iterator pos = m_create_send.begin ();
	   pos != m_create_send.end ();
	   ++pos) {
	(*pos)->automaton_destroyed ();
      }
      for (std::set<automaton_helper_interface*>::const_iterator pos = m_create_recv.begin ();
	   pos != m_create_recv.end ();
	   ++pos) {
	(*pos)->automaton_destroyed ();
      }
      for (std::set<automaton_helper_interface*>::const_iterator pos = m_destroy_send.begin ();
	   pos != m_destroy_send.end ();
	   ++pos) {
	(*pos)->automaton_destroyed ();
      }
      for (std::set<automaton_helper_interface*>::const_iterator pos = m_destroy_recv.begin ();
	   pos != m_destroy_recv.end ();
	   ++pos) {
	(*pos)->automaton_destroyed ();
      }
    }

  protected:
    void create (automaton_helper_interface* helper) {
      assert (helper != 0);
      // We should not have seen this helper before.  Otherwise, we will get a create_key_exists error.
      assert (m_create_send.count (helper) == 0 &&
	      m_create_recv.count (helper) == 0 &&
	      m_destroy_send.count (helper) == 0 &&
	      m_destroy_recv.count (helper) == 0);
      // Add to the set and send queue.
      m_create_send.insert (helper);
      schedule ();
    }

    void bind () { assert (false); }
    void unbind () { assert (false); }

    void destroy (automaton_helper_interface* helper) {
      assert (helper != 0);

      // Error to destroy again.
      assert (m_destroy_send.count (helper) == 0 &&
	      m_destroy_recv.count (helper) == 0);

      std::set<automaton_helper_interface*>::const_iterator send_iter = m_create_send.find (helper);
      std::set<automaton_helper_interface*>::const_iterator recv_iter = m_create_recv.find (helper);

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

  private:

    void schedule ();

    bool sys_create_precondition () const {
      return !m_create_send.empty ();
    }

    SYSTEM_OUTPUT (automaton_interface, sys_create, std::pair<shared_ptr<generator_interface> COMMA void*>) {
      std::pair<bool, std::pair<shared_ptr<generator_interface>, void*> > retval;

      if (sys_create_precondition ()) {
	std::set<automaton_helper_interface*>::iterator pos = m_create_send.begin ();
	automaton_helper_interface* helper = *pos;
	m_create_send.erase (pos);
	m_create_recv.insert (helper);
	retval = std::make_pair (true, std::make_pair (helper->get_generator (), helper));
      }

      schedule ();
      return retval;
    }

    SYSTEM_OUTPUT (automaton_interface, sys_bind, int) { assert (false); }
    SYSTEM_OUTPUT (automaton_interface, sys_unbind, int) { assert (false); }

    bool sys_destroy_precondition () const {
      return !m_destroy_send.empty ();
    }

    SYSTEM_OUTPUT (automaton_interface, sys_destroy, void*) {
      std::pair<bool, void*> retval;

      if (sys_destroy_precondition ()) {
	std::set<automaton_helper_interface*>::iterator pos = m_destroy_send.begin ();
	automaton_helper_interface* helper = *pos;
	m_destroy_send.erase (pos);
	m_destroy_recv.insert (helper);
	retval = std::make_pair (true, helper);
      }

      schedule ();
      return retval;
    }

    SYSTEM_INPUT (automaton_interface, sys_create_key_exists, void*, t) {
      // We prevent this in create.
      assert (false);
    }

    SYSTEM_INPUT (automaton_interface, sys_instance_exists, void*, t) {
      // Find the helper (sanity check).
      std::set<automaton_helper_interface*>::const_iterator pos = m_create_recv.find (static_cast<automaton_helper_interface*> (t));
      assert (pos != m_create_recv.end ());
      (*pos)->instance_exists ();
      // The creation failed so erase.
      m_create_recv.erase (pos);
      schedule ();
    }

    SYSTEM_INPUT (automaton_interface, sys_automaton_created, std::pair<void* COMMA aid_t>, t) {
      // Find the helper (sanity check).
      std::set<automaton_helper_interface*>::const_iterator pos = m_create_recv.find (static_cast<automaton_helper_interface*> (t.first));
      assert (pos != m_create_recv.end ());
      (*pos)->automaton_created (t.second);
      schedule ();
    }

    SYSTEM_INPUT (automaton_interface, sys_bind_key_exists, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_output_automaton_dne, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_input_automaton_dne, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_binding_exists, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_output_action_unavailable, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_input_action_unavailable, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_bound, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_bind_key_dne, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_unbound, void*, t) { assert (false); }

    SYSTEM_INPUT (automaton_interface, sys_create_key_dne, void*, t) {
      // We prevent this in destroy.
      assert (false);
    }

    SYSTEM_INPUT (automaton_interface, sys_automaton_destroyed, void*, t) {
      // Find the helper (sanity check).
      std::set<automaton_helper_interface*>::const_iterator pos = m_destroy_recv.find (static_cast<automaton_helper_interface*> (t));
      assert (pos != m_destroy_recv.end ());
      (*pos)->automaton_destroyed ();
      // Remove from the destroy receive set.
      m_destroy_recv.erase (pos);
      schedule ();
    }

    SYSTEM_INPUT (automaton_interface, sys_recipient_dne, void*, t) { assert (false); }
    SYSTEM_INPUT (automaton_interface, sys_event_delivered, void*, t) { assert (false); }
  };

}

#endif
