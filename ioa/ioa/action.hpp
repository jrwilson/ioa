#ifndef __action_hpp__
#define __action_hpp__

#include <set>
#include "automaton.hpp"

namespace ioa {

  /*
    Interface Hierarchy

    - action
      - input
        - untyped
        - typed
      - executable
        - output
          - untyped
	  - typed
        - independent

    The input and output interfaces are split by type to enable safe composition.
    Namely, untyped input actions can be composed with untyped output actions and typed input actiosn can be composed with typed output actions of the same time.

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
  private:
    const generic_automaton_handle m_handle;

  public:
    action_interface (const generic_automaton_handle& handle) :
      m_handle (handle)
    { }

    const generic_automaton_handle get_automaton_handle () const {
      return m_handle;
    }
  };

  // std::ostream& operator<<(std::ostream& output, const action_interface&);

  class input_action_interface :
    public action_interface
  {
  private:
    const generic_automaton_handle m_composer;
    
  public:
    input_action_interface (const generic_automaton_handle& handle,
  			    const generic_automaton_handle& composer) :
      action_interface (handle),
      m_composer (composer)
    { }
    
    virtual ~input_action_interface () { }

    const generic_automaton_handle get_composer_handle () const
    {
      return m_composer;
    }

    virtual bool operator== (const input_action_interface& ia) const = 0;

    virtual const void* get_member_ptr () const = 0;
    
    virtual bool involves_parameter (const void*) const = 0;
  };

  class untyped_input_action_interface :
    public input_action_interface
  {
  public:
    untyped_input_action_interface (const generic_automaton_handle& handle,
  				    const generic_automaton_handle& composer) :
      input_action_interface (handle, composer)
    { }
    
    virtual ~untyped_input_action_interface () { }
    
    virtual void operator() () = 0;
  };

  template <class T>
  class typed_input_action_interface :
    public input_action_interface
  {
  public:
    typed_input_action_interface (const generic_automaton_handle& handle,
  				  const generic_automaton_handle& composer) :
      input_action_interface (handle, composer)
    { }
    
    virtual ~typed_input_action_interface () { }
    
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
    public executable_action_interface
  {
  public:
    output_action_interface (const generic_automaton_handle& handle) :
      executable_action_interface (handle)
    { }

    virtual bool operator== (const output_action_interface& oa) const = 0;
    
    virtual const void* get_member_ptr () const = 0;

    virtual bool involves_parameter (const void*) const = 0;
  };

  class untyped_output_action_interface :
    public output_action_interface
  {
  public:
    untyped_output_action_interface (const generic_automaton_handle& handle) :
      output_action_interface (handle)
    { }

    virtual ~untyped_output_action_interface () { }

    virtual bool operator() () = 0;
  };

  template <class T>
  class typed_output_action_interface :
    public output_action_interface
  {
  public:
    typed_output_action_interface (const generic_automaton_handle& handle) :
      output_action_interface (handle)
    { }

    virtual ~typed_output_action_interface () { }

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
    public untyped_input_action_interface,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
		 M& m,
  		 const generic_automaton_handle& composer) :
      untyped_input_action_interface (handle, composer),
      member_ref<M> (m)
    { }

    bool involves_parameter (const void* parameter) const {
      return false;
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
    public untyped_input_action_interface,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
		 const parameter_handle<PT>& parameter,
  		 const generic_automaton_handle& composer) :
      untyped_input_action_interface (handle, composer),
      member_ref<M> (m),
      param<PT> (parameter)
    { }

    bool involves_parameter (const void* parameter) const {
      return this->m_parameter.second == parameter;
    }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter.second);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void operator() () {
      this->m_member (this->m_parameter.second);
    }

  };

  template <class M, class VT>
  class action_impl<input_category, M, valued, VT, unparameterized, null_type> : 
    public typed_input_action_interface<VT>,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
  		 const generic_automaton_handle& composer) :
      typed_input_action_interface<VT> (handle, composer),
      member_ref<M> (m)
    { }

    bool involves_parameter (const void* parameter) const {
      return false;
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
    public typed_input_action_interface<VT>,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
		 const parameter_handle<PT>& parameter,
  		 const generic_automaton_handle& composer) :
      typed_input_action_interface<VT> (handle, composer),
      member_ref<M> (m),
      param<PT> (parameter)
    { }

    bool involves_parameter (const void* parameter) const {
      return this->m_parameter.second == parameter;
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool operator== (const input_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter.second);
    }

    void operator() (const VT t) {
      this->m_member (t, this->m_parameter.second);
    }

  };

  template <class M>
  class action_impl<output_category, M, unvalued, null_type, unparameterized, null_type> :
    public untyped_output_action_interface,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m) :
      untyped_output_action_interface (handle),
      member_ref<M> (m)
    { }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool involves_parameter (const void* parameter) const {
      return false;
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
    public untyped_output_action_interface,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
		 const parameter_handle<PT>& parameter) :
      untyped_output_action_interface (handle),
      member_ref<M> (m),
      param<PT> (parameter)
    { }

    bool involves_parameter (const void* parameter) const {
      return this->m_parameter.second == parameter;
    }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter.second);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool operator() () {
      return this->m_member (this->m_parameter.second);
    }

    void execute () {
      (*this) ();
    }
    
  };

  template <class M, class VT>
  class action_impl<output_category, M, valued, VT, unparameterized, null_type> :
    public typed_output_action_interface<VT>,
    private member_ref<M>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m) :
      typed_output_action_interface<VT> (handle),
      member_ref<M> (m)
    { }

    bool involves_parameter (const void* parameter) const {
      return false;
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
    public typed_output_action_interface<VT>,
    private member_ref<M>,
    private param<PT>
  {
  public:
    action_impl (const generic_automaton_handle& handle,
  		 M& m,
		 const parameter_handle<PT>& parameter) :
      typed_output_action_interface<VT> (handle),
      member_ref<M> (m),
      param<PT> (parameter)
    { }

    bool involves_parameter (const void* parameter) const {
      return this->m_parameter.second == parameter;
    }

    bool operator== (const output_action_interface& oa) const {
      return &this->m_member == oa.get_member_ptr () &&
  	oa.involves_parameter (this->m_parameter.second);
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    std::pair<bool, VT> operator() () {
      return this->m_member (this->m_parameter.second); 
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
      this->m_member (this->m_parameter.second);
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
  	    const generic_automaton_handle& composer) :
      action_impl<action_category,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member, composer)
    { }

    action (const generic_automaton_handle& handle,
  	    Member& member,
	    const parameter_handle<parameter_type>& parameter,
  	    const generic_automaton_handle& composer) :
      action_impl<action_category,
		  Member,
  		  value_status,
  		  value_type,
  		  parameter_status,
  		  parameter_type> (handle, member, parameter, composer)
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
