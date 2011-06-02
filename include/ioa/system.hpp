#ifndef __system_hpp__
#define __system_hpp__

#include "sequential_set.hpp"
#include <list>
#include "automaton_handle.hpp"
#include "binding.hpp"
#include <ioa/generator_interface.hpp>
#include "mutex.hpp"
#include "shared_mutex.hpp"
#include "unique_lock.hpp"
#include "shared_lock.hpp"
#include <map>

namespace ioa {

  // TODO:  Memory allocation.

  class automaton_record_interface :
    public mutex
  {
  private:
    std::auto_ptr<automaton_interface> m_instance;
    aid_t m_aid;
    sequential_set<bid_t> m_bids;
    std::set<automaton_record_interface*> m_children;
    automaton_record_interface* m_parent;

  public:
    automaton_record_interface (automaton_interface* instance,
				const aid_t aid) :
      m_instance (instance),
      m_aid (aid),
      m_parent (0)
    { }

    virtual ~automaton_record_interface () {
      assert (m_children.empty ());
    }


    const aid_t get_aid () const {
      return m_aid;
    }

    automaton_interface* get_instance () const {
      return m_instance.get ();
    }

    bid_t take_bid () {
      return m_bids.take ();
    }

    void replace_bid (const bid_t bid) {
      m_bids.replace (bid);
    }

    void add_child (automaton_record_interface* child) {
      m_children.insert (child);
      child->set_parent (this);
    }

    void remove_child (automaton_record_interface* child) {
      m_children.erase (child);
    }

    automaton_record_interface* get_child () const {
      if (m_children.empty ()) {
	return 0;
      }
      else {
	return *(m_children.begin ());
      }
    }

    void set_parent (automaton_record_interface* parent) {
      m_parent = parent;
    }
    
    automaton_record_interface* get_parent () const {
      return m_parent;
    }
  };

  class root_automaton_record :
    public automaton_record_interface
  {
  public:
    root_automaton_record (automaton_interface* instance,
			   const aid_t aid) :
      automaton_record_interface (instance, aid)
    { }
  };

  template <class P, class D>
  class automaton_record :
    public automaton_record_interface
  {
  private:
    P* m_parent_instance;
    D& m_d;
    
  public:
    automaton_record (automaton_interface* instance,
		      const aid_t aid,
		      P* parent_instance,
		      D& d) :
      automaton_record_interface (instance, aid),
      m_parent_instance (parent_instance),
      m_d (d)
    { }
    
    ~automaton_record () {
      assert (this->get_parent () != 0);
      aid_t parent_aid = this->get_parent ()->get_aid ();
      system_scheduler::set_current_aid (parent_aid, *m_parent_instance);
      m_parent_instance->automaton_destroyed (m_d);
      system_scheduler::clear_current_aid ();
    }

  };

  class system
  {
  private:    
    
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
    static std::map<aid_t, automaton_record_interface*> m_records;
    static std::list<binding_interface*> m_bindings;
    
  public:
    
    static void clear (void) {
      // Delete all root automata.
      while (!m_records.empty ()) {
      	for (std::map<aid_t, automaton_record_interface*>::const_iterator pos = m_records.begin ();
      	     pos != m_records.end ();
      	     ++pos) {
      	  if (pos->second->get_parent () == 0) {
      	    inner_destroy (pos->second);
      	    break;
      	  }
      	}
      }

      assert (m_aids.empty ());
      assert (m_instances.empty ());
      assert (m_records.empty ());
      assert (m_bindings.empty ());
    }

    ~system (void) {
      clear ();
    }
    
    template <class I>
    static automaton_handle<I> cast_aid (const I* /* */, const aid_t aid) {
      return automaton_handle<I> (aid);
    }

    template <class I>
    automaton_handle<I>
    static create (std::auto_ptr<generator_interface<I> > generator)
    {
      unique_lock lock (m_mutex);

      // Take an aid.
      automaton_handle<I> handle (m_aids.take ());

      // Set the current aid.
      system_scheduler::set_current_aid (handle.aid ());

      // Run the generator.
      I* instance = (*generator) ();
      assert (instance != 0);

      // Clear the current aid.
      system_scheduler::clear_current_aid ();
      
      if (m_instances.count (instance) != 0) {
	// Root automaton instance exists.  Bad news.
	m_aids.replace (handle.aid ());
      	return automaton_handle<I> ();
      }

      m_instances.insert (instance);
      automaton_record_interface* record = new root_automaton_record (instance, handle.aid ());
      m_records.insert (std::make_pair (handle.aid (), record));

      // Initialize the automaton.
      system_scheduler::set_current_aid (handle.aid (), *instance);
      instance->init ();
      system_scheduler::clear_current_aid ();

      return handle;
    }

    template <class P, class I, class D>
    static automaton_handle<I>
    create (const automaton_handle<P>& automaton,
    	    std::auto_ptr<generator_interface<I> > generator,
	    D& d)
    {
      unique_lock lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid ())) {
	return automaton_handle<I> ();
      }

      P* p = aid_to_instance<P> (automaton.aid ());

      // Take an aid.
      automaton_handle<I> handle (m_aids.take ());

      // Set the current aid.
      system_scheduler::set_current_aid (handle.aid ());

      // Run the generator.
      I* instance = (*generator) ();
      assert (instance != 0);

      // Clear the current aid.
      system_scheduler::clear_current_aid ();

      if (m_instances.count (instance) != 0) {
	// Return the aid.
	m_aids.replace (handle.aid ());

	system_scheduler::set_current_aid (automaton.aid (), *p);
	p->instance_exists (instance, d);
	system_scheduler::clear_current_aid ();

	return automaton_handle<I> ();
      }

      m_instances.insert (instance);      
      automaton_record_interface* record = new automaton_record<P, D> (instance, handle.aid (), p, d);
      m_records.insert (std::make_pair (handle.aid (), record));
      automaton_record_interface* parent = m_records[automaton.aid ()];
      parent->add_child (record);

      // Tell the parent the child was created.
      system_scheduler::set_current_aid (automaton.aid (), *p);
      p->automaton_created (handle, d);
      system_scheduler::clear_current_aid ();

      // Initialize the child.
      system_scheduler::set_current_aid (handle.aid (), *instance);
      instance->init ();
      system_scheduler::clear_current_aid ();

      return handle;
    }
    
  private:
    template <class OI, class OM, class II, class IM, class I, class D>
    static bid_t
    bind (const action<OI, OM>& output,
	  output_category /* */,
	  const action<II, IM>& input,
	  input_category /* */,
	  const automaton_handle<I>& binder,
	  D& d)
    {
      if (!m_aids.contains (binder.aid ())) {
  	// Binder DNE.
  	return -1;
      }

      I* instance = aid_to_instance<I> (binder.aid ());

      if (!m_aids.contains (output.get_aid ())) {
	system_scheduler::set_current_aid (binder.aid (), *instance);
	instance->output_automaton_dne (d);
	system_scheduler::clear_current_aid ();
	return -1;
      }

      if (!m_aids.contains (input.get_aid ())) {
	system_scheduler::set_current_aid (binder.aid (), *instance);
	instance->input_automaton_dne (d);
	system_scheduler::clear_current_aid ();
	return -1;
      }
      
      std::list<binding_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_equal (output, input, binder.aid ()));
      
      if (pos != m_bindings.end ()) {
  	// Bound.
	system_scheduler::set_current_aid (binder.aid (), *instance);
	instance->binding_exists (d);
	system_scheduler::clear_current_aid ();
	return -1;
      }
      
      std::list<binding_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
									   m_bindings.end (),
									   binding_input_equal (input));
      
      if (in_pos != m_bindings.end ()) {
  	// Input unavailable.
	system_scheduler::set_current_aid (binder.aid (), *instance);
	instance->input_action_unavailable (d);
	system_scheduler::clear_current_aid ();
	return -1;
      }
      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (output));
      
      if (output.get_aid () == input.get_aid () ||
  	  (out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_aid ()))) {
  	// Output unavailable.
	system_scheduler::set_current_aid (binder.aid (), *instance);
	instance->output_action_unavailable (d);
	system_scheduler::clear_current_aid ();
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
      bid_t bid = m_records[binder.aid ()]->take_bid ();

      OI* output_instance = aid_to_instance<OI> (output.get_aid ());
      II* input_instance = aid_to_instance<II> (input.get_aid ());

      c->bind (bid, *output_instance, output, *input_instance, input, *instance, binder.aid (), d);

      return bid;
    }    

  public:
    template <class OI, class OM, class II, class IM, class I, class D>
    static bid_t
    bind (const action<OI, OM>& output,
	  const action<II, IM>& input,
	  const automaton_handle<I>& binder,
	  D& d)
    {
      unique_lock lock (m_mutex);
      
      return bind (output,
		   typename action<OI, OM>::action_category (),
		   input,
		   typename action<II, IM>::action_category (),
		   binder,
		   d);
    }

    template <class I, class D>
    static bool
    unbind (const bid_t bid,
	    const automaton_handle<I>& binder,
	    D& d)
    {
      if (!m_aids.contains (binder.aid ())) {
	return false;
      }
      
      I* instance = aid_to_instance<I> (binder.aid ());
      
      std::list<binding_interface*>::iterator pos = std::find_if (m_bindings.begin (),
								  m_bindings.end (),
								  binding_aid_bid_equal (binder.aid (), bid));
      
      if (pos == m_bindings.end ()) {
	// Not bound.
	system_scheduler::set_current_aid (binder.aid (), *instance);
	instance->binding_dne (d);
	system_scheduler::clear_current_aid ();
	return false;
      }
      
      binding_interface* c = *pos;
      
      // Unbind.
      c->unbind (binder.aid (), bid);
      m_records[binder.aid ()]->replace_bid (bid);
      
      if (c->empty ()) {
	delete c;
	m_bindings.erase (pos);
      }
      
      return true;
    }

  private:
    class child_automaton
    {
    private:
      aid_t m_handle;
    public:
      child_automaton (aid_t handle) :
	m_handle (handle) { }

      bool operator() (const std::pair<aid_t, aid_t>& p) const {
	return p.second == m_handle;
      }
    };

    static void
    inner_destroy (automaton_record_interface* automaton)
    {

      for (automaton_record_interface* child = automaton->get_child ();
      	   child != 0;
      	   child = automaton->get_child ()) {
      	inner_destroy (child);
      }

      // Update bindings.
      for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
      	   pos != m_bindings.end ();
      	   ) {
      	(*pos)->unbind_automaton (automaton->get_aid ());
      	if ((*pos)->empty ()) {
      	  delete *pos;
      	  pos = m_bindings.erase (pos);
      	}
      	else {
      	  ++pos;
      	}
      }

      // Update parent-child relationships.
      automaton_record_interface* parent = automaton->get_parent ();
      if (parent != 0) {
      	parent->remove_child (automaton);
      }

      m_aids.replace (automaton->get_aid ());
      m_instances.erase (automaton->get_instance ());
      m_records.erase (automaton->get_aid ());
      delete automaton;
    }

  public:

    template <class I>
    bool
    destroy (const automaton_handle<I>& target)
    {
      unique_lock lock (m_mutex);
      
      if (!m_aids.contains (target.aid ())) {
	return false;
      }

      inner_destroy (m_records[target.aid ()]);
      return true;
    }

    template <class P, class I, class D>
    static bool
    destroy (const automaton_handle<P>& automaton,
	     const automaton_handle<I>& target,
	     D& d)
    {
      unique_lock lock (m_mutex);

      if (!m_aids.contains (automaton.aid ())) {
	return false;
      }

      P* instance = aid_to_instance<P> (automaton.aid ());

      if (!m_aids.contains (target.aid ())) {
	system_scheduler::set_current_aid (automaton.aid (), *instance);
	instance->target_automaton_dne (d);
	system_scheduler::clear_current_aid ();
	return false;
      }

      automaton_record_interface* parent = m_records[automaton.aid ()];
      automaton_record_interface* child = m_records[target.aid ()];

      if (parent != child->get_parent ()) {
	system_scheduler::set_current_aid (automaton.aid (), *instance);
	instance->destroyer_not_creator (d);
	system_scheduler::clear_current_aid ();
	return false;
      }

      inner_destroy (child);
      return true;
    }

  private:

    template <class I>
    static I* aid_to_instance (const aid_t aid) {
      assert (m_aids.contains (aid));
      I* instance = dynamic_cast<I*> (m_records[aid]->get_instance ());
      assert (instance != 0);
      return instance;
    }

    template <class I, class M>
    static void
    execute0 (const action<I, M>& ac)
    {
      I* instance = aid_to_instance<I> (ac.get_aid ());
      lock_automaton (ac.get_aid ());
      system_scheduler::set_current_aid (ac.get_aid (), *instance);
      ac.execute (*instance);
      system_scheduler::clear_current_aid ();
      unlock_automaton (ac.get_aid ());
    }

    template <class I, class M>
    static void
    execute1 (const action<I, M>& ac,
	      output_category /* */)
    {      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (ac));
      
      if (out_pos == m_bindings.end ()) {
	// Not bound.
	execute0 (ac);
      }
      else {	
	(*out_pos)->execute ();
      }
    }

    template <class I, class M>
    static void
    execute1 (const action<I, M>& ac,
	      internal_category /* */)
    {
      execute0 (ac);
    }

    template <class I, class M>
    static void
    execute1 (const action<I, M>& ac,
	      event_category /* */)
    {
      execute0 (ac);
    }

  public:
    
    template <class I, class M>
    static bool
    execute (const action<I, M>& ac)
    {
      shared_lock lock (m_mutex);
      
      if (!m_aids.contains (ac.get_aid ())) {
	return false;
      }

      execute1 (ac, typename action<I, M>::action_category ());
      return true;
    }

    template <class F, class I, class M, class D>
    bool
    execute (const automaton_handle<F>& from,
	     const action<I, M>& ac,
	     D& d)
    {
      shared_lock lock (m_mutex);

      if (!m_aids.contains (from.aid ())) {
	// Deliverer does not exists.
	return false;
      }

      if (!m_aids.contains (ac.get_aid ())) {
	// Recipient does not exist.
	F* instance = aid_to_instance<F> (from.aid ());
	lock_automaton (from.aid ());
	system_scheduler::set_current_aid (from.aid (), *instance);
	instance->recipient_dne (d);
	system_scheduler::clear_current_aid ();
	unlock_automaton (from.aid ());
	return false;
      }

      execute1 (ac, typename action<I, M>::action_category ());
      return true;
    }

    static void lock_automaton (const aid_t handle)
    {
      m_records[handle]->lock ();
    }

    static void unlock_automaton (const aid_t handle)
    {
      m_records[handle]->unlock ();
    }
    
  };

}

#endif
