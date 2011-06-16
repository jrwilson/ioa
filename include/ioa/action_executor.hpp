#ifndef __action_executor_hpp__
#define __action_executor_hpp__

#include <ioa/model_interface.hpp>
#include <ioa/system_scheduler_interface.hpp>
#include <memory>
#include <map>

namespace ioa {

  template <class OVS, class OVT, class IVS, class IVT> struct bind_check;
  
  template <>
  struct bind_check<unvalued, null_type, unvalued, null_type> { };

  // These two lines are extremely important.
  // They ensure that the types match when binding an input to an output.
  template <class VT>
  struct bind_check<valued, VT, valued, VT> { };

  template <class I, class M, class K, class VS, class VT> class action_executor_impl;
  
  template <class I, class M>
  class action_executor_impl<I, M, input_category, unvalued, null_type> :
    public unvalued_input_executor_interface
  {
  private:
    const action<I, M> m_action;
    I* m_instance;
      
  public:
    action_executor_impl (const action<I, M>& action) :
      m_action (action),
      m_instance (0)
    { }

    action_executor_impl (const action_executor_impl& other) :
      m_action (other.m_action),
      m_instance (other.m_instance)
    { }
      
    bool fetch_instance (model_interface& model) {
      m_instance = dynamic_cast<I*> (model.get_instance (m_action.automaton));
      return m_instance != 0;
    }
      
    void operator() (system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action (*m_instance);
      system_scheduler.clear_current_aid ();
    }
    
    void bound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action.bound (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }

    void unbound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action.unbound (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }

    const action_interface& get_action () const {
      return m_action;
    }

    action_executor_impl* clone () const {
      return new action_executor_impl (*this);
    }
  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, input_category, valued, VT> :
    public valued_input_executor_interface<VT>
  {
  private:
    const action<I, M> m_action;
    I* m_instance;
      
  public:
    action_executor_impl (const action<I, M>& action) :
      m_action (action),
      m_instance (0)
    { }

    action_executor_impl (const action_executor_impl& other) :
      m_action (other.m_action),
      m_instance (other.m_instance)
    { }
      
    bool fetch_instance (model_interface& model) {
      m_instance = dynamic_cast<I*> (model.get_instance (m_action.automaton));
      return m_instance != 0;
    }
      
    void operator() (system_scheduler_interface& system_scheduler, const VT& t) const {
      assert (m_instance != 0);
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action (*m_instance, t);
      system_scheduler.clear_current_aid ();
    }

    void bound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action.bound (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }

    void unbound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action.unbound (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }
    
    const action_interface& get_action () const {
      return m_action;
    }

    action_executor_impl* clone () const {
      return new action_executor_impl (*this);
    }      
  };
    
  template <class I, class M>
  class action_executor_impl<I, M, output_category, unvalued, null_type> :
    public unvalued_output_executor_interface
  {
  private:
    struct record
    {
      system_scheduler_interface& m_system_scheduler;
      const unvalued_output_executor_interface& m_output;
      std::auto_ptr<unvalued_input_executor_interface> m_input;
      const aid_t m_binder;
      void* const m_key;

      record (system_scheduler_interface& system_scheduler,
	      const unvalued_output_executor_interface& output,
	      const input_executor_interface& input,
	      const aid_t binder,
	      void* const key) :
	m_system_scheduler (system_scheduler),
	m_output (output),
	m_input (dynamic_cast<unvalued_input_executor_interface*> (input.clone ())),
	m_binder (binder),
	m_key (key)
      {
	m_system_scheduler.bound (m_binder, m_key);
	m_system_scheduler.output_bound (m_output);
	m_system_scheduler.input_bound (*m_input.get ());
      }

      ~record () {
	m_system_scheduler.unbound (m_binder, m_key);
	m_system_scheduler.output_unbound (m_output);
	m_system_scheduler.input_unbound (*m_input.get ());
      }

    };
      
    const action<I, M> m_action;
    I* m_instance;
    std::map<aid_t, record*> m_records;
      
  public:
    action_executor_impl (const action<I, M>& action) :
      m_action (action),
      m_instance (0)
    { }

    action_executor_impl (const action_executor_impl& other) :
      m_action (other.m_action),
      m_instance (other.m_instance)
    { }

    ~action_executor_impl () {
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	delete pos->second;
      }
    }
      
    bool fetch_instance (model_interface& model) {
      m_instance = dynamic_cast<I*> (model.get_instance (m_action.automaton));
      return m_instance != 0;
    }
      
    void operator() (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);

      // Lock the automata in order.
      bool output_processed;
	
      // Lock in order.
      output_processed = false;
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (!output_processed &&
	    m_action.get_aid () < pos->first) {
	  model.lock_automaton (m_action.get_aid ());
	  output_processed = true;
	}
	model.lock_automaton (pos->first);
      }


      // Execute.
      system_scheduler.set_current_aid (m_action.get_aid ());
      if (m_action.precondition (*m_instance)) {
	m_action (*m_instance);
	system_scheduler.clear_current_aid ();
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  (*(pos->second->m_input)) (system_scheduler);
	}	  
      }
      else {
	system_scheduler.clear_current_aid ();
      }

      // Unlock.
      output_processed = false;
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (!output_processed &&
	    m_action.get_aid () < pos->first) {
	  model.unlock_automaton (m_action.get_aid ());
	  output_processed = true;
	}
	model.unlock_automaton (pos->first);
      }
    }

    void bound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action.bound (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }

    void unbound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action.unbound (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }

    const action_interface& get_action () const {
      return m_action;
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    bool involves_output (const action_interface& output) const {
      return m_action == output;
    }

    bool involves_input (const action_interface& input) const {
      typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
      if (pos == m_records.end ()) {
	return false;
      }
      else {
	return pos->second->m_input->get_action () == input;
      }
    }

    bool involves_input_automaton (const aid_t aid) const {
      return m_records.find (aid) != m_records.end ();
    }

    bool involves_binding (const action_interface& output,
			   const action_interface& input,
			   const aid_t binder) const {
      typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
      if (pos == m_records.end ()) {
	return false;
      }
      else {
	return m_action == output && pos->second->m_input->get_action () == input && pos->second->m_binder == binder;
      }
    }

    bool involves_aid_key (const aid_t binder,
			   void* const key) const {
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (pos->second->m_binder == binder &&
	    pos->second->m_key == key) {
	  return true;
	}
      }
      return false;
    }

    bool empty () const {
      return m_records.empty ();
    }

    size_t size () const {
      return m_records.size ();
    }

    void bind (system_scheduler_interface& system_scheduler,
	       const input_executor_interface& input,
	       const aid_t binder,
	       void* const key) {
      m_records.insert (std::make_pair (input.get_action ().get_aid (), new record (system_scheduler, *this, input, binder, key)));
    }

    void unbind (const aid_t binder,
		 void* const key) {
      for (typename std::map<aid_t, record*>::iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ) {
	if (pos->second->m_binder == binder &&
	    pos->second->m_key == key) {
	  delete pos->second;
	  m_records.erase (pos++);
	  break;
	}
	else {
	  ++pos;
	}
      }
    }

    void unbind_automaton (const aid_t aid) {
      if (m_action.get_aid () == aid) {
	// We are the aid so delete everything.
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  delete pos->second;
	}
	m_records.clear ();
      }
      else {
	// Look for the aid
	for (typename std::map<aid_t, record*>::iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ) {
	  if (pos->first == aid ||
	      pos->second->m_binder == aid) {
	    delete pos->second;
	    m_records.erase (pos++);
	  }
	  else {
	    ++pos;
	  }
	}
      }
    }
      
  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, output_category, valued, VT> :
    public valued_output_executor_interface<VT>
  {
  private:
    struct record
    {
      system_scheduler_interface& m_system_scheduler;
      const valued_output_executor_interface<VT>& m_output;
      std::auto_ptr<valued_input_executor_interface<VT> > m_input;
      const aid_t m_binder;
      void* const m_key;

      record (system_scheduler_interface& system_scheduler,
	      const valued_output_executor_interface<VT>& output,
	      const input_executor_interface& input,
	      const aid_t binder,
	      void* const key) :
	m_system_scheduler (system_scheduler),
	m_output (output),
	m_input (dynamic_cast<valued_input_executor_interface<VT>*> (input.clone ())),
	m_binder (binder),
	m_key (key)
      {
	m_system_scheduler.bound (m_binder, m_key);
	m_system_scheduler.output_bound (m_output);
	m_system_scheduler.input_bound (*m_input.get ());
      }

      ~record () {
	m_system_scheduler.unbound (m_binder, m_key);
	m_system_scheduler.output_unbound (m_output);
	m_system_scheduler.input_unbound (*m_input.get ());
      }

    };

      
      
    const action<I, M> m_action;
    I* m_instance;
    std::map<aid_t, record*> m_records;
      
  public:
    action_executor_impl (const action<I, M>& action) :
      m_action (action),
      m_instance (0)
    { }

    action_executor_impl (const action_executor_impl& other) :
      m_action (other.m_action),
      m_instance (other.m_instance)
    { }

    ~action_executor_impl () {
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	delete pos->second;
      }
    }
      
    bool fetch_instance (model_interface& model) {
      m_instance = dynamic_cast<I*> (model.get_instance (m_action.automaton));
      return m_instance != 0;
    }
      
    void operator() (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);

      // Lock the automata in order.
      bool output_processed;
	
      // Lock in order.
      output_processed = false;
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (!output_processed &&
	    m_action.get_aid () < pos->first) {
	  model.lock_automaton (m_action.get_aid ());
	  output_processed = true;
	}
	model.lock_automaton (pos->first);
      }


      // Execute.
      system_scheduler.set_current_aid (m_action.get_aid ());
      if (m_action.precondition (*m_instance)) {
	VT value = m_action (*m_instance);
	system_scheduler.clear_current_aid ();

	const VT& value_ref = value;

	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  (*(pos->second->m_input)) (system_scheduler, value_ref);
	}	  
      }
      else {
	system_scheduler.clear_current_aid ();
      }

      // Unlock.
      output_processed = false;
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (!output_processed &&
	    m_action.get_aid () < pos->first) {
	  model.unlock_automaton (m_action.get_aid ());
	  output_processed = true;
	}
	model.unlock_automaton (pos->first);
      }
    }

    void bound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action.bound (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }

    void unbound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action.unbound (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }

    const action_interface& get_action () const {
      return m_action;
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    bool involves_output (const action_interface& output) const {
      return m_action == output;
    }

    bool involves_input (const action_interface& input) const {
      typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
      if (pos == m_records.end ()) {
	return false;
      }
      else {
	return pos->second->m_input->get_action () == input;
      }
    }

    bool involves_input_automaton (const aid_t aid) const {
      return m_records.find (aid) != m_records.end ();
    }

    bool involves_binding (const action_interface& output,
			   const action_interface& input,
			   const aid_t binder) const {
      typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
      if (pos == m_records.end ()) {
	return false;
      }
      else {
	return m_action == output && pos->second->m_input->get_action () == input && pos->second->m_binder == binder;
      }
    }

    bool involves_aid_key (const aid_t binder,
			   void* const key) const {
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (pos->second->m_binder == binder &&
	    pos->second->m_key == key) {
	  return true;
	}
      }
      return false;
    }

    bool empty () const {
      return m_records.empty ();
    }

    size_t size () const {
      return m_records.size ();
    }

    void bind (system_scheduler_interface& system_scheduler,
	       const input_executor_interface& input,
	       const aid_t binder,
	       void* const key) {
      m_records.insert (std::make_pair (input.get_action ().get_aid (), new record (system_scheduler, *this, input, binder, key)));
    }

    void unbind (const aid_t binder,
		 void* const key) {
      for (typename std::map<aid_t, record*>::iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ) {
	if (pos->second->m_binder == binder &&
	    pos->second->m_key == key) {
	  delete pos->second;
	  m_records.erase (pos++);
	  break;
	}
	else {
	  ++pos;
	}
      }
    }

    void unbind_automaton (const aid_t aid) {
      if (m_action.get_aid () == aid) {
	// We are the aid so delete everything.
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  delete pos->second;
	}
	m_records.clear ();
      }
      else {
	// Look for the aid
	for (typename std::map<aid_t, record*>::iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ) {
	  if (pos->first == aid ||
	      pos->second->m_binder == aid) {
	    delete pos->second;
	    m_records.erase (pos++);
	  }
	  else {
	    ++pos;
	  }
	}
      }
    }

  };
    
  template <class I, class M>
  class action_executor_impl<I, M, internal_category, unvalued, null_type> :
    public internal_executor_interface
  {
  private:
    const action<I, M> m_action;
    I* m_instance;
      
  public:
    action_executor_impl (const action<I, M>& action) :
      m_action (action),
      m_instance (0)
    { }
      
    bool fetch_instance (model_interface& model) {
      m_instance = dynamic_cast<I*> (model.get_instance (m_action.automaton));
      return m_instance != 0;
    }
      
    void operator() (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);

      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      if (m_action.precondition (*m_instance)) {
	m_action (*m_instance);
      }
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }
      
    const action_interface& get_action () const {
      return m_action;
    }
      
  };
    
  template <class I, class M, class VS, class VT>
  class action_executor_impl<I, M, event_category, VS, VT> :
    public event_executor_interface
  {
  private:
      
      
    const action<I, M> m_action;
    I* m_instance;
      
  public:
    action_executor_impl (const action<I, M>& action) :
      m_action (action),
      m_instance (0)
    { }
      
    bool fetch_instance (model_interface& model) {
      m_instance = dynamic_cast<I*> (model.get_instance (m_action.automaton));
      return m_instance != 0;
    }
      
    void operator() (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);

      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }
      
    const action_interface& get_action () const {
      return m_action;
    }
      
  };

  template <class I, class M, class VS, class VT>
  class action_executor_impl<I, M, system_input_category, VS, VT> :
    public system_input_executor_interface
  {
  private:
    const action<I, M> m_action;
    I* m_instance;
      
  public:
    action_executor_impl (const action<I, M>& action) :
      m_action (action),
      m_instance (0)
    { }
      
    bool fetch_instance (model_interface& model) {
      m_instance = model.get_instance (m_action.automaton);
      return m_instance != 0;
    }
      
    void operator() (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (m_instance != 0);
	
      model.lock_automaton (m_action.get_aid ());
      system_scheduler.set_current_aid (m_action.get_aid ());
      m_action (*m_instance);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (m_action.get_aid ());
    }
      
    const action_interface& get_action () const {
      return m_action;
    }
      
  };
    
  template <class I, class M>
  class action_executor :
    public action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type>
  {
  public:
    action_executor (const action<I, M>& ac) :
      action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type> (ac)
    { }
  };

  template <class OI, class OM, class II, class IM>
  class bind_executor :
    public bind_executor_interface,
    private bind_check<typename OM::value_status, typename OM::value_type, typename IM::value_status, typename IM::value_type>
  {
  private:
    action_executor<OI, OM> m_output;
    action_executor<II, IM> m_input;

  public:
    bind_executor (const action<OI, OM>& output,
		   const action<II, IM>& input) :
      m_output (output),
      m_input (input)
    { }

    output_executor_interface& get_output () {
      return m_output;
    }

    input_executor_interface& get_input () {
      return m_input;
    }

  };
  
  template <class OI, class OM, class II, class IM>
  shared_ptr<bind_executor_interface> make_bind_executor (const action<OI, OM>& output,
							  const action<II, IM>& input) {
    return shared_ptr<bind_executor_interface> (new bind_executor<OI, OM, II, IM> (output, input));
  }
  
}

#endif
