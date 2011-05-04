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

    const generic_automaton_handle get_automaton_handle () const {
      return m_handle;
    }

    void lock_automaton () {
      m_handle.value ()->lock ();
    }

    void unlock_automaton () {
      m_handle.value ()->unlock ();
    }
  };

  // std::ostream& operator<<(std::ostream& output, const action_interface&);

  class parameter_action_interface
  {
  public:
    virtual ~parameter_action_interface () { }
    virtual const void* get_member_ptr () const = 0;
    virtual bool is_parameterized () const = 0;
    virtual bool involves_parameter (const generic_parameter_handle&) const = 0;
    virtual bool parameter_exists () const = 0;
    virtual generic_parameter_handle get_parameter_handle () const = 0;
  };

  class input_action_interface :
    public action_interface,
    public parameter_action_interface
  {
  private:
    const generic_automaton_handle m_binder;
    
  public:
    input_action_interface (const generic_automaton_handle& handle,
  			    const generic_automaton_handle& binder) :
      action_interface (handle),
      m_binder (binder)
    { }
    
    virtual ~input_action_interface () { }

    const generic_automaton_handle get_binder_handle () const
    {
      return m_binder;
    }

    virtual bool operator== (const input_action_interface& ia) const = 0;
  };

  class unvalued_input_action_interface :
    public input_action_interface
  {
  public:
    unvalued_input_action_interface (const generic_automaton_handle& handle,
  				    const generic_automaton_handle& binder) :
      input_action_interface (handle, binder)
    { }
    
    virtual ~unvalued_input_action_interface () { }
    
    virtual void operator() () = 0;
  };

  template <class T>
  class valued_input_action_interface :
    public input_action_interface
  {
  public:
    valued_input_action_interface (const generic_automaton_handle& handle,
  				  const generic_automaton_handle& binder) :
      input_action_interface (handle, binder)
    { }
    
    virtual ~valued_input_action_interface () { }
    
    virtual void operator() (const T t) = 0;
  };

  class executable_action_interface :
    public action_interface
  {
  public:
    executable_action_interface (const generic_automaton_handle& handle) :
      action_interface (handle)
    { }

    virtual ~executable_action_interface () { }

    virtual void execute () = 0;
  };

  class output_action_interface :
    public executable_action_interface,
    public parameter_action_interface
  {
  public:
    output_action_interface (const generic_automaton_handle& handle) :
      executable_action_interface (handle)
    { }

    virtual bool operator== (const output_action_interface& oa) const = 0;
    
    virtual const void* get_member_ptr () const = 0;
  };

  class unvalued_output_action_interface :
    public output_action_interface
  {
  public:
    unvalued_output_action_interface (const generic_automaton_handle& handle) :
      output_action_interface (handle)
    { }

    virtual ~unvalued_output_action_interface () { }

    virtual bool operator() () = 0;
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

    virtual std::pair<bool, T> operator() () = 0;
  };

  class independent_action_interface :
    public executable_action_interface
  {
  public:
    independent_action_interface (const generic_automaton_handle& handle) :
      executable_action_interface (handle)
    { }
  };

  // Member.
  template <class Member>
  class member_ref
  {
  protected:
    Member& m_member;
    
  public:
    member_ref (Member& m) :
      m_member (m)
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
    M - member
    VS - value status
    VT - value type
    PS - parameter status
    PT - parameter type
  */
  template <class K, class M, class VS, class VT, class PS, class PT> class action_impl;

  template <class M>
  class action_impl<input_category, M, unvalued, null_type, unparameterized, null_type> :
    public unvalued_input_action_interface,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
		 M& m,
  		 const generic_automaton_handle& binder) :
      unvalued_input_action_interface (handle, binder),
      member_ref<M> (m)
    { }

    generic_parameter_handle get_parameter_handle () const {
      return generic_parameter_handle ();
    }

    bool is_parameterized () const {
      return false;
    }

    bool involves_parameter (const generic_parameter_handle&) const {
      return false;
    }

    bool parameter_exists () const {
      return true;
    }
    
    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }
    
    const void* get_member_ptr () const {
      return &this->m_member;
    }
    
    void operator() () {
      this->m_member ();
    }
  };

  template <class M, class PT>
  class action_impl<input_category, M, unvalued, null_type, parameterized, PT> : 
    public unvalued_input_action_interface,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
		 const parameter_handle<PT>& parameter,
  		 const generic_automaton_handle& binder) :
      unvalued_input_action_interface (handle, binder),
      member_ref<M> (m),
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

    bool parameter_exists () const {
      return this->m_handle.value ()->parameter_exists (this->m_parameter.value ());
    }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void operator() () {
      this->m_member (this->m_parameter.value ());
    }

  };

  template <class M, class VT>
  class action_impl<input_category, M, valued, VT, unparameterized, null_type> : 
    public valued_input_action_interface<VT>,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
  		 const generic_automaton_handle& binder) :
      valued_input_action_interface<VT> (handle, binder),
      member_ref<M> (m)
    { }

    generic_parameter_handle get_parameter_handle () const {
      return generic_parameter_handle ();
    }

    bool is_parameterized () const {
      return false;
    }

    bool involves_parameter (const generic_parameter_handle& parameter) const {
      return false;
    }

    bool parameter_exists () const {
      return true;
    }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void operator() (const VT t) {
      this->m_member (t);
    }

  };

  template <class M, class VT, class PT>
  class action_impl<input_category, M, valued, VT, parameterized, PT> :
    public valued_input_action_interface<VT>,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
		 const parameter_handle<PT>& parameter,
  		 const generic_automaton_handle& binder) :
      valued_input_action_interface<VT> (handle, binder),
      member_ref<M> (m),
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

    bool parameter_exists () const {
      return this->m_handle.value ()->parameter_exists (this->m_parameter.value ());
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter);
    }

    void operator() (const VT t) {
      this->m_member (t, this->m_parameter.value ());
    }

  };

  template <class M>
  class action_impl<output_category, M, unvalued, null_type, unparameterized, null_type> :
    public unvalued_output_action_interface,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m) :
      unvalued_output_action_interface (handle),
      member_ref<M> (m)
    { }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    generic_parameter_handle get_parameter_handle () const {
      return generic_parameter_handle ();
    }

    bool is_parameterized () const {
      return false;
    }

    bool involves_parameter (const generic_parameter_handle& parameter) const {
      return false;
    }

    bool parameter_exists () const {
      return true;
    }

    bool operator() () {
      return this->m_member ();
    }

    void execute () {
      (*this) ();
    }
  };

  template <class M, class PT>
  class action_impl<output_category, M, unvalued, null_type, parameterized, PT> :
    public unvalued_output_action_interface,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
		 const parameter_handle<PT>& parameter) :
      unvalued_output_action_interface (handle),
      member_ref<M> (m),
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

    bool parameter_exists () const {
      return this->m_handle.value ()->parameter_exists (this->m_parameter.value ());
    }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool operator() () {
      return this->m_member (this->m_parameter.value ());
    }

    void execute () {
      (*this) ();
    }
    
  };

  template <class M, class VT>
  class action_impl<output_category, M, valued, VT, unparameterized, null_type> :
    public valued_output_action_interface<VT>,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m) :
      valued_output_action_interface<VT> (handle),
      member_ref<M> (m)
    { }

    generic_parameter_handle get_parameter_handle () const {
      return generic_parameter_handle ();
    }

    bool is_parameterized () const {
      return false;
    }

    bool involves_parameter (const generic_parameter_handle& parameter) const {
      return false;
    }

    bool parameter_exists () const {
      return true;
    }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    std::pair<bool, VT> operator() () {
      return this->m_member ();
    }

    void execute () {
      (*this) ();
    }

  };

  template <class M, class VT, class PT>
  class action_impl<output_category, M, valued, VT, parameterized, PT> :
    public valued_output_action_interface<VT>,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
		 const parameter_handle<PT>& parameter) :
      valued_output_action_interface<VT> (handle),
      member_ref<M> (m),
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

    bool parameter_exists () const {
      return this->m_handle.value ()->parameter_exists (this->m_parameter.value ());
    }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    std::pair<bool, VT> operator() () {
      return this->m_member (this->m_parameter.value ()); 
    }

    void execute () {
      (*this) ();
    }

  };

  template <class M>
  class action_impl<internal_category, M, unvalued, null_type, unparameterized, null_type> :
    public independent_action_interface,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m) :
      independent_action_interface (handle),
      member_ref<M> (m)
    { }

    void execute () {
      this->m_member ();
    }
  };

  template <class M, class PT>
  class action_impl<internal_category, M, unvalued, null_type, parameterized, PT> :
    public independent_action_interface,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
		 M& m,
		 const parameter_handle<PT>& parameter) :
      independent_action_interface (handle),
      member_ref<M> (m),
      param<PT> (parameter)
    { }

    void execute () {
      this->m_member (this->m_parameter.value ());
    }
    
  };

  template <class M>
  class action_impl<event_category, M, unvalued, null_type, unparameterized, null_type> :
    public independent_action_interface,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle handle,
  		 M& m) :
      independent_action_interface (handle),
      member_ref<M> (m)
    { }

    void execute () {
      this->m_member ();
    }    
  };

  template <class M, class VT>
  class action_impl<event_category, M, valued, VT, unparameterized, null_type> :
    public independent_action_interface,
    private member_ref<M>
  {
  private:
    const VT m_t;

  public:
    action_impl (const generic_automaton_handle handle,
  		 M& m,
  		 const VT& t) :
      independent_action_interface (handle),
      member_ref<M> (m),
      m_t (t)
    { }

    void execute () {
      this->m_member (m_t);
    }    
  };
  
  template <class Member>
  class action :
    public action_impl<typename Member::action_category,
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

    action (const generic_automaton_handle& handle,
  	    Member& member) :
      action_impl<action_category,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member)
    { }

    action (const generic_automaton_handle& handle,
  	    Member& member,
	    const parameter_handle<parameter_type>& parameter) :
      action_impl<action_category,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member, parameter)
    { }
    
    action (const generic_automaton_handle& handle,
  	    Member& member,
  	    const generic_automaton_handle& binder) :
      action_impl<action_category,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member, binder)
    { }

    action (const generic_automaton_handle& handle,
  	    Member& member,
	    const parameter_handle<parameter_type>& parameter,
  	    const generic_automaton_handle& binder) :
      action_impl<action_category,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member, parameter, binder)
    { }

    action (const generic_automaton_handle& handle,
  	    Member& member,
	    const value_type& value) :
      action_impl<action_category,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member, value)
    { }

  };

}

#endif
