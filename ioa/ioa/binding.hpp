#ifndef __binding_hpp__
#define __binding_hpp__

#include <boost/foreach.hpp>
#include "action.hpp"
#include "system_interface.hpp"
#include "scheduler_interface.hpp"

namespace ioa {

  template <class OA, class IA>
  class binding_record_interface
  {
  public:
    virtual ~binding_record_interface () { }
    virtual OA& output_action () = 0;
    virtual IA& input_action () = 0;
    virtual aid_t binder () const = 0;
  };

  class default_unbind_success_listener
  {
  public:
    template <class OI, class OM, class II, class IM, class I, class D>
    void unbound (const action<OI, OM>& output_action,
		  const action<II, IM>& input_action,
		  const automaton_handle<I>& binder,
		  D&) { }
  };

  struct empty_d { };

  template <class OA, class IA, class OI, class OM, class II, class IM, class I, class USL = default_unbind_success_listener, class D = empty_d>
  class binding_record :
    public binding_record_interface<OA, IA>
  {
  private:
    concrete_action<OI, OM> m_output_action;
    concrete_action<II, IM> m_input_action;
    automaton_handle<I> m_binder;
    default_unbind_success_listener m_default_usl;
    USL& m_usl;
    empty_d m_default_d;
    D& m_d;

  public:
    binding_record (const concrete_action<OI, OM>& output_action,
		    const concrete_action<II, IM>& input_action,
		    const automaton_handle<I>& binder) :
      m_output_action (output_action),
      m_input_action (input_action),
      m_binder (binder),
      m_usl (m_default_usl),
      m_d (m_default_d)
    { }

    binding_record (const concrete_action<OI, OM>& output_action,
		    const concrete_action<II, IM>& input_action,
		    const automaton_handle<I>& binder,
		    USL& usl,
		    D& d) :
      m_output_action (output_action),
      m_input_action (input_action),
      m_binder (binder),
      m_usl (usl),
      m_d (d)
    { }

    ~binding_record () {
      m_usl.unbound (m_output_action, m_input_action, m_binder, m_d);
    }

    OA& output_action () {
      return m_output_action;
    }

    IA& input_action () {
      return m_input_action;
    }

    aid_t binder () const {
      return m_binder.aid ();
    }
  };

  template <class OA, class IA>
  class binding_equal
  {
  private:
    const output_action_interface& m_output;
    const input_action_interface& m_input;
    const aid_t m_binder;
    
  public:
    binding_equal (const output_action_interface& o,
		   const input_action_interface& i,
		   const aid_t binder) :
      m_output (o),
      m_input (i),
      m_binder (binder)
    { }
    
    bool operator() (binding_record_interface<OA, IA>* const& x) const {
      return m_output == x->output_action () &&
	m_input == x->input_action ()
	&& m_binder == x->binder ();
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
      return m_input == x->input_action ();
    }
  };
  
  template <class OA, class IA>
  class input_automaton_equal
  {
  private:
    aid_t m_automaton;
    
  public:
    input_automaton_equal (const aid_t automaton) :
      m_automaton (automaton)
    { }
    
    bool operator() (binding_record_interface<OA, IA>* const& x) const {
      return m_automaton == x->input_action ().get_aid ();
    }
  };
  
  class binding_interface
  {
  public:
    virtual ~binding_interface () { }
    virtual bool involves_output (const output_action_interface& output) const = 0;
    virtual bool involves_binding (const output_action_interface& output,
				   const input_action_interface& input,
				   const aid_t binder) const = 0;
    virtual bool involves_input (const input_action_interface& input) const = 0;

    virtual bool involves_input_automaton (const aid_t automaton) const = 0;
    virtual bool empty () const = 0;
    virtual void execute (scheduler_interface& scheduler, system_interface& system) = 0;
    virtual void unbind_parameter (const aid_t automaton,
				   const pid_t parameter) = 0;
    virtual void unbind_automaton (const aid_t automaton) = 0;
  };
  

  template <class OA, class IA>
  struct action_compare
  {
    bool operator() (binding_record_interface<OA, IA>* const& x,
		     binding_record_interface<OA, IA>* const& y) const {
      return x->input_action ().get_aid () < y->input_action ().get_aid ();
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
    
    bool involves_binding (const output_action_interface& output,
			   const input_action_interface& input,
			   const aid_t binder) const {
      return std::find_if (m_inputs.begin (),
			   m_inputs.end (),
			   binding_equal<OA, IA> (output, input, binder)) != m_inputs.end ();
    }
    
    bool involves_input (const input_action_interface& input) const {
      return std::find_if (m_inputs.begin (),
			   m_inputs.end (),
			   input_equal<OA, IA> (input)) != m_inputs.end ();
    }
    
    bool
    involves_input_automaton (const aid_t automaton) const
    {
      return std::find_if (m_inputs.begin (),
  			   m_inputs.end (),
  			   input_automaton_equal<OA, IA> (automaton)) != m_inputs.end ();
    }
    
    template <class OI, class OM, class II, class IM, class I, class USL, class D>
    void bind (const concrete_action<OI, OM>& output_action,
	       const concrete_action<II, IM>& input_action,
	       const automaton_handle<I>& binder,
	       USL& usl,
	       D& d) {
      // Can't bind to self.
      BOOST_ASSERT (output_action.automaton.aid () != input_action.automaton.aid ());
      // Can't already involve input.
      BOOST_ASSERT (!involves_input_automaton (input_action.automaton.aid ()));
      // Sanity check.
      if (!m_inputs.empty ()) {
	BOOST_ASSERT (output_action == get_output ());
      }

      binding_record_interface<OA, IA>* record = new binding_record<OA, IA, OI, OM, II, IM, I, USL, D> (output_action, input_action, binder, usl, d);
      m_inputs.insert (record);
    }

    template <class OI, class OM, class II, class IM, class I>
    void unbind (const concrete_action<OI, OM>& output_action,
		 const concrete_action<II, IM>& input_action,
		 const automaton_handle<I>& binder) {
      BOOST_ASSERT (!m_inputs.empty ());
      binding_record<OA, IA, OI, OM, II, IM, I> t (output_action, input_action, binder);
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
	    (*pos)->output_action ().get_aid () < (*pos)->input_action ().get_aid ()) {
	  system.lock_automaton ((*pos)->output_action ().get_aid ());
	  output_processed = true;
	}
	system.lock_automaton ((*pos)->input_action ().get_aid ());
      }

      // Execute.
      execute_dispatch (scheduler);

      // Unlock.
      output_processed = false;
      for (typename set_type::iterator pos = m_inputs.begin ();
	   pos != m_inputs.end ();
	   ++pos) {
	if (!output_processed &&
	    (*pos)->output_action ().get_aid () < (*pos)->input_action ().get_aid ()) {
	  system.unlock_automaton ((*pos)->output_action ().get_aid ());
	  output_processed = true;
	}
	system.unlock_automaton ((*pos)->input_action ().get_aid ());
      }

    }

    void unbind_parameter (const aid_t automaton,
			   const pid_t parameter) {
      if (get_output ().get_aid () == automaton &&
	  get_output ().get_pid () == parameter) {
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
	  if ((*pos)->input_action ().get_aid () == automaton &&
	      (*pos)->input_action ().get_pid () == parameter) {
	    delete (*pos);
	    m_inputs.erase (pos);
	    break;
	  }
	}
      }
    }
    
    void unbind_automaton (const aid_t automaton) {
      if (get_output ().get_aid () == automaton) {
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
	  if ((*pos)->input_action ().get_aid () == automaton ||
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
      scheduler.set_current_aid (get_output ().get_aid (), get_output ().get_instance ());
      if ((get_output ()) ()) {
	for (set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  scheduler.set_current_aid ((*pos)->input_action ().get_aid (), (*pos)->input_action ().get_instance ());
	  ((*pos)->input_action ()) ();
	}
      }
      scheduler.clear_current_aid ();
    }
  };

  template <class T>
  class binding_impl<valued, T> :
    public generic_binding_impl<valued_output_action_interface<T>, valued_input_action_interface<T> >
  {
  protected:
    typedef typename generic_binding_impl<valued_output_action_interface<T>, valued_input_action_interface<T> >::set_type set_type;

    void execute_dispatch (scheduler_interface& scheduler) {
      scheduler.set_current_aid (this->get_output ().get_aid (), this->get_output ().get_instance ());
      const std::pair<bool, T> p = (this->get_output ()) ();
      if (p.first) {
	for (typename set_type::iterator pos = this->m_inputs.begin ();
	     pos != this->m_inputs.end ();
	     ++pos) {
	  scheduler.set_current_aid ((*pos)->input_action ().get_aid (), (*pos)->input_action ().get_instance ());
	  ((*pos)->input_action ()) (p.second);
	}
      }
      scheduler.clear_current_aid ();
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
