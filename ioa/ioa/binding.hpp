#ifndef __binding_hpp__
#define __binding_hpp__

#include <boost/foreach.hpp>
#include "action.hpp"
#include "system_interface.hpp"

namespace ioa {

  template <class OA, class IA>
  class binding_record_interface
  {
  public:
    virtual ~binding_record_interface () { }
    virtual OA& output_action () = 0;
    virtual IA& input_action () = 0;
    virtual generic_automaton_handle binder () const = 0;
  };

  class default_unbind_success_listener
  {
  public:
    template <class OM, class IM>
    void unbound (const action<OM>& output_action,
		  const action<IM>& input_action,
		  const generic_automaton_handle& binder) { }
  };

  template <class OA, class IA, class OM, class IM, class USL = default_unbind_success_listener>
  class binding_record :
    public binding_record_interface<OA, IA>
  {
  private:
    action<OM> m_output_action;
    action<IM> m_input_action;
    generic_automaton_handle m_binder;
    default_unbind_success_listener m_default;
    USL& m_usl;

  public:
    binding_record (const action<OM>& output_action,
		    const action<IM>& input_action,
		    const generic_automaton_handle& binder) :
      m_output_action (output_action),
      m_input_action (input_action),
      m_binder (binder),
      m_usl (m_default)
    { }

    binding_record (const action<OM>& output_action,
		    const action<IM>& input_action,
		    const generic_automaton_handle& binder,
		    USL& usl) :
      m_output_action (output_action),
      m_input_action (input_action),
      m_binder (binder),
      m_usl (usl)
    { }

    ~binding_record () {
      m_usl.unbound (m_output_action, m_input_action, m_binder);
    }

    OA& output_action () {
      return m_output_action;
    }

    IA& input_action () {
      return m_input_action;
    }

    generic_automaton_handle binder () const {
      return m_binder;
    }
  };

  template <class OA, class IA>
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
    
    bool operator() (binding_record_interface<OA, IA>* const& x) const {
      return m_input.get_automaton_handle () == x->input_action ().get_automaton_handle ()
	&& m_input.get_member_ptr () == x->input_action ().get_member_ptr ()
	&& m_handle == x->binder ();
    }
  };
  
  template <class OA, class IA>
  class input_equal
  {
  private:
    const input_action_interface& m_input;
    
  public:
    input_equal (const input_action_interface& i) :
      m_input (i)
    { }
    
    bool operator() (binding_record_interface<OA, IA>* const& x) const {
      return m_input.get_automaton_handle () == x->input_action ().get_automaton_handle () &&
	m_input.get_member_ptr () == x->input_action ().get_member_ptr ();
    }
  };
  
  template <class OA, class IA>
  class input_automaton_equal
  {
  private:
    generic_automaton_handle m_automaton;
    
  public:
    input_automaton_equal (const generic_automaton_handle& automaton) :
      m_automaton (automaton)
    { }
    
    bool operator() (binding_record_interface<OA, IA>* const& x) const {
      return m_automaton == x->input_action ().get_automaton_handle ();
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
				   const generic_parameter_handle& parameter) = 0;
    virtual void unbind_automaton (const generic_automaton_handle& automaton) = 0;
  };
  

  template <class OA, class IA>
  struct action_compare
  {
    bool operator() (binding_record_interface<OA, IA>* const& x,
		     binding_record_interface<OA, IA>* const& y) const {
      return x->input_action ().get_automaton_handle () < y->input_action ().get_automaton_handle ();
    }
  };

  template <class OA, class IA>
  class generic_binding_impl :
    public binding_interface
  {
  protected:
    typedef std::set<binding_record_interface<OA, IA>*, action_compare<OA, IA> > set_type;
    set_type m_inputs;
    
  public:
    ~generic_binding_impl () {
      for (typename set_type::iterator pos = m_inputs.begin ();
	   pos != m_inputs.end ();
	   ++pos) {
  	delete (*pos);
      }
    }

  protected:
    OA& get_output () const {
      BOOST_ASSERT (!m_inputs.empty ());
      return (*(m_inputs.begin ()))->output_action ();
    }

  public:
    bool involves_output (const output_action_interface& output) const {
      if (!m_inputs.empty ()) {
	return get_output () == output;
      }
      else {
	return false;
      }
    }
    
    bool involves_input (const input_action_interface& input,
			 const generic_automaton_handle& binder) const {
      return std::find_if (m_inputs.begin (),
			   m_inputs.end (),
			   input_and_binder_equal<OA, IA> (input, binder)) != m_inputs.end ();
    }

    bool involves_input (const input_action_interface& input) const {
      return std::find_if (m_inputs.begin (),
			   m_inputs.end (),
			   input_equal<OA, IA> (input)) != m_inputs.end ();
    }
    
    bool
    involves_input_automaton (const generic_automaton_handle& automaton) const
    {
      return std::find_if (m_inputs.begin (),
  			   m_inputs.end (),
  			   input_automaton_equal<OA, IA> (automaton)) != m_inputs.end ();
    }
    
    template <class OM, class IM, class USL>
    void bind (const action<OM>& output_action,
	       const action<IM>& input_action,
	       const generic_automaton_handle& binder,
	       USL& usl) {
      if (!m_inputs.empty ()) {
	BOOST_ASSERT (output_action == get_output ());
      }
      binding_record_interface<OA, IA>* record = new binding_record<OA, IA, OM, IM, USL> (output_action, input_action, binder, usl);
      m_inputs.insert (record);
    }

    template <class OM, class IM>
    void unbind (const action<OM>& output_action,
		 const action<IM>& input_action,
		 const generic_automaton_handle& binder) {
      BOOST_ASSERT (!m_inputs.empty ());
      binding_record<OA, IA, OM, IM> t (output_action, input_action, binder);
      typename set_type::iterator pos = m_inputs.find (&t);
      if (pos != m_inputs.end ()) {
	binding_record_interface<OA, IA>* record = *pos;
	m_inputs.erase (pos);
	delete record;
      }
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
	if (!output_processed &&
	    (*pos)->output_action ().get_automaton_handle () < (*pos)->input_action ().get_automaton_handle ()) {
	  system.lock_automaton ((*pos)->output_action ().get_automaton_handle ());
	  output_processed = true;
	}
	system.lock_automaton ((*pos)->input_action ().get_automaton_handle ());
      }

      // Execute.
      execute_dispatch (scheduler);

      // Unlock.
      output_processed = false;
      for (typename set_type::iterator pos = m_inputs.begin ();
	   pos != m_inputs.end ();
	   ++pos) {
	if (!output_processed &&
	    (*pos)->output_action ().get_automaton_handle () < (*pos)->input_action ().get_automaton_handle ()) {
	  system.unlock_automaton ((*pos)->output_action ().get_automaton_handle ());
	  output_processed = true;
	}
	system.unlock_automaton ((*pos)->input_action ().get_automaton_handle ());
      }

    }

    void unbind_parameter (const generic_automaton_handle& automaton,
			   const generic_parameter_handle& parameter) {
      if (get_output ().get_automaton_handle () == automaton &&
	  get_output ().involves_parameter (parameter)) {
	// Unbind all.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  delete (*pos);
	}
	m_inputs.clear ();
      }
      else {
	// Try the inputs.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  if ((*pos)->input_action ().get_automaton_handle () == automaton &&
	      (*pos)->input_action ().involves_parameter (parameter)) {
	    delete (*pos);
	    m_inputs.erase (pos);
	    break;
	  }
	}
      }
    }
    
    void unbind_automaton (const generic_automaton_handle& automaton) {
      if (get_output ().get_automaton_handle () == automaton) {
	// Unbind all.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  delete (*pos);
	}
	m_inputs.clear ();
      }
      else {
	// Try the inputs.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  if ((*pos)->input_action ().get_automaton_handle () == automaton ||
	      (*pos)->binder () == automaton) {
	    delete (*pos);
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
  protected:
    typedef generic_binding_impl<unvalued_output_action_interface, unvalued_input_action_interface>::set_type set_type;

    void execute_dispatch (scheduler_interface& scheduler) {
      scheduler.set_current_handle (get_output ().get_automaton_handle ());
      if ((get_output ()) ()) {
	for (set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  scheduler.set_current_handle ((*pos)->input_action ().get_automaton_handle ());
	  ((*pos)->input_action ()) ();
	}
      }
      scheduler.clear_current_handle ();
    }
  };

  template <class T>
  class binding_impl<valued, T> :
    public generic_binding_impl<valued_output_action_interface<T>, valued_input_action_interface<T> >
  {
  protected:
    typedef typename generic_binding_impl<valued_output_action_interface<T>, valued_input_action_interface<T> >::set_type set_type;

    void execute_dispatch (scheduler_interface& scheduler) {
      scheduler.set_current_handle (this->get_output ().get_automaton_handle ());
      const std::pair<bool, T> p = (this->get_output ()) ();
      if (p.first) {
	for (typename set_type::iterator pos = this->m_inputs.begin ();
	     pos != this->m_inputs.end ();
	     ++pos) {
	  scheduler.set_current_handle ((*pos)->input_action ().get_automaton_handle ());
	  ((*pos)->input_action ()) (p.second);
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

  };

}

#endif
