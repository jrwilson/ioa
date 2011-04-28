#ifndef __macro_action_hpp__
#define __macro_action_hpp__

#include "action.hpp"

namespace ioa {

  class scheduler_interface {
  public:
    virtual ~scheduler_interface () { }
    virtual void set_current_automaton (automaton*) = 0;
  };

  scheduler_interface& get_scheduler ();

  struct action_compare {
    bool operator() (const action_interface* x,
		     const action_interface* y) {
      return x->get_automaton_handle ().get_automaton () < y->get_automaton_handle ().get_automaton();
    }
  };

  class macro_action {

  public:
    virtual ~macro_action () { }
    virtual void execute () const = 0;
    virtual bool involves_output (const output_action_interface& output_action) const = 0;
    virtual bool involves_input (const input_action_interface& input_action) const = 0;
    virtual bool involves_input_check_owner (const input_action_interface& input_action) const = 0;
    virtual bool involves_automaton (const generic_automaton_handle& handle) const = 0;

    virtual bool empty () const = 0;
    virtual void decompose (const input_action_interface& input_action) = 0;
    virtual void decompose (const generic_parameter_handle& handle) = 0;
    virtual void decompose (const generic_automaton_handle&) = 0;
 };

  template <class OutputAction, class InputAction>
  class macro_action_base :
    public macro_action {

  protected:
    OutputAction* m_output_action;
    typedef std::list<InputAction*> list_type;
    list_type m_input_actions;

  public:
    template <class T>
    macro_action_base (const T& output_action) :
      m_output_action (new T (output_action))
    { }

    ~macro_action_base () {
      for(typename list_type::const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	(*pos)->decompose();
  	delete (*pos);
      }
      delete m_output_action;
    }

    bool involves_output (const output_action_interface& output_action) const {
      return *m_output_action == output_action;
    }

    bool involves_input (const input_action_interface& input_action) const {
      for(typename list_type::const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
  	if((*(*pos)) == input_action) {
  	  return true;
  	}
      }
      return false;
    }

    bool involves_input_check_owner (const input_action_interface& input_action) const {
      for(typename list_type::const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
  	if((*(*pos)) == input_action && (*(*pos)).get_owner_handle () == input_action.get_owner_handle ()) {
  	  return true;
  	}
      }
      return false;
    }

    bool involves_automaton (const generic_automaton_handle& handle) const {
      if (m_output_action->get_automaton_handle () == handle) {
	return true;
      }
      for (typename list_type::const_iterator pos = m_input_actions.begin();
	   pos != m_input_actions.end();
	   ++pos) {
  	if((*(*pos)).get_automaton_handle () == handle) {
  	  return true;
  	}
      }
      return false;
    }
    
    template <class T>
    void add_input (const T& input_action) {
      InputAction* ia = new T (input_action);
      list_type temp;
      temp.push_back (ia);
      m_input_actions.merge (temp, action_compare ());
    }

    bool empty () const {
      return m_input_actions.empty ();
    }

    void decompose (const input_action_interface& input_action) {
      typename list_type::iterator pos;
      for (pos = m_input_actions.begin ();
	   pos != m_input_actions.end ();
	   ++pos) {
      	if((*(*pos)) == input_action && (*(*pos)).get_owner_handle () == input_action.get_owner_handle ()) {
      	  break;
      	}
      }

      if (pos != m_input_actions.end ()) {
      	(*pos)->decompose ();
      	delete (*pos);
      	m_input_actions.erase (pos);
      }
    }

    void decompose (const generic_parameter_handle& handle) {
      if (m_output_action->get_automaton_handle () == handle &&
	  m_output_action->involves_parameter (handle.get_parameter ())) {
	typename list_type::const_iterator pos;
	for (pos = m_input_actions.begin ();
	     pos != m_input_actions.end ();
	     ++pos) {
	  (*pos)->decompose ();
	  delete (*pos);
	}
	m_input_actions.clear ();
      }
      else {
	typename list_type::iterator pos = m_input_actions.begin ();
	while (pos != m_input_actions.end ()) {
	  if((*pos)->get_automaton_handle () == handle &&
	     (*pos)->involves_parameter (handle.get_parameter ())) {
	    (*pos)->decompose ();
	    delete (*pos);
	    pos = m_input_actions.erase (pos);
	  }
	  else {
	    ++pos;
	  }
	}
      }
    }

    void decompose (const generic_automaton_handle& handle) {
      if (m_output_action->get_automaton_handle () == handle) {
	typename list_type::const_iterator pos;
	for (pos = m_input_actions.begin ();
	     pos != m_input_actions.end ();
	     ++pos) {
	  (*pos)->decompose ();
	  delete (*pos);
	}
	m_input_actions.clear ();
      }
      else {
	typename list_type::iterator pos = m_input_actions.begin ();
	while (pos != m_input_actions.end ()) {
	  if((*pos)->get_automaton_handle () == handle ||
	     (*pos)->get_owner_handle () == handle) {
	    (*pos)->decompose ();
	    delete (*pos);
	    pos = m_input_actions.erase (pos);
	  }
	  else {
	    ++pos;
	  }
	}
      }
    }

    virtual void execute_core () const = 0;

    void execute () const {
      // Acquire locks (in order).
      bool locked_output = false;
      generic_automaton_handle ao = m_output_action->get_automaton_handle ();
      
      for(typename list_type::const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	generic_automaton_handle ai = (*pos)->get_automaton_handle ();
  	if(!locked_output && ao < ai) {
  	  ao.get_automaton ()->lock();
  	  locked_output = true;
  	}
  	ai.get_automaton ()->lock();
      }

      // Execute.
      execute_core ();
      
      // Release locks.
      locked_output = false;
      for(typename list_type::const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	generic_automaton_handle ai = (*pos)->get_automaton_handle ();
  	if(!locked_output && ao < ai) {
  	  ao.get_automaton ()->unlock();
  	  locked_output = true;
  	}
  	ai.get_automaton ()->unlock();
      }
    }
  };

  class untyped_macro_action :
    public macro_action_base<untyped_output_action_interface, untyped_input_action_interface> {

  public:
    template <class T>
    untyped_macro_action (const T& output) :
      macro_action_base<untyped_output_action_interface, untyped_input_action_interface> (output)
    { }

    void execute_core () const {
      scheduler_interface& scheduler = get_scheduler ();
      scheduler.set_current_automaton (m_output_action->get_automaton_handle ().get_automaton ());
      bool t = (*m_output_action) ();
      if (t) {
	for(list_type::const_iterator pos = m_input_actions.begin();
	    pos != m_input_actions.end();
	    ++pos) {
	  scheduler.set_current_automaton ((*pos)->get_automaton_handle ().get_automaton ());
	  (*(*pos)) ();
	}
      }
      scheduler.set_current_automaton (0);
    }

  };

  template <class T>
  class typed_macro_action :
    public macro_action_base<typed_output_action_interface<T>, typed_input_action_interface<T> > {
  public:
    template <class U>
    typed_macro_action (const U& output) :
      macro_action_base<typed_output_action_interface<T>, typed_input_action_interface<T> > (output)
    { }

    void execute_core () const {
      scheduler_interface& scheduler = get_scheduler ();
      scheduler.set_current_automaton (this->m_output_action->get_automaton_handle ().get_automaton ());
      std::pair<bool, T> t = (*this->m_output_action) ();
      if (t.first) {
	for (typename macro_action_base<typed_output_action_interface<T>, typed_input_action_interface<T> >::list_type::const_iterator pos = this->m_input_actions.begin();
	     pos != this->m_input_actions.end();
	     ++pos) {
	  scheduler.set_current_automaton ((*pos)->get_automaton_handle ().get_automaton ());
	  (*(*pos)) (t.second);
	}
      }
      scheduler.set_current_automaton (0);
    }

  };

}

#endif
