#ifndef __system_hpp__
#define __system_hpp__

#include <ioa/automaton_record.hpp>
#include <ioa/binding.hpp>

#include <ioa/generator_interface.hpp>
#include <ioa/unique_lock.hpp>
#include <ioa/shared_lock.hpp>

#include <map>
#include <list>

namespace ioa {

  // TODO:  Memory allocation.

  class system
  {
  private:    

    // Can't construct one.
    system () { }
    
    class binding_equal
    {
    private:
      const action_interface& m_output;
      const action_interface& m_input;
      const aid_t m_binder;

    public:
      binding_equal (const action_interface& output,
		     const action_interface& input,
		     const aid_t binder) :
  	m_output (output),
  	m_input (input),
	m_binder (binder)
      { }
      
      bool operator() (const binding_interface* c) const {
  	return c->involves_binding (m_output, m_input, m_binder);
      }
    };
    
    class binding_output_equal
    {
    private:
      const action_interface& m_output;
      
    public:
      binding_output_equal (const action_interface& output) :
  	m_output (output)
      { }
      
      bool operator() (const binding_interface* c) const {
  	return c->involves_output (m_output);
      }
    };
    
    class binding_input_equal
    {
    private:
      const action_interface& m_input;
      
    public:
      binding_input_equal (const action_interface& input) :
  	m_input (input)
      { }
      
      bool operator() (const binding_interface* c) const {
  	return c->involves_input (m_input);
      }
    };

    class binding_aid_bid_equal
    {
    private:
      const aid_t m_aid;
      const bid_t m_bid;

    public:
      binding_aid_bid_equal (const aid_t aid,
			     const bid_t bid) :
	m_aid (aid),
	m_bid (bid)
      { }

      bool operator() (const binding_interface* c) const {
	return c->involves_aid_bid (m_aid, m_bid);
      }
    };
    
    static shared_mutex m_mutex;
    static sequential_set<aid_t> m_aids;
    static std::set<automaton_interface*> m_instances;
    static std::map<aid_t, automaton_record*> m_records;
    static std::list<binding_interface*> m_bindings;

    template <class I>
    static I* automaton_handle_to_instance (const automaton_handle<I>& handle) {
      if (!m_aids.contains (handle)) {
	return 0;
      }
      return dynamic_cast<I*> (m_records[handle]->get_instance ());
    }

    static void inner_destroy (automaton_record* automaton);
    
  public:
    
    static void clear (void);
    static aid_t create (std::auto_ptr<generator_interface> generator);
    static aid_t create (const aid_t automaton,
			 std::auto_ptr<generator_interface> generator,
			 void* aux);

  private:
    template <class OI, class OM, class II, class IM>
    static bid_t
    bind (const action<OI, OM>& output,
	  output_category /* */,
	  const action<II, IM>& input,
	  input_category /* */,
	  const aid_t binder,
	  void* aux)
    {
      if (!m_aids.contains (binder)) {
  	// Binder DNE.
  	return -1;
      }

      OI* output_instance = automaton_handle_to_instance (output.automaton);

      if (0 == output_instance) {
	system_scheduler::output_automaton_dne (binder, aux);
	return -1;
      }

      II* input_instance = automaton_handle_to_instance (input.automaton);

      if (0 == input_instance) {
	system_scheduler::input_automaton_dne (binder, aux);
	return -1;
      }
      
      std::list<binding_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_equal (output, input, binder));
      
      if (pos != m_bindings.end ()) {
  	// Bound.
	system_scheduler::binding_exists (binder, aux);
	return -1;
      }
      
      std::list<binding_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
									   m_bindings.end (),
									   binding_input_equal (input));
      
      if (in_pos != m_bindings.end ()) {
  	// Input unavailable.
	system_scheduler::input_action_unavailable (binder, aux);
	return -1;
      }
      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (output));
      
      if (output.get_aid () == input.get_aid () ||
  	  (out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_aid ()))) {
  	// Output unavailable.
	system_scheduler::output_action_unavailable (binder, aux);
	return -1;
      }
      
      binding<OM>* c;
      
      if (out_pos != m_bindings.end ()) {
	c = dynamic_cast<binding<OM>*> (*out_pos);
	assert (c != 0);
      }
      else {
	c = new binding<OM> ();
	m_bindings.push_front (c);
      }
    
      // Bind.
      bid_t bid = m_records[binder]->take_bid ();

      // TODO:  Think about the bound and unbound actions so we don't need these instance.
      c->bind (bid, *output_instance, output, *input_instance, input, binder, aux);

      return bid;
    }    

  public:
    template <class OI, class OM, class II, class IM>
    static bid_t
    bind (const action<OI, OM>& output,
	  const action<II, IM>& input,
	  const aid_t binder,
	  void* aux)
    {
      unique_lock lock (m_mutex);
      
      return bind (output,
		   typename action<OI, OM>::action_category (),
		   input,
		   typename action<II, IM>::action_category (),
		   binder,
		   aux);
    }

    static bool unbind (const bid_t bid,
			const aid_t binder,
			void* aux);

    static bool destroy (const aid_t target);
    static bool destroy (const aid_t automaton,
			 const aid_t target,
			 void* aux);

  private:

    template <class I, class M>
    static void
    execute0 (I* instance, const action<I, M>& ac)
    {
      lock_automaton (ac.get_aid ());
      system_scheduler::set_current_aid (ac.get_aid (), *instance);
      ac (*instance);
      system_scheduler::clear_current_aid ();
      unlock_automaton (ac.get_aid ());
    }

    template <class I, class M>
    static void
    execute1 (I* instance,
	      const action<I, M>& ac,
	      output_category /* */)
    {      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (ac));
      
      if (out_pos == m_bindings.end ()) {
	// Not bound.
	execute0 (instance, ac);
      }
      else {	
	(*out_pos)->execute ();
      }
    }

    template <class I, class M>
    static void
    execute1 (I* instance,
	      const action<I, M>& ac,
	      internal_category /* */)
    {
      execute0 (instance, ac);
    }

    template <class I, class M>
    static void
    execute1 (I* instance,
	      const action<I, M>& ac,
	      event_category /* */)
    {
      execute0 (instance, ac);
    }

  public:
    
    template <class I, class M>
    static bool
    execute (const action<I, M>& ac)
    {
      shared_lock lock (m_mutex);

      I* instance = automaton_handle_to_instance (ac.automaton);
      if (0 == instance) {
	return false;
      }

      execute1 (instance, ac, typename action<I, M>::action_category ());
      return true;
    }

    template <class I, class M>
    static bool
    execute (const aid_t from,
	     const action<I, M>& ac,
	     void* aux)
    {
      shared_lock lock (m_mutex);

      if (!m_aids.contains (from)) {
	// Deliverer does not exists.
	return false;
      }

      I* instance = automaton_handle_to_instance (ac.automaton);

      if (0 == instance) {
	// Recipient does not exist.
	system_scheduler::recipient_dne (from, aux);
	return false;
      }

      execute1 (instance, ac, typename action<I, M>::action_category ());
      return true;
    }

    static void lock_automaton (const aid_t handle);
    static void unlock_automaton (const aid_t handle);
  };

}

#endif
