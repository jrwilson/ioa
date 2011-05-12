#ifndef __action_hpp__
#define __action_hpp__

#include <set>
#include "automaton.hpp"

namespace ioa {

  /*
    Interface Hierarchy

    - action
      - input
        - unvalued
        - valued
      - executable
        - output
          - unvalued
	  - valued
        - independent

    The input and output interfaces are split by type to enable safe composition.
    Namely, unvalued input actions can be composed with unvalued output actions and valued input actiosn can be composed with valued output actions of the same time.

    The executable interface contains output and independent actions.
    This split is necessary because outputs require special processing when they are executed, i.e., they can be composed and involve more than one automaton.
    Independent actions only involve one automaton and include internal actions and events.
    The intended usage of independent actions is that they will be executed after the automaton is locked.

    Events are asynchronous messages from other automata.
    Conceptually, events act like a permanent inbox and are a cross between an internal action and an input action.
    Events allow arbitrary automata to coordinate are are necessary for dynamic interfaces.
  */

  class action_interface
  {
  protected:
    const generic_automaton_handle m_handle;

  public:
    action_interface (const generic_automaton_handle& handle) :
      m_handle (handle)
    { }

    virtual ~action_interface() { }

    virtual const generic_automaton_handle get_automaton_handle () const {
      return m_handle;
    }

    virtual const void* get_member_ptr () const {
      return 0;
    }

    virtual bool is_parameterized () const {
      return false;
    }

    virtual bool involves_parameter (const generic_parameter_handle&) const {
      return false;
    }

    virtual generic_parameter_handle get_parameter_handle () const {
      return generic_parameter_handle ();
    }
  };

  class bindable_interface
  {
  public:
    virtual ~bindable_interface () { }
    virtual void bound () const = 0;
    virtual void unbound () const = 0;
  };

  class input_action_interface :
    public action_interface,
    public bindable_interface
  {
  public:
    input_action_interface (const generic_automaton_handle& handle) :
      action_interface (handle)
    { }

    virtual ~input_action_interface () { }

    virtual bool operator== (const input_action_interface& ia) const = 0;
  };

  class unvalued_input_action_interface :
    public input_action_interface
  {
  public:
    unvalued_input_action_interface (const generic_automaton_handle& handle) :
      input_action_interface (handle)
    { }

    virtual ~unvalued_input_action_interface () { }
    
    virtual void operator() () const = 0;
  };

  template <class T>
  class valued_input_action_interface :
    public input_action_interface
  {
  public:
    valued_input_action_interface (const generic_automaton_handle& handle) :
      input_action_interface (handle)
    { }

    virtual ~valued_input_action_interface () { }
    
    virtual void operator() (const T t) const = 0;
  };

  class executable_action_interface :
    public action_interface
  {
  public:
    executable_action_interface (const generic_automaton_handle& handle) :
      action_interface (handle)
    { }

    virtual ~executable_action_interface () { }

    virtual void execute () const = 0;
  };

  class output_action_interface :
    public executable_action_interface,
    public bindable_interface
  {
  public:
    output_action_interface (const generic_automaton_handle& handle) :
      executable_action_interface (handle)
    { }

    virtual bool operator== (const output_action_interface& oa) const = 0;
  };

  class unvalued_output_action_interface :
    public output_action_interface
  {
  public:
    unvalued_output_action_interface (const generic_automaton_handle& handle) :
      output_action_interface (handle)
    { }

    virtual ~unvalued_output_action_interface () { }

    virtual bool operator() () const = 0;
  };

  template <class T>
  class valued_output_action_interface :
    public output_action_interface
  {
  public:
    valued_output_action_interface (const generic_automaton_handle& handle) :
      output_action_interface (handle)
    { }

    virtual ~valued_output_action_interface () { }

    virtual std::pair<bool, T> operator() () const = 0;
  };

  class independent_action_interface :
    public executable_action_interface
  {
  public:
    independent_action_interface (const generic_automaton_handle& handle) :
      executable_action_interface (handle)
    { }
  };

  template <class I>
  class handle_impl
  {
  private:
    const automaton_handle<I> m_typed_handle;
    
  public:
    handle_impl (const automaton_handle<I> handle) :
      m_typed_handle (handle)
    { }

    const automaton_handle<I> get_typed_automaton_handle () const {
      return m_typed_handle;
    }
    
  };

  // Member.
  template <class I, class Member>
  class member_ref
  {
  protected:
    Member& m_member;
    
  public:
    member_ref (const automaton_handle<I>& handle,
		Member I::*member_ptr) :
      m_member ((*(handle.value ())).*member_ptr)
    { }
  };
  
  // Parameter.
  template <class T>
  class param
  {
  protected:
    const parameter_handle<T> m_parameter;

  public:
    param (const parameter_handle<T>& parameter) :
      m_parameter (parameter)
    { }
  };

  class null_type { };

  struct input_category { };
  struct output_category { };
  struct internal_category { };
  struct event_category { };

  struct unvalued { };
  struct valued { };

  struct unparameterized { };
  struct parameterized { };

  struct no_value {
    typedef unvalued value_status;
    typedef null_type value_type;
  };

  template <class T>
  struct value {
    typedef valued value_status;
    typedef T value_type;
  };

  struct no_parameter {
    typedef unparameterized parameter_status;
    typedef null_type parameter_type;
  };

  template <class T>
  struct parameter {
    typedef parameterized parameter_status;
    typedef T parameter_type;
  };

  struct input {
    typedef input_category action_category;
  };

  struct output {
    typedef output_category action_category;
  };

  struct internal : public no_value {
    typedef internal_category action_category;
  };

  struct event : public no_parameter {
    typedef event_category action_category;
  };

  /*
    K - category
    I - instance
    M - member
    VS - value status
    VT - value type
    PS - parameter status
    PT - parameter type
  */
  template <class K, class I, class M, class VS, class VT, class PS, class PT> class action_impl;

  template <class I, class M>
  class action_impl<input_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public unvalued_input_action_interface,
    public handle_impl<I>,
    private member_ref<I, M>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
		 M I::*member_ptr) :
      unvalued_input_action_interface (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr)
    { }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }
    
    const void* get_member_ptr () const {
      return &this->m_member;
    }
    
    void operator() () const {
      this->m_member ();
    }

    void bound () const {
      this->m_member.bound ();
    }

    void unbound () const {
      this->m_member.unbound ();
    }

  };

  template <class I, class M, class PT>
  class action_impl<input_category, I, M, unvalued, null_type, parameterized, PT> : 
    public unvalued_input_action_interface,
    public handle_impl<I>,
    private member_ref<I, M>,
    private param<PT>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr,
		 const parameter_handle<PT>& parameter) :
      unvalued_input_action_interface (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr),
      param<PT> (parameter)
    { }

    generic_parameter_handle get_parameter_handle () const {
      return this->m_parameter;
    }

    bool is_parameterized () const {
      return true;
    }

    bool involves_parameter (const generic_parameter_handle& parameter) const {
      return this->m_parameter == parameter;
    }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void operator() () const {
      this->m_member (this->m_parameter.value ());
    }

    void bound () const {
      this->m_member.bound (this->m_parameter.value ());
    }

    void unbound () const {
      this->m_member.unbound (this->m_parameter.value ());
    }

  };

  template <class I, class M, class VT>
  class action_impl<input_category, I, M, valued, VT, unparameterized, null_type> : 
    public valued_input_action_interface<VT>,
    public handle_impl<I>,
    private member_ref<I, M>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr) :
      valued_input_action_interface<VT> (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr)
    { }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void operator() (const VT t) const {
      this->m_member (t);
    }

    void bound () const {
      this->m_member.bound ();
    }

    void unbound () const {
      this->m_member.unbound ();
    }

  };

  template <class I, class M, class VT, class PT>
  class action_impl<input_category, I, M, valued, VT, parameterized, PT> :
    public valued_input_action_interface<VT>,
    public handle_impl<I>,
    private member_ref<I, M>,
    private param<PT>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr,
		 const parameter_handle<PT>& parameter) :
      valued_input_action_interface<VT> (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr),
      param<PT> (parameter)
    { }

    generic_parameter_handle get_parameter_handle () const {
      return this->m_parameter;
    }

    bool is_parameterized () const {
      return true;
    }

    bool involves_parameter (const generic_parameter_handle& parameter) const {
      return this->m_parameter == parameter;
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter);
    }

    void operator() (const VT t) const {
      this->m_member (t, this->m_parameter.value ());
    }

    void bound () const {
      this->m_member.bound (this->m_parameter.value ());
    }

    void unbound () const {
      this->m_member.unbound (this->m_parameter.value ());
    }

  };

  template <class I, class M>
  class action_impl<output_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public unvalued_output_action_interface,
    public handle_impl<I>,
    private member_ref<I, M>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr) :
      unvalued_output_action_interface (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr)
    { }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool operator() () const {
      return this->m_member ();
    }

    void execute () const {
      (*this) ();
    }

    void bound () const {
      this->m_member.bound ();
    }

    void unbound () const {
      this->m_member.unbound ();
    }

  };

  template <class I, class M, class PT>
  class action_impl<output_category, I, M, unvalued, null_type, parameterized, PT> :
    public unvalued_output_action_interface,
    public handle_impl<I>,
    private member_ref<I, M>,
    private param<PT>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr,
		 const parameter_handle<PT>& parameter) :
      unvalued_output_action_interface (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr),
      param<PT> (parameter)
    { }

    generic_parameter_handle get_parameter_handle () const {
      return this->m_parameter;
    }

    bool is_parameterized () const {
      return true;
    }

    bool involves_parameter (const generic_parameter_handle& parameter) const {
      return this->m_parameter == parameter;
    }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool operator() () const {
      return this->m_member (this->m_parameter.value ());
    }

    void execute () const {
      (*this) ();
    }

    void bound () const {
      this->m_member.bound (this->m_parameter.value ());
    }

    void unbound () const {
      this->m_member.unbound (this->m_parameter.value ());
    }
    
  };

  template <class I, class M, class VT>
  class action_impl<output_category, I, M, valued, VT, unparameterized, null_type> :
    public valued_output_action_interface<VT>,
    public handle_impl<I>,
    private member_ref<I, M>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr) :
      valued_output_action_interface<VT> (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr)
    { }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    std::pair<bool, VT> operator() () const {
      return this->m_member ();
    }

    void execute () const {
      (*this) ();
    }

    void bound () const {
      this->m_member.bound ();
    }

    void unbound () const {
      this->m_member.unbound ();
    }

  };

  template <class I, class M, class VT, class PT>
  class action_impl<output_category, I, M, valued, VT, parameterized, PT> :
    public valued_output_action_interface<VT>,
    public handle_impl<I>,
    private member_ref<I, M>,
    private param<PT>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr,
		 const parameter_handle<PT>& parameter) :
      valued_output_action_interface<VT> (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr),
      param<PT> (parameter)
    { }

    generic_parameter_handle get_parameter_handle () const {
      return this->m_parameter;
    }

    bool is_parameterized () const {
      return true;
    }

    bool involves_parameter (const generic_parameter_handle& parameter) const {
      return this->m_parameter == parameter;
    }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    std::pair<bool, VT> operator() () const {
      return this->m_member (this->m_parameter.value ()); 
    }

    void execute () const {
      (*this) ();
    }

    void bound () const {
      this->m_member.bound (this->m_parameter.value ());
    }

    void unbound () const {
      this->m_member.unbound (this->m_parameter.value ());
    }

  };

  template <class I, class M>
  class action_impl<internal_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public independent_action_interface,
    public handle_impl<I>,
    private member_ref<I, M>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr) :
      independent_action_interface (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr)
    { }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void execute () const {
      this->m_member ();
    }
  };

  template <class I, class M, class PT>
  class action_impl<internal_category, I, M, unvalued, null_type, parameterized, PT> :
    public independent_action_interface,
    public handle_impl<I>,
    private member_ref<I, M>,
    private param<PT>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
		 M I::*member_ptr,
		 const parameter_handle<PT>& parameter) :
      independent_action_interface (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr),
      param<PT> (parameter)
    { }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool is_parameterized () const {
      return true;
    }

    bool involves_parameter (const generic_parameter_handle& parameter) const {
      return this->m_parameter == parameter;
    }

    generic_parameter_handle get_parameter_handle () const {
      return this->m_parameter;
    }

    void execute () const {
      this->m_member (this->m_parameter.value ());
    }
    
  };

  template <class I, class M>
  class action_impl<event_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public independent_action_interface,
    public handle_impl<I>,
    private member_ref<I, M>
  {
  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr) :
      independent_action_interface (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr)
    { }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void execute () const {
      this->m_member ();
    }    
  };

  template <class I, class M, class VT>
  class action_impl<event_category, I, M, valued, VT, unparameterized, null_type> :
    public independent_action_interface,
    public handle_impl<I>,
    private member_ref<I, M>
  {
  private:
    const VT m_t;

  public:
    action_impl (const automaton_handle<I>& handle,
  		 M I::*member_ptr,
  		 const VT& t) :
      independent_action_interface (handle),
      handle_impl<I> (handle),
      member_ref<I, M> (handle, member_ptr),
      m_t (t)
    { }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void execute () const {
      this->m_member (m_t);
    }    
  };

  // template <class M, class VT>
  // class action_impl<system_event_category, M, valued, VT, unparameterized, null_type> :
  //   public independent_action_interface,
  //   private member_ref<M>
  // {
  // private:
  //   const VT m_t;

  // public:
  //   action_impl (const generic_automaton_handle handle,
  // 		 M& m,
  // 		 const VT& t) :
  //     independent_action_interface (handle),
  //     member_ref<M> (m),
  //     m_t (t)
  //   { }

  //   const void* get_member_ptr () const {
  //     return &this->m_member;
  //   }

  //   void execute () const {
  //     this->m_member (m_t);
  //   }    
  // };
  
  template <class I, class Member>
  class action :
    public action_impl<typename Member::action_category,
		       I,
		       Member,
  		       typename Member::value_status,
  		       typename Member::value_type,
  		       typename Member::parameter_status,
  		       typename Member::parameter_type>
  {
  public:
    typedef typename Member::action_category action_category;
    typedef typename Member::value_status value_status;
    typedef typename Member::value_type value_type;
    typedef typename Member::parameter_status parameter_status;
    typedef typename Member::parameter_type parameter_type;

    action (const automaton_handle<I>& handle,
  	    Member I::*member_ptr) :
      action_impl<action_category,
		  I,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member_ptr)
    { }

    action (const automaton_handle<I>& handle,
  	    Member I::*member_ptr,
	    const parameter_handle<parameter_type>& parameter) :
      action_impl<action_category,
		  I,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member_ptr, parameter)
    { }
    
    action (const automaton_handle<I>& handle,
  	    Member I::*member_ptr,
	    const value_type& value) :
      action_impl<action_category,
		  I,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member_ptr, value)
    { }

  };

  template <class I, class M>
  action<I, M>
  make_action (const automaton_handle<I>& handle,
	       M I::*member_ptr) {
    return action<I, M> (handle, member_ptr);
  }

  template <class I, class M>
  action<I, M>
  make_action (const automaton_handle<I>& handle,
	       M I::*member_ptr,
	       const parameter_handle<typename M::parameter_type>& parameter) {
    return action<I, M> (handle, member_ptr, parameter);
  }

  template <class I, class M>
  action<I, M>
  make_action (const automaton_handle<I>& handle,
	       M I::*member_ptr,
	       const typename M::value_type& v) {
    return action<I, M> (handle, member_ptr, v);
  }


}

#endif
