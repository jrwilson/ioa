#ifndef __system_hpp__
#define __system_hpp__

#include <boost/foreach.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <set>
#include <list>
#include "automaton.hpp"
#include "action.hpp"

namespace ioa {

  class system
  {
  private:    
    struct automata_instance_equal
    {
      void* instance;
      
      automata_instance_equal (void* instance) :
	instance (instance)
      { }
      
      bool operator() (const automaton* automaton) const {
	return instance == automaton->get_instance ();
      }
    };

  private:
    // struct output
    // {
    //   timestamp<automaton> automaton_ts;
    //   output_action* action;
    //   timestamp<void> parameter;

    //   output (const timestamp<automaton>& automaton_ts,
    // 	      output_action* action) :
    // 	automaton_ts (automaton_ts),
    // 	action (action)
    //   { }

    //   output (const timestamp<automaton>& automaton_ts,
    // 	      output_action* action,
    // 	      const timestamp<void>& parameter) :
    // 	automaton_ts (automaton_ts),
    // 	action (action),
    // 	parameter (parameter)
    //   { }

    //   bool operator== (const output& o) const {
    // 	return automaton_ts == o.automaton_ts && action == o.action;
    //   }

    //   bool parameter_exists () const {
    // 	if (parameter.valid ()) {
    // 	  return automaton_ts->parameter_exists (parameter);
    // 	}
    // 	else {
    // 	  return true;
    // 	}
    //   }
    // };

    // struct input
    // {
    //   timestamp<automaton> automaton_ts;
    //   input_action* action;
    //   timestamp<void> parameter;
    //   timestamp<automaton> composer;
      
    //   input (const timestamp<automaton>& automaton_ts,
    // 	     input_action* action,
    // 	     const timestamp<automaton>& composer) :
    // 	automaton_ts (automaton_ts),
    // 	action (action),
    // 	composer (composer)
    //   { }

    //   input (const timestamp<automaton>& automaton_ts,
    // 	     input_action* action,
    // 	     const timestamp<void>& parameter,
    // 	     const timestamp<automaton>& composer) :
    // 	automaton_ts (automaton_ts),
    // 	action (action),
    // 	parameter (parameter),
    // 	composer (composer)
    //   { }
      
    //   bool operator< (const input& i) const {
    // 	return automaton_ts.get () < i.automaton_ts.get ();
    //   }

    //   bool parameter_exists () const {
    // 	if (parameter.valid ()) {
    // 	  return automaton_ts->parameter_exists (parameter);
    // 	}
    // 	else {
    // 	  return true;
    // 	}
    //   }
    // };

    class input_equal
    {
    private:
      const input_action_interface& m_input;

    public:
      input_equal (const input_action_interface& i) :
	m_input (i)
      { }

      bool operator() (const input_action_interface& i) const {
	return m_input.get_automaton_ts () == i.get_automaton_ts () &&
	  m_input.get_member_ptr () == i.get_member_ptr () &&
	  m_input.get_composer_ts () == i.get_composer_ts ();
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

      bool operator() (const input_action_interface& i) const {
	return m_input.get_automaton_ts () == i.get_automaton_ts () &&
	  m_input.get_member_ptr () == i.get_member_ptr ();
      }
    };

    class input_equal_automaton_only
    {
    private:
      timestamp<automaton> m_automaton_ts;

    public:
      input_equal_automaton_only (const timestamp<automaton>& automaton_ts) :
	m_automaton_ts (automaton_ts)
      { }

      bool operator() (const input_action_interface& i) const {
	return m_automaton_ts == i.get_automaton_ts ();
      }
    };

    class composition
    {
    private:
      output m_output;
      std::set<input> m_inputs;

    public:
      composition (const output& output) :
	m_output (output)
      { }

      bool involves_output (const output& output) const {
	return m_output == output;
      }

      bool involves_input (const input& input, bool check_owner) const {
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
      involves_input_automaton (const timestamp<automaton>& automaton) const
      {
	return std::find_if (m_inputs.begin (),
	 		     m_inputs.end (),
	 		     input_equal_automaton_only (automaton)) != m_inputs.end ();
      }

      void compose (const input& input) {
	m_inputs.insert (input);
      }

    };

    class composition_equal
    {
    private:
      const output& m_output;
      const input& m_input;

    public:
      composition_equal (const output& output,
			 const input& input) :
	m_output (output),
	m_input (input)
      { }

      bool operator() (const composition* c) const {
	return c->involves_output (m_output) && c->involves_input (m_input, true);
      }
    };

    class composition_output_equal
    {
    private:
      const output& m_output;

    public:
      composition_output_equal (const output& output) :
	m_output (output)
      { }

      bool operator() (const composition* c) const {
	return c->involves_output (m_output);
      }
    };

    class composition_input_equal
    {
    private:
      const input& m_input;

    public:
      composition_input_equal (const input& input) :
	m_input (input)
      { }

      bool operator() (const composition* c) const {
	return c->involves_input (m_input, false);
      }
    };

    boost::shared_mutex m_mutex;
    std::list<automaton*> m_automata;
    std::map<timestamp<automaton>, timestamp<automaton> > m_parent_child;
    std::list<composition*> m_compositions;
    
    class automaton_timestamp_equal {
    private:
      const timestamp<automaton>& m_automaton;

    public:
      automaton_timestamp_equal (const timestamp<automaton>& automaton) :
	m_automaton (automaton)
      { }

      bool operator() (const automaton* automaton) const {
	return m_automaton == automaton->get_timestamp ();
      }
    };

    automaton*
    timestamp_to_automaton (const timestamp<automaton>& automaton_ts) const
    {
      std::list<automaton*>::const_iterator pos = std::find_if (m_automata.begin (),
								m_automata.end (),
								automaton_timestamp_equal (automaton_ts));
      if (pos != m_automata.end ()) {
	return *pos;
      }
      else {
	return 0;
      }
    }

  public:

    ~system ()
    {
      BOOST_FOREACH (automaton* p, m_automata) {
	delete p;
      }
    }

    enum create_result_type {
      CREATE_CREATOR_DNE,
      CREATE_EXISTS,
      CREATE_SUCCESS,
    };

    template <class T>
    struct create_result
    {
      create_result_type type;
      timestamp<typed_automaton<T> > automaton;

      create_result (const timestamp<typed_automaton<T> >& automaton) :
	type (CREATE_SUCCESS),
	automaton (automaton)
      { }

      create_result (const create_result_type type) :
	type (type)
      {
	BOOST_ASSERT (type != CREATE_SUCCESS);
      }
    };

    template <class T>
    create_result<T>
    create (T* instance)
    {
      BOOST_ASSERT (instance != 0);

      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      std::list<automaton*>::const_iterator pos = std::find_if (m_automata.begin (),
									  m_automata.end (),
									  automata_instance_equal (instance));
      if (pos != m_automata.end ()) {
	return create_result<T> (CREATE_EXISTS);
      }

      typed_automaton<T>* created = new typed_automaton<T> (instance);
      m_automata.push_back (created);
      
      return create_result<T> (created->get_typed_timestamp ());
    }

    template <class T>
    create_result<T>
    create (const timestamp<automaton>& creator,
	    T* instance)
    {
      BOOST_ASSERT (instance != 0);

      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      if (timestamp_to_automaton (creator) == 0) {
	return create_result<T> (CREATE_CREATOR_DNE);
      }

      std::list<automaton*>::const_iterator pos = std::find_if (m_automata.begin (),
									  m_automata.end (),
									  automata_instance_equal (instance));
      if (pos != m_automata.end ()) {
	return create_result<T> (CREATE_EXISTS);
      }

      typed_automaton<T>* created = new typed_automaton<T> (instance);
      m_automata.push_back (created);
      
      m_parent_child.insert (std::make_pair (creator, created));

      return create_result<T> (created->get_typed_timestamp ());
    }

    enum declare_result_type {
      DECLARE_AUTOMATON_DNE,
      DECLARE_EXISTS,
      DECLARE_SUCCESS,
    };

    struct declare_result
    {
      declare_result_type type;

      declare_result (declare_result_type type) :
	type (type)
      { }
    };

    declare_result
    declare (const timestamp<automaton>& a,
	     void* parameter)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      automaton* pa = timestamp_to_automaton (a);
      if (pa == 0) {
	return declare_result (DECLARE_AUTOMATON_DNE);
      }

      if (pa->parameter_exists (parameter)) {
	return declare_result (DECLARE_EXISTS);
      }

      pa->declare (parameter);
      return declare_result (DECLARE_SUCCESS);
    }

    enum compose_result_type {
      COMPOSE_COMPOSER_AUTOMATON_DNE,
      COMPOSE_OUTPUT_AUTOMATON_DNE,
      COMPOSE_INPUT_AUTOMATON_DNE,
      COMPOSE_OUTPUT_PARAMETER_DNE,
      COMPOSE_INPUT_PARAMETER_DNE,
      COMPOSE_EXISTS,
      COMPOSE_INPUT_ACTION_UNAVAILABLE,
      COMPOSE_OUTPUT_ACTION_UNAVAILABLE,
      COMPOSE_SUCCESS,
    };

    struct compose_result
    {
      compose_result_type type;

      compose_result (compose_result_type type) :
	type (type)
      { }
    };

  private:
    template <class OM, class IM>
    compose_result
    compose (const action<OM>& output,
	     const action<IM>& input)
    {
      if (timestamp_to_automaton (input.composer) == 0) {
	// Owner DNE.
	return compose_result (COMPOSE_COMPOSER_AUTOMATON_DNE);
      }

      if (!output.parameter_exists ()) {
	return compose_result (COMPOSE_OUTPUT_PARAMETER_DNE);
      }

      if (!input.parameter_exists ()) {
	return compose_result (COMPOSE_INPUT_PARAMETER_DNE);
      }

      std::list<composition*>::const_iterator pos = std::find_if (m_compositions.begin (),
								  m_compositions.end (),
								  composition_equal (output, input));

      if (pos != m_compositions.end ()) {
	// Composed.
	return compose_result (COMPOSE_EXISTS);
      }

      std::list<composition*>::const_iterator in_pos = std::find_if (m_compositions.begin (),
								     m_compositions.end (),
								     composition_input_equal (input));

      if (in_pos != m_compositions.end ()) {
	// Input unavailable.
	return compose_result (COMPOSE_INPUT_ACTION_UNAVAILABLE);
      }

      std::list<composition*>::const_iterator out_pos = std::find_if (m_compositions.begin (),
								      m_compositions.end (),
								      composition_output_equal (output));

      if (output.automaton_ts == input.automaton_ts ||
	  (out_pos != m_compositions.end () && (*out_pos)->involves_input_automaton (input.automaton_ts))) {
	// Output unavailable.
	return compose_result (COMPOSE_OUTPUT_ACTION_UNAVAILABLE);
      }

      if (out_pos == m_compositions.end ()) {
	m_compositions.push_front (new composition (output));
	out_pos = m_compositions.begin ();
      }

      // Compose.
      (*out_pos)->compose (input);
      return compose_result (COMPOSE_SUCCESS);
    }

    template <class OI, class OM>
    OM*
    get_output_member (const timestamp<typed_automaton<OI> >& output_automaton,
		       OM OI::*output_member_ptr)
    {
      automaton* poa = timestamp_to_automaton (output_automaton);
      if (poa == 0) {
	return 0;
      }
      
      typed_automaton<OI>* toa = static_cast<typed_automaton<OI>*> (poa);
      return &((*(toa->get_typed_instance ())).*output_member_ptr);
    }

    template <class II, class IM>
    IM*
    get_input_member (const timestamp<typed_automaton<II> >& input_automaton,
		      IM II::*input_member_ptr)
    {
      automaton* pia = timestamp_to_automaton (input_automaton);
      if (pia == 0) {
	return 0;
      }

      typed_automaton<II>* tia = static_cast<typed_automaton<II>*> (pia);
      return &((*(tia->get_typed_instance ())).*input_member_ptr);
    }

  public:
    template <class OI, class OM, class II, class IM>
    compose_result
    compose (const timestamp<typed_automaton<OI> >& output_automaton,
	     OM OI::*output_member_ptr,
	     const timestamp<typed_automaton<II> >& input_automaton,
	     IM II::*input_member_ptr,
	     const timestamp<automaton>& owner_automaton)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      OM* output_member = get_output_member (output_automaton, output_member_ptr);
      if (output_member == 0) {
	return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      IM* input_member = get_input_member (input_automaton, input_member_ptr);
      if (input_member == 0) {
	return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
      }

      action<OM> o (output_automaton, output_member);
      action<IM> i (input_automaton, input_member, owner_automaton);

      return compose (o, i);
    }

    template <class OI, class OM, class II, class IM>
    compose_result
    compose (const timestamp<typed_automaton<OI> >& output_automaton,
	     OM OI::*output_member_ptr,
 	     const timestamp<void>& output_parameter,	     
	     const timestamp<typed_automaton<II> >& input_automaton,
	     IM II::*input_member_ptr,
	     const timestamp<automaton>& owner_automaton)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      OM* output_member = get_output_member (output_automaton, output_member_ptr);
      if (output_member == 0) {
	return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      IM* input_member = get_input_member (input_automaton, input_member_ptr);
      if (input_member == 0) {
	return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
      }

      action<OM> o (output_automaton, output_member, output_parameter);
      action<IM> i (input_automaton, input_member, owner_automaton);

      return compose (o, i);
    }

    template <class OI, class OM, class II, class IM>
    compose_result
    compose (const timestamp<typed_automaton<OI> >& output_automaton,
	     OM OI::*output_member_ptr,
	     const timestamp<typed_automaton<II> >& input_automaton,
	     IM II::*input_member_ptr,
	     const timestamp<void>& input_parameter,	     
	     const timestamp<automaton>& owner_automaton)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      OM* output_member = get_output_member (output_automaton, output_member_ptr);
      if (output_member == 0) {
	return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      IM* input_member = get_input_member (input_automaton, input_member_ptr);
      if (input_member == 0) {
	return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
      }

      action<OM> o (output_automaton, output_member);
      action<IM> i (input_automaton, input_member, input_parameter, owner_automaton);

      return compose (o, i);
    }

    // decompose_result decompose (output* o, input* i) { }
    // rescind_result rescind (automaton* automaton, void* parameter) { }
    // destroy_result destroy (automaton* destroyer, automaton* destroyee) { }

    template <class OI, class OM>
    void
    execute_output (const timestamp<typed_automaton<OI> >& output_automaton,
		    OM OI::*output_member_ptr)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);

      OM* output_member = get_output_member (output_automaton, output_member_ptr);
      // For right now.
      BOOST_ASSERT (output_member != 0);

      action<OM> ac (output_automaton, output_member);
      ac ();
    }

    // void execute_internal () { }
    // void deliver_event () { }
  };

}

#endif
