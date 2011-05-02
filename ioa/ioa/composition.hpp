#ifndef __composition_hpp__
#define __composition_hpp__

#include <boost/foreach.hpp>
#include "action.hpp"

namespace ioa {

  class decompose_listener_interface
  {
  public:
    virtual ~decompose_listener_interface () { }
    virtual void decomposed (const generic_automaton_handle& output_automaton,
			     const void* output_member_ptr,
			     const generic_parameter_handle& output_parameter,
			     const generic_automaton_handle& input_automaton,
			     const void* input_member_ptr) = 0;
    virtual void decomposed (const generic_automaton_handle& output_automaton,
			     const void* output_member_ptr,
			     const generic_automaton_handle& input_automaton,
			     const void* input_member_ptr,
			     const generic_parameter_handle& input_parameter) = 0;
  };

  class input_equal
  {
  private:
    const input_action_interface& m_input;
    
  public:
    input_equal (const input_action_interface& i) :
      m_input (i)
    { }
    
    bool operator() (const input_action_interface* i) const {
      return m_input.get_automaton_handle () == i->get_automaton_handle () &&
	m_input.get_member_ptr () == i->get_member_ptr () &&
	m_input.get_composer_handle () == i->get_composer_handle ();
    }
  };
  
  class input_equal_no_composer
  {
  private:
    const input_action_interface& m_input;
    
  public:
    input_equal_no_composer (const input_action_interface& i) :
      m_input (i)
    { }
    
    bool operator() (const input_action_interface* i) const {
      return m_input.get_automaton_handle () == i->get_automaton_handle () &&
	m_input.get_member_ptr () == i->get_member_ptr ();
    }
  };

  class input_equal_automaton_only
  {
  private:
    generic_automaton_handle m_automaton;
    
  public:
    input_equal_automaton_only (const generic_automaton_handle& automaton) :
      m_automaton (automaton)
    { }
    
    bool operator() (const input_action_interface* i) const {
      return m_automaton == i->get_automaton_handle ();
    }
  };
  
  class composition_interface
  {
  public:
    virtual ~composition_interface () { }
    virtual bool involves_output (const output_action_interface& output) const = 0;
    virtual bool involves_input (const input_action_interface& input,
  				 bool check_owner) const = 0;
    virtual bool involves_input_automaton (const generic_automaton_handle& automaton) const = 0;
    virtual bool empty () const = 0;
    virtual void execute () = 0;
    virtual void decompose_parameter (const generic_automaton_handle& automaton,
				      const generic_parameter_handle& parameter,
				      decompose_listener_interface& listener) = 0;
  };
  
  struct action_compare
  {
    bool operator() (const action_interface* x, const action_interface* y) const {
      return x->get_automaton_handle () < y->get_automaton_handle ();
    }
  };

  template <class OA, class IA>
  class generic_composition_impl :
    public composition_interface
  {
  protected:
    OA* m_output;
    typedef std::set<IA*, action_compare> set_type;
    set_type m_inputs;
    
  public:
    template <class T>
    generic_composition_impl (const action<T>& output) :
      m_output (new action<T> (output))
    { }
    
    ~generic_composition_impl () {
      delete m_output;
      BOOST_FOREACH (IA* p, m_inputs) {
  	delete p;
      }
    }
    
    bool involves_output (const output_action_interface& output) const {
      return *m_output == output;
    }
    
    bool involves_input (const input_action_interface& input,
  			 bool check_owner) const {
      if (check_owner) {
  	return std::find_if (m_inputs.begin (),
  			     m_inputs.end (),
  			     input_equal (input)) != m_inputs.end ();
      }
      else {
  	return std::find_if (m_inputs.begin (),
  			     m_inputs.end (),
  			     input_equal_no_composer (input)) != m_inputs.end ();
      }
    }
    
    bool
    involves_input_automaton (const generic_automaton_handle& automaton) const
    {
      return std::find_if (m_inputs.begin (),
  			   m_inputs.end (),
  			   input_equal_automaton_only (automaton)) != m_inputs.end ();
    }
    
    template <class T>
    void compose (const T& input) {
      m_inputs.insert (new T (input));
    }

    void decompose (IA& input) {
      m_inputs.erase (&input);
    }

    bool empty () const {
      return m_inputs.empty ();
    }

    void execute () {
      bool output_processed;

      // Lock in order.
      output_processed = false;
      BOOST_FOREACH (IA* i, m_inputs) {
	if (!output_processed && m_output->get_automaton_handle () < i->get_automaton_handle ()) {
	  m_output->lock_automaton ();
	  output_processed = true;
	}
	i->lock_automaton ();
      }

      // Execute.
      execute_dispatch ();

      // Unlock.
      output_processed = false;
      BOOST_FOREACH (IA* i, m_inputs) {
	if (!output_processed && m_output->get_automaton_handle () < i->get_automaton_handle ()) {
	  m_output->unlock_automaton ();
	  output_processed = true;
	}
	i->unlock_automaton ();
      }

    }

    void decompose_parameter (const generic_automaton_handle& automaton,
			      const generic_parameter_handle& parameter,
			      decompose_listener_interface& listener) {
      if (m_output->get_automaton_handle () == automaton &&
	  m_output->involves_parameter (parameter)) {
	// Decompose all.
	BOOST_FOREACH (IA* i, m_inputs) {
	  listener.decomposed (m_output->get_automaton_handle (),
			       m_output->get_member_ptr (),
			       parameter,
			       i->get_automaton_handle (),
			       i->get_member_ptr ());
	  delete i;
	}
	m_inputs.clear ();
      }
      else {
	// Try the inputs.
	for (typename set_type::iterator pos = m_inputs.begin ();
	     pos != m_inputs.end ();
	     ++pos) {
	  if ((*pos)->get_automaton_handle () == automaton &&
	      (*pos)->involves_parameter (parameter)) {
	    listener.decomposed (m_output->get_automaton_handle (),
				 m_output->get_member_ptr (),
				 (*pos)->get_automaton_handle (),
				 (*pos)->get_member_ptr (),
				 parameter);
	    delete *pos;
	    m_inputs.erase (pos);
	    break;
	  }
	}
      }
    }

  protected:
    virtual void execute_dispatch () = 0;
  };
  
  template <class VS, class VT> class composition_impl;

  template <>
  class composition_impl<unvalued, null_type> :
    public generic_composition_impl<unvalued_output_action_interface, unvalued_input_action_interface>
  {
  public:
    template <class Action>
    composition_impl (const Action& action) :
      generic_composition_impl<unvalued_output_action_interface, unvalued_input_action_interface> (action)
    { }

  protected:
    void execute_dispatch () {
      if ((*m_output) ()) {
	BOOST_FOREACH (unvalued_input_action_interface* i, m_inputs) {
	  (*i) ();
	}
      }
    }
  };

  template <class T>
  class composition_impl<valued, T> :
    public generic_composition_impl<valued_output_action_interface<T>, valued_input_action_interface<T> >
  {
  public:
    template <class Action>
    composition_impl (const Action& action) :
      generic_composition_impl<valued_output_action_interface<T>, valued_input_action_interface<T> > (action)
    { }

  protected:
    void execute_dispatch () {
      const std::pair<bool, T> p = (*(this->m_output)) ();
      if (p.first) {
       	BOOST_FOREACH (valued_input_action_interface<T>* i, this->m_inputs) {
       	  (*i) (p.second);
       	}
      }
    }

  };
  
  template <class Member>
  class composition :
    public composition_impl<typename Member::value_status,
			    typename Member::value_type>
  {
  public:
    typedef typename Member::value_status value_status;
    typedef typename Member::value_type value_type;

    composition (const action<Member>& action) :
      composition_impl<value_status, value_type> (action)
    { }

  };

}

#endif
