#ifndef __binding_hpp__
#define __binding_hpp__

#include <boost/foreach.hpp>
#include "action.hpp"

namespace ioa {

  class scheduler_interface
  {
  public:
    virtual ~scheduler_interface () { }
    virtual void set_current_handle (const generic_automaton_handle& handle) = 0;
    virtual void clear_current_handle () = 0;
  };

  class system_interface
  {
  public:
    virtual ~system_interface () { }
    virtual void lock_automaton (const generic_automaton_handle& handle) = 0;
    virtual void unlock_automaton (const generic_automaton_handle& handle) = 0;
  };

  class rescind_listener_interface
  {
  public:
    virtual ~rescind_listener_interface () { }
    virtual void unbound (const generic_automaton_handle& output_automaton,
			  const void* output_member_ptr,
			  const generic_parameter_handle& output_parameter,
			  const generic_automaton_handle& input_automaton,
			  const void* input_member_ptr) = 0;
    virtual void unbound (const generic_automaton_handle& output_automaton,
			  const void* output_member_ptr,
			  const generic_automaton_handle& input_automaton,
			  const void* input_member_ptr,
			  const generic_parameter_handle& input_parameter) = 0;
  };
  
  class destroy_listener_interface
  {
  public:
    virtual ~destroy_listener_interface () { }
    virtual void destroyed (const generic_automaton_handle& parent,
			    const generic_automaton_handle& child) = 0;
    virtual void unbound (const generic_automaton_handle& output_automaton,
			  const void* output_member_ptr,
			  const generic_automaton_handle& input_automaton,
			  const void* input_member_ptr,
			  const generic_automaton_handle& binder_automaton) = 0;
    virtual void unbound (const generic_automaton_handle& output_automaton,
			  const void* output_member_ptr,
			  const generic_parameter_handle& output_parameter,
			  const generic_automaton_handle& input_automaton,
			  const void* input_member_ptr) = 0;
    virtual void unbound (const generic_automaton_handle& output_automaton,
			  const void* output_member_ptr,
			  const generic_automaton_handle& input_automaton,
			  const void* input_member_ptr,
			  const generic_parameter_handle& input_parameter) = 0;
  };
  
  class input_and_binder_equal
  {
  private:
    const input_action_interface& m_input;
    const generic_automaton_handle m_handle;
    
  public:
    input_and_binder_equal (const input_action_interface& i,
			    const generic_automaton_handle& handle) :
      m_input (i),
      m_handle (handle)
    { }
    
    bool operator() (const std::pair<input_action_interface*, generic_automaton_handle>& i) const {
      return m_input.get_automaton_handle () == i.first->get_automaton_handle ()
	&& m_input.get_member_ptr () == i.first->get_member_ptr ()
	&& m_handle == i.second;
    }
  };
  
  class input_equal
  {
  private:
    const input_action_interface& m_input;
    
  public:
    input_equal (const input_action_interface& i) :
      m_input (i)
    { }
    
    bool operator() (const std::pair<input_action_interface*, generic_automaton_handle>& i) const {
      return m_input.get_automaton_handle () == i.first->get_automaton_handle () &&
	m_input.get_member_ptr () == i.first->get_member_ptr ();
    }
  };
  
  class input_automaton_equal
  {
  private:
    generic_automaton_handle m_automaton;
    
  public:
    input_automaton_equal (const generic_automaton_handle& automaton) :
      m_automaton (automaton)
    { }
    
    bool operator() (const std::pair<input_action_interface*, generic_automaton_handle>& i) const {
      return m_automaton == i.first->get_automaton_handle ();
    }
  };
  
  class binding_interface
  {
  public:
    virtual ~binding_interface () { }
    virtual bool involves_output (const output_action_interface& output) const = 0;
    virtual bool involves_input (const input_action_interface& input,
  				 const generic_automaton_handle& binder) const = 0;
    virtual bool involves_input (const input_action_interface& input) const = 0;

    virtual bool involves_input_automaton (const generic_automaton_handle& automaton) const = 0;
    virtual bool empty () const = 0;
    virtual void execute (scheduler_interface& scheduler, system_interface& system) = 0;
    virtual void unbind_parameter (const generic_automaton_handle& automaton,
				   const generic_parameter_handle& parameter,
				   rescind_listener_interface& listener) = 0;
    virtual void unbind_automaton (const generic_automaton_handle& automaton,
				   destroy_listener_interface& listener) = 0;
  };
  
  struct action_compare
  {
    bool operator() (const std::pair<action_interface*, generic_automaton_handle>& x,
		     const std::pair<action_interface*, generic_automaton_handle>& y) const {
      return x.first->get_automaton_handle () < y.first->get_automaton_handle ();
    }
  };

  template <class OA, class IA>
  class generic_binding_impl :
    public binding_interface
  {
  protected:
    OA* m_output;
    typedef std::set<std::pair<IA*, generic_automaton_handle>, action_compare> set_type;
    set_type m_inputs;
    
  public:
    template <class T>
    generic_binding_impl (const action<T>& output) :
      m_output (new action<T> (output))
    { }
    
    ~generic_binding_impl () {
      delete m_output;
      for (typename set_type::iterator pos = m_inputs.begin ();
	   pos != m_inputs.end ();
	   ++pos) {
  	delete pos->first;
      }
    }
    
    bool involves_output (const output_action_interface& output) const {
      return *m_output == output;
    }
    
    bool involves_input (const input_action_interface& input,
			 const generic_automaton_handle& binder) const {
      return std::find_if (m_inputs.begin (),
			   m_inputs.end (),
			   input_and_binder_equal (input, binder)) != m_inputs.end ();
    }

    bool involves_input (const input_action_interface& input) const {
      return std::find_if (m_inputs.begin (),
			   m_inputs.end (),
			   input_equal (input)) != m_inputs.end ();
    }
    
    bool
    involves_input_automaton (const generic_automaton_handle& automaton) const
    {
      return std::find_if (m_inputs.begin (),
  			   m_inputs.end (),
  			   input_automaton_equal (automaton)) != m_inputs.end ();
    }
    
    template <class T>
    void bind (const T& input,
	       const generic_automaton_handle& binder) {
      IA* ia = new T (input);
      m_inputs.insert (std::make_pair (ia, binder));
    }

    template <class T>
    void unbind (const T& input,
		 const generic_automaton_handle& binder) {
      T t (input);
      m_inputs.erase (std::make_pair (&t, binder));
    }

    bool empty () const {
      return m_inputs.empty ();
    }

    void execute (scheduler_interface& scheduler,
		  system_interface& system) {
      bool output_processed;

      // Lock in order.
      output_processed = false;
      for (typename set_type::iterator pos = m_inputs.begin ();
	   pos != m_inputs.end ();
	   ++pos) {
	if (!output_processed && m_output->get_automaton_handle () < pos->first->get_automaton_handle ()) {
	  system.lock_automaton (m_output->get_automaton_handle ());
	  output_processed = true;
	}
	system.lock_automaton (pos->first->get_automaton_handle ());
      }

      // Execute.
      execute_dispatch (scheduler);

      // Unlock.
      output_processed = false;
      for (typename set_type::iterator pos = m_inputs.begin ();
	   pos != m_inputs.end ();
	   ++pos) {
	if (!output_processed && m_output->get_automaton_handle () < pos->first->get_automaton_handle ()) {
	  system.unlock_automaton (m_output->get_automaton_handle ());
	  output_processed = true;
	}
	system.unlock_automaton (pos->first->get_automaton_handle ());
      }

    }

    void unbind_parameter (const generic_automaton_handle& automaton,
			      const generic_parameter_handle& parameter,
			      rescind_listener_interface& listener) {
      if (m_output->get_automaton_handle () == automaton &&
	  m_output->involves_parameter (parameter)) {
	// Unbind all.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  listener.unbound (m_output->get_automaton_handle (),
			    m_output->get_member_ptr (),
			    parameter,
			    pos->first->get_automaton_handle (),
			    pos->first->get_member_ptr ());
	  delete pos->first;
	}
	m_inputs.clear ();
      }
      else {
	// Try the inputs.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  if (pos->first->get_automaton_handle () == automaton &&
	      pos->first->involves_parameter (parameter)) {
	    listener.unbound (m_output->get_automaton_handle (),
			      m_output->get_member_ptr (),
			      pos->first->get_automaton_handle (),
			      pos->first->get_member_ptr (),
			      parameter);
	    delete pos->first;
	    m_inputs.erase (pos);
	    break;
	  }
	}
      }
    }
    
    void unbind_automaton (const generic_automaton_handle& automaton,
			      destroy_listener_interface& listener) {
      if (m_output->get_automaton_handle () == automaton) {
	// Unbind all.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  if (m_output->is_parameterized ()) {
	    listener.unbound (m_output->get_automaton_handle (),
			      m_output->get_member_ptr (),
			      m_output->get_parameter_handle (),
			      pos->first->get_automaton_handle (),
			      pos->first->get_member_ptr ());
	  }
	  else {
	    listener.unbound (m_output->get_automaton_handle (),
			      m_output->get_member_ptr (),
			      pos->first->get_automaton_handle (),
			      pos->first->get_member_ptr (),
			      pos->second);
	  }
	  delete pos->first;
	}
	m_inputs.clear ();
      }
      else {
	// Try the inputs.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  if (pos->first->get_automaton_handle () == automaton ||
	      pos->second == automaton) {
	    if (pos->first->is_parameterized ()) {
	      listener.unbound (m_output->get_automaton_handle (),
				m_output->get_member_ptr (),
				pos->first->get_automaton_handle (),
				pos->first->get_member_ptr (),
				pos->first->get_parameter_handle ());
	    }
	    else {
	      listener.unbound (m_output->get_automaton_handle (),
				m_output->get_member_ptr (),
				pos->first->get_automaton_handle (),
				pos->first->get_member_ptr (),
				pos->second);
	    }
	    delete pos->first;
	    m_inputs.erase (pos);
	    break;
	  }
	}
      }
    }

  protected:
    virtual void execute_dispatch (scheduler_interface&) = 0;
  };
  
  template <class VS, class VT> class binding_impl;

  template <>
  class binding_impl<unvalued, null_type> :
    public generic_binding_impl<unvalued_output_action_interface, unvalued_input_action_interface>
  {
  public:
    template <class Action>
    binding_impl (const Action& action) :
      generic_binding_impl<unvalued_output_action_interface, unvalued_input_action_interface> (action)
    { }

  protected:
    typedef generic_binding_impl<unvalued_output_action_interface, unvalued_input_action_interface>::set_type set_type;

    void execute_dispatch (scheduler_interface& scheduler) {
      scheduler.set_current_handle (m_output->get_automaton_handle ());
      if ((*m_output) ()) {
	for (set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  scheduler.set_current_handle (pos->first->get_automaton_handle ());
	  (*(pos->first)) ();
	}
      }
      scheduler.clear_current_handle ();
    }
  };

  template <class T>
  class binding_impl<valued, T> :
    public generic_binding_impl<valued_output_action_interface<T>, valued_input_action_interface<T> >
  {
  public:
    template <class Action>
    binding_impl (const Action& action) :
      generic_binding_impl<valued_output_action_interface<T>, valued_input_action_interface<T> > (action)
    { }

  protected:
    typedef typename generic_binding_impl<valued_output_action_interface<T>, valued_input_action_interface<T> >::set_type set_type;

    void execute_dispatch (scheduler_interface& scheduler) {
      scheduler.set_current_handle (this->m_output->get_automaton_handle ());
      const std::pair<bool, T> p = (*(this->m_output)) ();
      if (p.first) {
	for (typename set_type::iterator pos = this->m_inputs.begin ();
	     pos != this->m_inputs.end ();
	     ++pos) {
	  scheduler.set_current_handle (pos->first->get_automaton_handle ());
	  (*(pos->first)) (p.second);
	}
      }
      scheduler.clear_current_handle ();
    }

  };
  
  template <class Member>
  class binding :
    public binding_impl<typename Member::value_status,
			    typename Member::value_type>
  {
  public:
    typedef typename Member::value_status value_status;
    typedef typename Member::value_type value_type;

    binding (const action<Member>& action) :
      binding_impl<value_status, value_type> (action)
    { }

  };

}

#endif
