#ifndef __action_executor_hpp__
#define __action_executor_hpp__

#include <ioa/executor_interface.hpp>
#include <ioa/automaton_handle.hpp>
#include <ioa/action.hpp>
#include <ioa/model_interface.hpp>
#include <ioa/system_scheduler_interface.hpp>
#include <memory>
#include <map>

namespace ioa {

  template <class OVS, class OVT, class IVS, class IVT> struct bind_check;
  
  template <class I, class M>
  struct action_executor_core
  {
    const automaton_handle<I> m_handle;
    M I::*m_member_ptr;
    I* m_instance;

    action_executor_core (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      m_handle (handle),
      m_member_ptr (member_ptr),
      m_instance (0)
    { }

    action_executor_core (const action_executor_core& other) :
      m_handle (other.m_handle),
      m_member_ptr (other.m_member_ptr),
      m_instance (other.m_instance)
    { }

    bool fetch_instance (model_interface& model) {
      m_instance = dynamic_cast<I*> (model.get_instance (m_handle));
      return m_instance != 0;
    }

    aid_t get_aid () const {
      return m_handle;
    }

    void* get_member_ptr () const {
      I* tmp = 0;
      return &(tmp->*m_member_ptr);
    }
  };

  template <class I, class M, class K, class VS, class VT, class PS, class PT> class action_executor_impl;
  
  template <class I, class M>
  class action_executor_impl<I, M, input_category, unvalued, null_type, unparameterized, null_type> :
    public unvalued_input_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (handle, member_ptr)
    { }
      
    void operator() (system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance));
      system_scheduler.clear_current_aid ();
    }

    void set_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound ();
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound ();
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    action_executor_impl* clone () const {
      return new action_executor_impl (*this);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return 0;
    }

  };

  template <class I, class M, class PT>
  class action_executor_impl<I, M, input_category, unvalued, null_type, parameterized, PT> :
    public unvalued_input_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    PT m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other),
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      action_executor_core<I, M> (handle, member_ptr),
      m_parameter (parameter)
    { }

    void operator() (system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), m_parameter);
      system_scheduler.clear_current_aid ();
    }

    void set_parameter (const aid_t) {
      // Not auto_parameterized.
    }
    
    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    action_executor_impl* clone () const {
      return new action_executor_impl (*this);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }

  };

  template <class I, class M>
  class action_executor_impl<I, M, input_category, unvalued, null_type, auto_parameterized, aid_t> :
    public unvalued_input_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    aid_t m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other),
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (handle, member_ptr),
      m_parameter (-1)
    { }

    void set_parameter (const aid_t aid) {
      m_parameter = aid;
    }
      
    void operator() (system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), m_parameter);
      system_scheduler.clear_current_aid ();
    }
    
    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    action_executor_impl* clone () const {
      return new action_executor_impl (*this);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }

  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, input_category, valued, VT, unparameterized, null_type> :
    public valued_input_executor_interface<VT>,
    private action_executor_core<I, M>
  {
  private:
    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (handle, member_ptr)
    { }
      
    void operator() (system_scheduler_interface& system_scheduler,
		     const VT& t) const {
      assert (this->m_instance != 0);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), t);
      system_scheduler.clear_current_aid ();
    }

    void set_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    void bound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound ();
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound ();
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }
    
    action_executor_impl* clone () const {
      return new action_executor_impl (*this);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return 0;
    }

  };

  template <class I, class M, class VT, class PT>
  class action_executor_impl<I, M, input_category, valued, VT, parameterized, PT> :
    public valued_input_executor_interface<VT>,
    private action_executor_core<I, M>
  {
  private:
    PT m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other),
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      action_executor_core<I, M> (handle, member_ptr),
      m_parameter (parameter)
    { }
      
    void operator() (system_scheduler_interface& system_scheduler,
		     const VT& t) const {
      assert (this->m_instance != 0);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), t, m_parameter);
      system_scheduler.clear_current_aid ();
    }

    void set_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    void bound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }
    
    action_executor_impl* clone () const {
      return new action_executor_impl (*this);
    }      

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }

  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, input_category, valued, VT, auto_parameterized, aid_t> :
    public valued_input_executor_interface<VT>,
    private action_executor_core<I, M>
  {
  private:
    aid_t m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other),
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (handle, member_ptr),
      m_parameter (-1)
    { }

    void set_parameter (const aid_t aid) {
      m_parameter = aid;
    }
      
    void operator() (system_scheduler_interface& system_scheduler,
		     const VT& t) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), t, m_parameter);
      system_scheduler.clear_current_aid ();
    }

    void bound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model, system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }
    
    action_executor_impl* clone () const {
      return new action_executor_impl (*this);
    }      

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }

  };

  template <class I, class M, class OE, class IE>
  struct output_core :
    public action_executor_core<I, M>
  {
    struct record
    {
      system_scheduler_interface& m_system_scheduler;
      model_interface& m_model;
      const OE& m_output;
      std::auto_ptr<IE> m_input;
      const aid_t m_binder;
      void* const m_key;
      
      record (system_scheduler_interface& system_scheduler,
	      model_interface& model,
	      const OE& output,
	      const input_executor_interface& input,
	      const aid_t binder,
	      void* const key) :
	m_system_scheduler (system_scheduler),
	m_model (model),
	m_output (output),
	m_input (dynamic_cast<IE*> (input.clone ())),
	m_binder (binder),
	m_key (key)
      {
	m_model.add_bind_key (m_binder, m_key);
	m_system_scheduler.bound (m_binder, automaton::BOUND_RESULT, m_key);
	m_system_scheduler.output_bound (m_output);
	m_system_scheduler.input_bound (*m_input.get ());
      }
      
      ~record () {
	m_model.remove_bind_key (m_binder, m_key);
	m_system_scheduler.unbound (m_binder, automaton::UNBOUND_RESULT, m_key);
	m_system_scheduler.output_unbound (m_output);
	m_system_scheduler.input_unbound (*m_input.get ());
      }
      
    };

    std::map<aid_t, record*> m_records;

    output_core (const automaton_handle<I>& handle,
		 M I::*member_ptr,
		 OE& output) :
      action_executor_core<I, M> (handle, member_ptr)
    { }

    output_core (const output_core& other) :
      action_executor_core<I, M> (other)
    {
      // Don't copy the records.
      assert (m_records.empty ());
    }

    ~output_core () {
      assert (m_records.empty ());
    }

    void lock_in_order (model_interface& model) const {
      // Lock in order.
      bool output_processed = false;
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (!output_processed &&
	    this->m_handle < pos->first) {
	  model.lock_automaton (this->m_handle);
	  output_processed = true;
	}
	model.lock_automaton (pos->first);
      }
    }

    void unlock_in_order (model_interface& model) const {
      // Unlock.
      bool output_processed = false;
      for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	if (!output_processed &&
	    this->m_handle < pos->first) {
	  model.unlock_automaton (this->m_handle);
	  output_processed = true;
	}
	model.unlock_automaton (pos->first);
      }
    }

    bool involves_output (const OE& this_output, const action_executor_interface& output) const {
      return this_output == output;
    }

    bool involves_input (const action_executor_interface& input) const {
      typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
      if (pos == m_records.end ()) {
	return false;
      }
      else {
	return *(pos->second->m_input) == input;
      }
    }

    bool involves_input_automaton (const aid_t aid) const {
      return m_records.find (aid) != m_records.end ();
    }

    bool involves_binding (const OE& this_output,
			   const action_executor_interface& output,
			   const action_executor_interface& input,
			   const aid_t binder) const {
      typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
      if (pos == m_records.end ()) {
	return false;
      }
      else {
	return this_output == output && *(pos->second->m_input) == input && pos->second->m_binder == binder;
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
	       model_interface& model,
	       const OE& output,
	       const input_executor_interface& input,
	       const aid_t binder,
	       void* const key) {
      m_records.insert (std::make_pair (input.get_aid (), new record (system_scheduler, model, output, input, binder, key)));
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
      if (aid == this->m_handle) {
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
  class action_executor_impl<I, M, output_category, unvalued, null_type, unparameterized, null_type> :
    public unvalued_output_executor_interface,
    private output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>
  {
  private:
    typedef typename output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::record record;

    action_executor_impl (const action_executor_impl& other) :
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface> (other)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface> (handle, member_ptr, *this)
    { }

    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);

      // Lock in order.
      this->lock_in_order (model);

      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance)->*(this->m_member_ptr)).precondition (*(this->m_instance))) {
	((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance));
	system_scheduler.clear_current_aid ();
	for (typename std::map<aid_t, record*>::const_iterator pos = this->m_records.begin ();
	     pos != this->m_records.end ();
	     ++pos) {
	  (*(pos->second->m_input)) (system_scheduler);
	}	  
      }
      else {
	system_scheduler.clear_current_aid ();
      }

      // Unlock in order.
      this->unlock_in_order (model);
    }

    void set_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound ();
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound ();
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return 0;
    }

    bool involves_output (const action_executor_interface& exec) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_output (*this, exec);
    }

    bool involves_input (const action_executor_interface& exec) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_input (exec);
    }

    bool involves_input_automaton (const aid_t aid) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_input_automaton (aid);
    }

    bool involves_binding (const action_executor_interface& output,
			   const action_executor_interface& input,
			   const aid_t aid) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_binding (*this, output, input, aid);
    }

    bool involves_aid_key (const aid_t aid,
			   void* const key) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_aid_key (aid, key);
    }

    bool empty () const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::empty ();
    }

    size_t size () const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::size ();
    }

    void bind (system_scheduler_interface& system_scheduler,
	       model_interface& model,
	       const input_executor_interface& input,
	       const aid_t aid,
	       void* const key) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::bind (system_scheduler, model, *this, input, aid, key);
    }

    void unbind (const aid_t aid,
		 void* const key) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::unbind (aid, key);
    }

    void unbind_automaton (const aid_t aid) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::unbind_automaton (aid);
    }
      
  };

  template <class I, class M, class PT>
  class action_executor_impl<I, M, output_category, unvalued, null_type, parameterized, PT> :
    public unvalued_output_executor_interface,
    private output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>
  {
  private:
    typedef typename output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::record record;
    PT m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface> (other),
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface> (handle, member_ptr, *this),
      m_parameter (parameter)
    { }

    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);

      // Lock in order.
      this->lock_in_order (model);

      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance)->*(this->m_member_ptr)).precondition (*(this->m_instance), m_parameter)) {
	((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), m_parameter);
	system_scheduler.clear_current_aid ();
	for (typename std::map<aid_t, record*>::const_iterator pos = this->m_records.begin ();
	     pos != this->m_records.end ();
	     ++pos) {
	  (*(pos->second->m_input)) (system_scheduler);
	}	  
      }
      else {
	system_scheduler.clear_current_aid ();
      }
      
      // Unlock in order.
      this->unlock_in_order (model);
    }

    void set_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }
      
    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }

    bool involves_output (const action_executor_interface& exec) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_output (*this, exec);
    }

    bool involves_input (const action_executor_interface& exec) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_input (exec);
    }

    bool involves_input_automaton (const aid_t aid) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_input_automaton (aid);
    }

    bool involves_binding (const action_executor_interface& output,
			   const action_executor_interface& input,
			   const aid_t aid) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_binding (*this, output, input, aid);
    }

    bool involves_aid_key (const aid_t aid,
			   void* const key) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_aid_key (aid, key);
    }

    bool empty () const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::empty ();
    }

    size_t size () const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::size ();
    }

    void bind (system_scheduler_interface& system_scheduler,
	       model_interface& model,
	       const input_executor_interface& input,
	       const aid_t aid,
	       void* const key) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::bind (system_scheduler, model, *this, input, aid, key);
    }

    void unbind (const aid_t aid,
		 void* const key) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::unbind (aid, key);
    }

    void unbind_automaton (const aid_t aid) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::unbind_automaton (aid);
    }

  };

  template <class I, class M>
  class action_executor_impl<I, M, output_category, unvalued, null_type, auto_parameterized, aid_t> :
    public unvalued_output_executor_interface,
    private output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>
  {
  private:
    typedef typename output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::record record;
    aid_t m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface> (other),
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface> (handle, member_ptr, *this),
      m_parameter (-1)
    { }

    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const aid_t aid) :
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface> (handle, member_ptr, *this),
      m_parameter (aid)
    { }

    void set_parameter (const aid_t aid) {
      m_parameter = aid;
    }

    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);

      // Lock in order.
      this->lock_in_order (model);

      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance)->*(this->m_member_ptr)).precondition (*(this->m_instance), m_parameter)) {
	((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), m_parameter);
	system_scheduler.clear_current_aid ();
	for (typename std::map<aid_t, record*>::const_iterator pos = this->m_records.begin ();
	     pos != this->m_records.end ();
	     ++pos) {
	  (*(pos->second->m_input)) (system_scheduler);
	}	  
      }
      else {
	system_scheduler.clear_current_aid ();
      }
      
      // Unlock in order.
      this->unlock_in_order (model);
    }

    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }
      
    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }

    bool involves_output (const action_executor_interface& exec) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_output (*this, exec);
    }

    bool involves_input (const action_executor_interface& exec) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_input (exec);
    }

    bool involves_input_automaton (const aid_t aid) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_input_automaton (aid);
    }

    bool involves_binding (const action_executor_interface& output,
			   const action_executor_interface& input,
			   const aid_t aid) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_binding (*this, output, input, aid);
    }

    bool involves_aid_key (const aid_t aid,
			   void* const key) const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::involves_aid_key (aid, key);
    }

    bool empty () const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::empty ();
    }

    size_t size () const {
      return output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::size ();
    }

    void bind (system_scheduler_interface& system_scheduler,
	       model_interface& model,
	       const input_executor_interface& input,
	       const aid_t aid,
	       void* const key) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::bind (system_scheduler, model, *this, input, aid, key);
    }

    void unbind (const aid_t aid,
		 void* const key) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::unbind (aid, key);
    }

    void unbind_automaton (const aid_t aid) {
      output_core<I, M, unvalued_output_executor_interface, unvalued_input_executor_interface>::unbind_automaton (aid);
    }

  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, output_category, valued, VT, unparameterized, null_type> :
    public valued_output_executor_interface<VT>,
    private output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >
  {
  private:
    typedef typename output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::record record;

    action_executor_impl (const action_executor_impl& other) :
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> > (other)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> > (handle, member_ptr, *this)
    { }

    void set_parameter (const aid_t) {
      // Not auto_parameterized.
    }
      
    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);

      // Lock in order.
      this->lock_in_order (model);

      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance)->*(this->m_member_ptr)).precondition (*(this->m_instance))) {
	VT v = ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance));
	const VT& value = v;
	system_scheduler.clear_current_aid ();
	for (typename std::map<aid_t, record*>::const_iterator pos = this->m_records.begin ();
	     pos != this->m_records.end ();
	     ++pos) {
	  (*(pos->second->m_input)) (system_scheduler, value);
	}	  
      }
      else {
	system_scheduler.clear_current_aid ();
      }

      // Unlock in order.
      this->unlock_in_order (model);
    }

    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound ();
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound ();
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return 0;
    }

    bool involves_output (const action_executor_interface& exec) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_output (*this, exec);
    }

    bool involves_input (const action_executor_interface& exec) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_input (exec);
    }

    bool involves_input_automaton (const aid_t aid) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_input_automaton (aid);
    }

    bool involves_binding (const action_executor_interface& output,
			   const action_executor_interface& input,
			   const aid_t aid) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_binding (*this, output, input, aid);
    }

    bool involves_aid_key (const aid_t aid,
			   void* const key) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_aid_key (aid, key);
    }

    bool empty () const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::empty ();
    }

    size_t size () const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::size ();
    }

    void bind (system_scheduler_interface& system_scheduler,
	       model_interface& model,
	       const input_executor_interface& input,
	       const aid_t aid,
	       void* const key) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::bind (system_scheduler, model, *this, input, aid, key);
    }

    void unbind (const aid_t aid,
		 void* const key) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::unbind (aid, key);
    }

    void unbind_automaton (const aid_t aid) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::unbind_automaton (aid);
    }

  };

  template <class I, class M, class VT, class PT>
  class action_executor_impl<I, M, output_category, valued, VT, parameterized, PT> :
    public valued_output_executor_interface<VT>,
    private output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >
  {
  private:
    typedef typename output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::record record;
    PT m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> > (other),
      m_parameter (other.m_parameter)
    { }
    
  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> > (handle, member_ptr, *this),
      m_parameter (parameter)
    { }

    void set_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);

      // Lock in order.
      this->lock_in_order (model);

      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance)->*(this->m_member_ptr)).precondition (*(this->m_instance), m_parameter)) {
	VT v = ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), m_parameter);
	const VT& value = v;
	system_scheduler.clear_current_aid ();
	for (typename std::map<aid_t, record*>::const_iterator pos = this->m_records.begin ();
	     pos != this->m_records.end ();
	     ++pos) {
	  (*(pos->second->m_input)) (system_scheduler, value);
	}	  
      }
      else {
	system_scheduler.clear_current_aid ();
      }

      // Unlock in order.
      this->unlock_in_order (model);
    }

    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }

    bool involves_output (const action_executor_interface& exec) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_output (*this, exec);
    }

    bool involves_input (const action_executor_interface& exec) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_input (exec);
    }

    bool involves_input_automaton (const aid_t aid) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_input_automaton (aid);
    }

    bool involves_binding (const action_executor_interface& output,
			   const action_executor_interface& input,
			   const aid_t aid) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_binding (*this, output, input, aid);
    }

    bool involves_aid_key (const aid_t aid,
			   void* const key) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_aid_key (aid, key);
    }

    bool empty () const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::empty ();
    }

    size_t size () const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::size ();
    }

    void bind (system_scheduler_interface& system_scheduler,
	       model_interface& model,
	       const input_executor_interface& input,
	       const aid_t aid,
	       void* const key) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::bind (system_scheduler, model, *this, input, aid, key);
    }

    void unbind (const aid_t aid,
		 void* const key) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::unbind (aid, key);
    }

    void unbind_automaton (const aid_t aid) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::unbind_automaton (aid);
    }

  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, output_category, valued, VT, auto_parameterized, aid_t> :
    public valued_output_executor_interface<VT>,
    private output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >
  {
  private:
    typedef typename output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::record record;
    aid_t m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> > (other),
      m_parameter (other.m_parameter)
    { }
    
  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> > (handle, member_ptr, *this),
      m_parameter (-1)
    { }

    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const aid_t aid) :
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> > (handle, member_ptr, *this),
      m_parameter (aid)
    { }

    void set_parameter (const aid_t aid) {
      m_parameter = aid;
    }
      
    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);

      // Lock in order.
      this->lock_in_order (model);

      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance)->*(this->m_member_ptr)).precondition (*(this->m_instance), m_parameter)) {
	VT v = ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), m_parameter);
	const VT& value = v;
	system_scheduler.clear_current_aid ();
	for (typename std::map<aid_t, record*>::const_iterator pos = this->m_records.begin ();
	     pos != this->m_records.end ();
	     ++pos) {
	  (*(pos->second->m_input)) (system_scheduler, value);
	}	  
      }
      else {
	system_scheduler.clear_current_aid ();
      }

      // Unlock in order.
      this->unlock_in_order (model);
    }

    void bound (model_interface& model,
		system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).bound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    void unbound (model_interface& model,
		  system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
      assert (m_parameter != -1);
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)).unbound (m_parameter);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }

    bool involves_output (const action_executor_interface& exec) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_output (*this, exec);
    }

    bool involves_input (const action_executor_interface& exec) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_input (exec);
    }

    bool involves_input_automaton (const aid_t aid) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_input_automaton (aid);
    }

    bool involves_binding (const action_executor_interface& output,
			   const action_executor_interface& input,
			   const aid_t aid) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_binding (*this, output, input, aid);
    }

    bool involves_aid_key (const aid_t aid,
			   void* const key) const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::involves_aid_key (aid, key);
    }

    bool empty () const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::empty ();
    }

    size_t size () const {
      return output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::size ();
    }

    void bind (system_scheduler_interface& system_scheduler,
	       model_interface& model,
	       const input_executor_interface& input,
	       const aid_t aid,
	       void* const key) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::bind (system_scheduler, model, *this, input, aid, key);
    }

    void unbind (const aid_t aid,
		 void* const key) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::unbind (aid, key);
    }

    void unbind_automaton (const aid_t aid) {
      output_core<I, M, valued_output_executor_interface<VT>, valued_input_executor_interface<VT> >::unbind_automaton (aid);
    }

  };

  template <class I, class M>
  class action_executor_impl<I, M, internal_category, unvalued, null_type, unparameterized, null_type> :
    public internal_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    action_executor_impl (const action_executor_impl& other)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (handle, member_ptr)
    { }
      
    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);

      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance)->*(this->m_member_ptr)).precondition (*(this->m_instance))) {
	((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance));
      }
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return 0;
    }
      
  };

  template <class I, class M, class PT>
  class action_executor_impl<I, M, internal_category, unvalued, null_type, parameterized, PT> :
    public internal_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    PT m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      action_executor_core<I, M> (handle, member_ptr),
      m_parameter (parameter)
    { }
      
    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);

      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance)->*(this->m_member_ptr)).precondition (*(this->m_instance), m_parameter)) {
	((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), m_parameter);
      }
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return static_cast<size_t> (m_parameter);
    }
      
  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, system_input_category, valued, VT, unparameterized, null_type> :
    public system_input_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    const VT m_value;

    action_executor_impl (const action_executor_impl& other)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const VT& value) :
      action_executor_core<I, M> (handle, member_ptr),
      m_value (value)
    { }
      
    void operator() (model_interface& model,
		     system_scheduler_interface& system_scheduler) const {
      assert (this->m_instance != 0);
	
      model.lock_automaton (this->m_handle);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance)->*(this->m_member_ptr)) (*(this->m_instance), m_value);
      system_scheduler.clear_current_aid ();
      model.unlock_automaton (this->m_handle);
    }

    bool operator== (const action_executor_interface& other) const {
      // System inputs are unique.
      return false;
    }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return 0;
    }
      
  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, system_output_category, valued, VT, unparameterized, null_type> :
    public action_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    action_executor_impl (const action_executor_impl& other)
    { }

  public:
    action_executor_impl (const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (handle, member_ptr)
    { }

    bool fetch_instance (model_interface& model) {
      return action_executor_core<I,M>::fetch_instance (model);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    size_t get_pid () const {
      return 0;
    }
  };
    
  template <class I, class M>
  class action_executor :
    public action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type, typename M::parameter_status, typename M::parameter_type>
  {
  public:
    action_executor (const automaton_handle<I>& handle,
		     M I::*member_ptr) :
      action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type, typename M::parameter_status, typename M::parameter_type> (handle, member_ptr)
    { }

    action_executor (const automaton_handle<I>& handle,
		     M I::*member_ptr,
		     const typename M::parameter_type& parameter) :
      action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type, typename M::parameter_status, typename M::parameter_type> (handle, member_ptr, parameter)
    { }

    action_executor (const automaton_handle<I>& handle,
		     M I::*member_ptr,
		     const typename M::value_type& value,
		     system_input_category /* */) :
      action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type, typename M::parameter_status, typename M::parameter_type> (handle, member_ptr, value)
    { }
    
  };

  template <>
  struct bind_check<unvalued, null_type, unvalued, null_type> { };

  // These two lines are extremely important.
  // They ensure that the types match when binding an input to an output.
  template <class VT>
  struct bind_check<valued, VT, valued, VT> { };

  template <class OI, class OM, class II, class IM>
  class bind_executor :
    public bind_executor_interface,
    private bind_check<typename OM::value_status, typename OM::value_type, typename IM::value_status, typename IM::value_type>
  {
  private:
    action_executor<OI, OM> m_output;
    action_executor<II, IM> m_input;

  public:
    bind_executor (const automaton_handle<OI>& output_handle,
		   OM OI::*output_member_ptr,
		   const automaton_handle<II>& input_handle,
		   IM II::*input_member_ptr) :
      m_output (output_handle, output_member_ptr),
      m_input (input_handle, input_member_ptr)
    { }

    bind_executor (const automaton_handle<OI>& output_handle,
		   OM OI::*output_member_ptr,
		   const typename OM::parameter_type& output_parameter,
		   const automaton_handle<II>& input_handle,
		   IM II::*input_member_ptr) :
      m_output (output_handle, output_member_ptr, output_parameter),
      m_input (input_handle, input_member_ptr)
    { }

    bind_executor (const automaton_handle<OI>& output_handle,
		   OM OI::*output_member_ptr,
		   const automaton_handle<II>& input_handle,
		   IM II::*input_member_ptr,
		   const typename IM::parameter_type& input_parameter) :
      m_output (output_handle, output_member_ptr),
      m_input (input_handle, input_member_ptr, input_parameter)
    { }

    bind_executor (const automaton_handle<OI>& output_handle,
		   OM OI::*output_member_ptr,
		   const typename OM::parameter_type& output_parameter,
		   const automaton_handle<II>& input_handle,
		   IM II::*input_member_ptr,
		   const typename IM::parameter_type& input_parameter) :
      m_output (output_handle, output_member_ptr, output_parameter),
      m_input (input_handle, input_member_ptr, input_parameter)
    { }

    output_executor_interface& get_output () {
      return m_output;
    }

    input_executor_interface& get_input () {
      return m_input;
    }

  };
  
  template <class OI, class OM, class II, class IM>
  shared_ptr<bind_executor_interface> make_bind_executor (const automaton_handle<OI>& output_handle,
							  OM OI::*output_member_ptr,
							  const automaton_handle<II>& input_handle,
							  IM II::*input_member_ptr) {
    return shared_ptr<bind_executor_interface> (new bind_executor<OI, OM, II, IM> (output_handle, output_member_ptr, input_handle, input_member_ptr));
  }

  template <class OI, class OM, class II, class IM>
  shared_ptr<bind_executor_interface> make_bind_executor (const automaton_handle<OI>& output_handle,
							  OM OI::*output_member_ptr,
							  const typename OM::parameter_type& output_parameter,
							  const automaton_handle<II>& input_handle,
							  IM II::*input_member_ptr) {
    return shared_ptr<bind_executor_interface> (new bind_executor<OI, OM, II, IM> (output_handle, output_member_ptr, output_parameter, input_handle, input_member_ptr));
  }

  template <class OI, class OM, class II, class IM>
  shared_ptr<bind_executor_interface> make_bind_executor (const automaton_handle<OI>& output_handle,
							  OM OI::*output_member_ptr,
							  const automaton_handle<II>& input_handle,
							  IM II::*input_member_ptr,
							  const typename IM::parameter_type& input_parameter) {
    return shared_ptr<bind_executor_interface> (new bind_executor<OI, OM, II, IM> (output_handle, output_member_ptr, input_handle, input_member_ptr, input_parameter));
  }

  template <class OI, class OM, class II, class IM>
  shared_ptr<bind_executor_interface> make_bind_executor (const automaton_handle<OI>& output_handle,
							  OM OI::*output_member_ptr,
							  const typename OM::parameter_type& output_parameter,
							  const automaton_handle<II>& input_handle,
							  IM II::*input_member_ptr,
							  const typename IM::parameter_type& input_parameter) {
    return shared_ptr<bind_executor_interface> (new bind_executor<OI, OM, II, IM> (output_handle, output_member_ptr, output_parameter, input_handle, input_member_ptr, input_parameter));
  }
  
}

#endif
