#ifndef __composition_hpp__
#define __composition_hpp__

namespace ioa {

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
    virtual void execute () = 0;
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
    std::set<IA*, action_compare> m_inputs;
    
  public:
    template <class T>
    generic_composition_impl (const T& output) :
      m_output (new T (output))
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

    virtual void execute_dispatch () = 0;

    void execute () {
      bool output_processed;

      // Lock in order.
      output_processed = false;
      BOOST_FOREACH (IA* i, m_inputs) {
	// TODO:  Clean this up.
	if (!output_processed && m_output->get_automaton_handle () < i->get_automaton_handle ()) {
	  m_output->get_automaton_handle ().value()->lock ();
	  output_processed = true;
	}
	i->get_automaton_handle ().value ()->lock ();
      }

      // Execute.
      execute_dispatch ();

      // Unlock.
      output_processed = false;
      BOOST_FOREACH (IA* i, m_inputs) {
	// TODO:  Clean this up.
	if (!output_processed && m_output->get_automaton_handle () < i->get_automaton_handle ()) {
	  m_output->get_automaton_handle ().value()->unlock ();
	  output_processed = true;
	}
	i->get_automaton_handle ().value ()->unlock ();
      }

    }
    
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

    void execute_dispatch () {
      if ((*m_output) ()) {
	BOOST_FOREACH (unvalued_input_action_interface* i, m_inputs) {
	  (*i) ();
	}
      }
    }
  };
  
  template <class Action>
  class composition :
    public composition_impl<typename Action::value_status,
			    typename Action::value_type>
  {
  public:
    typedef typename Action::value_status value_status;
    typedef typename Action::value_type value_type;

    composition (const Action& action) :
      composition_impl<value_status, value_type> (action)
    { }

  };

}

#endif
