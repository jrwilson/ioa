#ifndef __action_executor_hpp__
#define __action_executor_hpp__

#include <cassert>
#include <ioa/mutex.hpp>
#include <ioa/automaton_handle.hpp>
#include <ioa/action.hpp>
#include <ioa/scheduler_interface.hpp>

namespace ioa {

  class action_executor_interface
  {
  public:
    virtual ~action_executor_interface () { }
    virtual aid_t get_aid () const = 0;
    virtual void* get_member_ptr () const = 0;
    virtual void* get_pid () const = 0;

    virtual bool operator== (const action_executor_interface& x) const {
      return
  	get_aid () == x.get_aid () &&
  	get_member_ptr () == x.get_member_ptr () &&
  	get_pid () == x.get_pid ();
    }

    bool operator!= (const action_executor_interface& x) const {
      return !(*this == x);
    }

    bool operator< (const action_executor_interface& x) const {
      if (get_aid () != x.get_aid ()) {
	return get_aid () < x.get_aid ();
      }
      if (get_member_ptr () != x.get_member_ptr ()) {
	return get_member_ptr () < x.get_member_ptr ();
      }
      return get_pid () < x.get_pid ();
    }

  };

  class input_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~input_executor_interface () { }
    virtual input_executor_interface* clone () const = 0;
    virtual void set_auto_parameter (const aid_t) = 0;
    virtual void lock () = 0;
    virtual void unlock () = 0;
  };

  class unvalued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~unvalued_input_executor_interface () { }
    virtual void operator() (scheduler_interface&) const = 0;
  };

  template <typename T>
  class valued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~valued_input_executor_interface () { }
    virtual void operator() (scheduler_interface&, const T& t) const = 0;
  };

  class local_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~local_executor_interface () { }
    //virtual void operator() (scheduler_interface&) const = 0;
  };
  
  class output_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~output_executor_interface () { }
    virtual output_executor_interface* clone () const = 0;
    virtual void set_auto_parameter (const aid_t) = 0;
  };

  class unvalued_output_executor_interface :
    public output_executor_interface
  {
  public:
    virtual ~unvalued_output_executor_interface () { }
  };

  template <typename T>
  class valued_output_executor_interface :
    public output_executor_interface
  {
  public:
    virtual ~valued_output_executor_interface () { }
  };
  
  class internal_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~internal_executor_interface () { }
  };
    
  template <class I, class M>
  class action_executor_core
  {
  protected:
    I& m_instance;
    mutex& m_mutex;
    const automaton_handle<I> m_handle;
    M I::*m_member_ptr;

  public:
    action_executor_core (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      m_instance (instance),
      m_mutex (mutex),
      m_handle (handle),
      m_member_ptr (member_ptr)
    { }

    aid_t get_aid () const {
      return m_handle;
    }

    void* get_member_ptr () const {
      return &(m_instance.*m_member_ptr);
    }

    void lock () {
      m_mutex.lock ();
    }

    void unlock () {
      m_mutex.unlock ();
    }

    template <class Iterator>
    void lock_in_order (Iterator begin,
			Iterator end) {
      bool output_processed = false;
      for (Iterator iter = begin; iter != end; ++iter) {
	if (!output_processed && get_aid () < (*iter)->get_aid ()) {
	  lock ();
	  output_processed = true;
	}
	(*iter)->lock ();
      }
      if (!output_processed) {
	lock ();
      }
    }

    template <class Iterator>
    void unlock_in_order (Iterator begin,
			  Iterator end) {
      bool output_processed = false;
      for (Iterator iter = begin; iter != end; ++iter) {
	if (!output_processed && get_aid () < (*iter)->get_aid ()) {
	  unlock ();
	  output_processed = true;
	}
	(*iter)->unlock ();
      }
      if (!output_processed) {
	unlock ();
      }
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
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr)
    { }
      
    void operator() (scheduler_interface& system_scheduler) const {
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance);
      ((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance));
      system_scheduler.clear_current_aid ();
    }

    void set_auto_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    input_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return 0;
    }

    void lock () {
      action_executor_core<I,M>::lock ();
    }

    void unlock () {
      action_executor_core<I,M>::unlock ();
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
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (parameter)
    { }

    void operator() (scheduler_interface& system_scheduler) const {
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, m_parameter);
      ((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
      system_scheduler.clear_current_aid ();
    }

    void set_auto_parameter (const aid_t) {
      // Not auto_parameterized.
    }
    
    input_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
    }

    void lock () {
      action_executor_core<I,M>::lock ();
    }

    void unlock () {
      action_executor_core<I,M>::unlock ();
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
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (-1)
    { }

    void set_auto_parameter (const aid_t aid) {
      m_parameter = aid;
    }
      
    void operator() (scheduler_interface& system_scheduler) const {
      assert (m_parameter != -1);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, m_parameter);
      ((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
      system_scheduler.clear_current_aid ();
    }
    
    input_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
    }

    void lock () {
      action_executor_core<I,M>::lock ();
    }

    void unlock () {
      action_executor_core<I,M>::unlock ();
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
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr)
    { }
      
    void operator() (scheduler_interface& system_scheduler,
		     const VT& t) const {
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, t);
      ((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance));
      system_scheduler.clear_current_aid ();
    }

    void set_auto_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    input_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return 0;
    }

    void lock () {
      action_executor_core<I,M>::lock ();
    }

    void unlock () {
      action_executor_core<I,M>::unlock ();
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
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (parameter)
    { }
      
    void operator() (scheduler_interface& system_scheduler,
		     const VT& t) const {
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, t, m_parameter);
      ((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
      system_scheduler.clear_current_aid ();
    }

    void set_auto_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    input_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }      

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
    }

    void lock () {
      action_executor_core<I,M>::lock ();
    }

    void unlock () {
      action_executor_core<I,M>::unlock ();
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
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (-1)
    { }

    void set_auto_parameter (const aid_t aid) {
      m_parameter = aid;
    }
      
    void operator() (scheduler_interface& system_scheduler,
		     const VT& t) const {
      assert (m_parameter != -1);
      system_scheduler.set_current_aid (this->m_handle);
      ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, t, m_parameter);
      ((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
      system_scheduler.clear_current_aid ();
    }

    input_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }      

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
    }

    void lock () {
      action_executor_core<I,M>::lock ();
    }

    void unlock () {
      action_executor_core<I,M>::unlock ();
    }
  };
    
  template <class I, class M>
  class action_executor_impl<I, M, output_category, unvalued, null_type, unparameterized, null_type> :
    public unvalued_output_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other)
    { }

  public:
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr)
    { }

    template<typename Iterator>
    void operator() (scheduler_interface& system_scheduler,
		     Iterator begin,
		     Iterator end) {
      // Lock in order.
      this->lock_in_order (begin, end);
      
      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance).*(this->m_member_ptr)).precondition (const_cast<const I&> (this->m_instance))) {
	((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance);
	((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance));
	system_scheduler.clear_current_aid ();
	for (Iterator iter = begin; iter != end; ++iter) {
	  unvalued_input_executor_interface& input = dynamic_cast<unvalued_input_executor_interface&> (*(*iter));
	  input (system_scheduler);
	}
      }
      else {
	system_scheduler.clear_current_aid ();
      }
      
      // Unlock.
      this->unlock_in_order (begin, end);
    }

    void set_auto_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return 0;
    }
  };

  template <class I, class M, class PT>
  class action_executor_impl<I, M, output_category, unvalued, null_type, parameterized, PT> :
    public unvalued_output_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    PT m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other),
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (parameter)
    { }

    template<typename Iterator>
    void operator() (scheduler_interface& system_scheduler,
		     Iterator begin,
		     Iterator end) {
      // Lock in order.
      this->lock_in_order (begin, end);
      
      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance).*(this->m_member_ptr)).precondition (const_cast<const I&> (this->m_instance), m_parameter)) {
	((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, m_parameter);
	((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
	system_scheduler.clear_current_aid ();
	for (Iterator iter = begin; iter != end; ++iter) {
	  unvalued_input_executor_interface& input = dynamic_cast<unvalued_input_executor_interface&> (*(*iter));
	  input (system_scheduler);
	}
      }
      else {
	system_scheduler.clear_current_aid ();
      }
      
      // Unlock.
      this->unlock_in_order (begin, end);
    }

    void set_auto_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }
      
    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
    }
  };

  template <class I, class M>
  class action_executor_impl<I, M, output_category, unvalued, null_type, auto_parameterized, aid_t> :
    public unvalued_output_executor_interface,
    private action_executor_core<I, M>
  {
  private:
    aid_t m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other),
      m_parameter (other.m_parameter)
    { }

  public:
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (-1)
    { }

    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const aid_t aid) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (aid)
    { }

    void set_auto_parameter (const aid_t aid) {
      m_parameter = aid;
    }

    template <class Iterator>
    void operator() (scheduler_interface& system_scheduler,
		     Iterator begin,
		     Iterator end) {
      assert (m_parameter != -1);

      // Lock in order.
      this->lock_in_order (begin, end);
      
      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance).*(this->m_member_ptr)).precondition (const_cast<const I&> (this->m_instance), m_parameter)) {
	((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, m_parameter);
	((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
	system_scheduler.clear_current_aid ();
	for (Iterator iter = begin; iter != end; ++iter) {
	  unvalued_input_executor_interface& input = dynamic_cast<unvalued_input_executor_interface&> (*(*iter));
	  input (system_scheduler);
	}
      }
      else {
	system_scheduler.clear_current_aid ();
      }
      
      // Unlock.
      this->unlock_in_order (begin, end);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }
      
    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
    }
  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, output_category, valued, VT, unparameterized, null_type> :
    public valued_output_executor_interface<VT>,
    private action_executor_core<I, M>
  {
  private:
    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other)
    { }

  public:
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr)
    { }

    void set_auto_parameter (const aid_t) {
      // Not auto_parameterized.
    }
    
    template <class Iterator>
    void operator() (scheduler_interface& system_scheduler,
		     Iterator begin,
		     Iterator end) {
      // Lock in order.
      this->lock_in_order (begin, end);
      
      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance).*(this->m_member_ptr)).precondition (const_cast<const I&> (this->m_instance))) {
	VT value = ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance);
	const VT& ref = value;
	((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance));
	system_scheduler.clear_current_aid ();
	for (Iterator iter = begin; iter != end; ++iter) {
	  valued_input_executor_interface<VT>& input = dynamic_cast<valued_input_executor_interface<VT>&> (*(*iter));
	  input (system_scheduler, ref);
	}
      }
      else {
	system_scheduler.clear_current_aid ();
      }
      
      // Unlock.
      this->unlock_in_order (begin, end);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return 0;
    }
  };

  template <class I, class M, class VT, class PT>
  class action_executor_impl<I, M, output_category, valued, VT, parameterized, PT> :
    public valued_output_executor_interface<VT>,
    private action_executor_core<I, M>
  {
  private:
    PT m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other),
      m_parameter (other.m_parameter)
    { }
    
  public:
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const PT& parameter) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (parameter)
    { }

    void set_auto_parameter (const aid_t) {
      // Not auto_parameterized.
    }

    template <class Iterator>
    void operator() (scheduler_interface& system_scheduler,
		     Iterator begin,
		     Iterator end) {
      // Lock in order.
      this->lock_in_order (begin, end);
      
      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance).*(this->m_member_ptr)).precondition (const_cast<const I&> (this->m_instance), m_parameter)) {
	VT value = ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, m_parameter);
	const VT& ref = value;
	((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
	system_scheduler.clear_current_aid ();
	for (Iterator iter = begin; iter != end; ++iter) {
	  valued_input_executor_interface<VT>& input = dynamic_cast<valued_input_executor_interface<VT>&> (*(*iter));
	  input (system_scheduler, ref);
	}
      }
      else {
	system_scheduler.clear_current_aid ();
      }
      
      // Unlock.
      this->unlock_in_order (begin, end);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
    }
  };

  template <class I, class M, class VT>
  class action_executor_impl<I, M, output_category, valued, VT, auto_parameterized, aid_t> :
    public valued_output_executor_interface<VT>,
    private action_executor_core<I, M>
  {
  private:
    aid_t m_parameter;

    action_executor_impl (const action_executor_impl& other) :
      action_executor_core<I, M> (other),
      m_parameter (other.m_parameter)
    { }
    
  public:
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (-1)
    { }

    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const aid_t aid) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (aid)
    { }

    void set_auto_parameter (const aid_t aid) {
      m_parameter = aid;
    }
    
    template <class Iterator>
    void operator() (scheduler_interface& system_scheduler,
		     Iterator begin,
		     Iterator end) {
      assert (m_parameter != -1);

      // Lock in order.
      this->lock_in_order (begin, end);
      
      // Execute.
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance).*(this->m_member_ptr)).precondition (const_cast<const I&> (this->m_instance), m_parameter)) {
	VT value = ((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, m_parameter);
	const VT& ref = value;
	((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
	system_scheduler.clear_current_aid ();
	for (Iterator iter = begin; iter != end; ++iter) {
	  valued_input_executor_interface<VT>& input = dynamic_cast<valued_input_executor_interface<VT>&> (*(*iter));
	  input (system_scheduler, ref);
	}
      }
      else {
	system_scheduler.clear_current_aid ();
      }
      
      // Unlock.
      this->unlock_in_order (begin, end);
    }

    output_executor_interface* clone () const {
      return new action_executor_impl (*this);
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
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
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr)
    { }
      
    void operator() (scheduler_interface& system_scheduler) {
      action_executor_core<I,M>::lock ();
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance).*(this->m_member_ptr)).precondition (const_cast<const I&> (this->m_instance))) {
	((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance);
	((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance));
      }
      system_scheduler.clear_current_aid ();
      action_executor_core<I,M>::unlock ();
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
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
    action_executor_impl (I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
  			  M I::*member_ptr,
  			  const PT& parameter) :
      action_executor_core<I, M> (instance, mutex, handle, member_ptr),
      m_parameter (parameter)
    { }
      
    void operator() (scheduler_interface& system_scheduler) {
      // Not bound.
      action_executor_core<I,M>::lock ();
      system_scheduler.set_current_aid (this->m_handle);
      if (((this->m_instance).*(this->m_member_ptr)).precondition (const_cast<const I&> (this->m_instance), m_parameter)) {
	((this->m_instance).*(this->m_member_ptr)).effect (this->m_instance, m_parameter);
	((this->m_instance).*(this->m_member_ptr)).schedule (const_cast<const I&> (this->m_instance), m_parameter);
      }
      system_scheduler.clear_current_aid ();
      action_executor_core<I,M>::unlock ();
    }

    aid_t get_aid () const {
      return action_executor_core<I,M>::get_aid ();
    }

    void* get_member_ptr () const {
      return action_executor_core<I,M>::get_member_ptr ();
    }

    void* get_pid () const {
      return reinterpret_cast<void*> (m_parameter);
    }
  };

  template <class I, class M>
  class action_executor :
    public action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type, typename M::parameter_status, typename M::parameter_type>
  {
  public:
    action_executor (I& instance,
		     mutex& mutex,
		     const automaton_handle<I>& handle,
		     M I::*member_ptr) :
      action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type, typename M::parameter_status, typename M::parameter_type> (instance, mutex, handle, member_ptr)
    { }

    action_executor (I& instance,
		     mutex& mutex,
		     const automaton_handle<I>& handle,
		     M I::*member_ptr,
		     const typename M::parameter_type& parameter) :
      action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type, typename M::parameter_status, typename M::parameter_type> (instance, mutex, handle, member_ptr, parameter)
    { }
  };
  
}

#endif
