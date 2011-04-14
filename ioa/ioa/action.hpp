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
    See macro_action.hpp to see how this is used.

    The executable interface contains output and independent actions.
    This split is necessary because outputs require special processing when they are executed, i.e., they can be composed and involve more than one automaton.
    Independent actions only involve one automaton and include internal actions, events, and anonymous internal actions.
    The intended usage of independent actions is that they will be executed after the automaton is locked.

    Events are asynchronous messages from other automata.
    Conceptually, events act like a permanent inbox and are a cross between an internal action and an input action.
    Events allow arbitrary automata to coordinate are are necessary for dynamic interfaces.

    Anonymous internal actions are used to allow the system to deliver the result of a system call to an automaton.
    Anonymous internal actions consist of a nullary functor.

  */

  class action_interface
  {
  private:
    automaton* m_automaton;
    
  public:
    action_interface (automaton* automaton) :
      m_automaton (automaton)
    { }

    automaton* get_automaton () const
    {
      return m_automaton;
    }
  };

  std::ostream& operator<<(std::ostream& output, const action_interface&);

  class input_action_interface :
    public action_interface
  {
  private:
    const automaton* m_owner;
    
  public:
    input_action_interface (automaton* automaton,
			    const automaton* owner) :
      action_interface (automaton),
      m_owner (owner)
    { }
    
    virtual ~input_action_interface () { }

    virtual void decompose () = 0;

    const automaton* get_owner () const
    {
      return m_owner;
    }

    bool operator== (const input_action_interface& ia) const
    {
      return get_member_ptr () == ia.get_member_ptr ();
    }
    
    virtual const void* get_member_ptr () const = 0;
    
    virtual bool involves_parameter (const void*) const = 0;
  };

  class untyped_input_action_interface :
    public input_action_interface
  {
  public:
    untyped_input_action_interface (automaton* automaton,
				    const automaton* owner) :
      input_action_interface (automaton, owner)
    { }
    
    virtual ~untyped_input_action_interface () { }
    
    virtual void operator() () = 0;
  };

  template <class T>
  class typed_input_action_interface :
    public input_action_interface
  {
  public:
    typed_input_action_interface (automaton* automaton,
				  const automaton* owner) :
      input_action_interface (automaton, owner)
    { }
    
    virtual ~typed_input_action_interface () { }
    
    virtual void operator() (const T t) = 0;
  };

  class executable_action_interface :
    public action_interface
  {
  public:
    executable_action_interface (automaton* automaton) :
      action_interface (automaton)
    { }

    virtual ~executable_action_interface () { }

    virtual void execute () = 0;
  };

  class output_action_interface :
    public executable_action_interface
  {
  public:
    output_action_interface (automaton* automaton) :
      executable_action_interface (automaton)
    { }
    
    bool operator== (const output_action_interface& oa) const
    {
      return get_member_ptr () == oa.get_member_ptr ();
    }
    
    virtual const void* get_member_ptr () const = 0;
    
    virtual bool involves_parameter (const void*) const = 0;
  };

  class untyped_output_action_interface :
    public output_action_interface
  {
  public:
    untyped_output_action_interface (automaton* automaton) :
      output_action_interface (automaton)
    { }

    virtual ~untyped_output_action_interface () { }

    virtual bool operator() () = 0;
  };

  template <class T>
  class typed_output_action_interface :
    public output_action_interface
  {
  public:
    typed_output_action_interface (automaton* automaton) :
      output_action_interface (automaton)
    { }

    virtual ~typed_output_action_interface () { }

    virtual std::pair<bool, T> operator() () = 0;
  };

  class independent_action_interface :
    public executable_action_interface
  {
  public:
    independent_action_interface (automaton* automaton) :
      executable_action_interface (automaton)
    { }
  };

  // Member.
  template <class Member>
  class ref_member
  {
  protected:
    Member& m_member;

  public:
    ref_member (Member& m) :
      m_member (m)
    { }
  };

  template <class Member>
  class copy_member
  {
  protected:
    Member m_member;
    
  public:
    copy_member (const Member& m) :
      m_member (m)
    { }
  };

  // Callback
  template <class Callback>
  class callback
  {
  protected:
    // TODO:  We get a copy of the callback.  Could we get by with a reference?
    Callback m_callback;

  public:
    callback (const Callback& callback) :
      m_callback (callback)
    { }
  };

  // Parameter.
  template <class T>
  class parameter_impl {
  protected:
    T* m_parameter;

  public:
    parameter_impl (T* parameter) :
      m_parameter (parameter)
    { }
  };

  class null_type { };

  struct input_category { };
  struct output_category { };
  struct internal_category { };
  struct event_category { };
  struct anonymous_internal_category { };

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

  struct anonymous_internal : public no_value, public no_parameter {
    typedef anonymous_internal_category action_category;
  };

  /*
    K - category
    VS - value status
    VT - value type
    PS - parameter status
    PT - parameter type
    M - member
    C - callback
  */
  template <class K, class VS, class VT, class PS, class PT, class M, class C> class action_impl;

  template <class M, class C>
  class action_impl<input_category, unvalued, null_type, unparameterized, null_type, M, C> :
    public untyped_input_action_interface,
    private ref_member<M>,
    private callback<C>
  {
  public:
    action_impl (automaton* automaton,
	    M& m,
	    const automaton* owner,
	    const C& c) :
      untyped_input_action_interface (automaton, owner),
      ref_member<M> (m),
      callback<C> (c)
    { }
    
    bool involves_parameter (const void* parameter) const {
      return false;
    }
    
    void decompose () {
      this->m_callback ();
    }
    
    const void* get_member_ptr () const {
      return &this->m_member;
    }
    
    void operator() () {
      this->m_member ();
    }
  };

  template <class PT, class M, class C>
  class action_impl<input_category, unvalued, null_type, parameterized, PT, M, C> : 
    public untyped_input_action_interface,
    private parameter_impl<PT>,
    private ref_member<M>,
    private callback<C> {

  public:
    action_impl (automaton* automaton,
	PT* p,
	M& m,
	const automaton* owner,
	const C& c) :
      untyped_input_action_interface (automaton, owner),
      parameter_impl<PT> (p),
      ref_member<M> (m),
      callback<C> (c)
    { }

    bool involves_parameter (const void* parameter) const {
      return this->m_parameter == parameter;
    }

    void decompose () {
      this->m_callback ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void operator() () {
      this->m_member (this->m_parameter);
    }

  };

  template <class VT, class M, class C>
  class action_impl<input_category, valued, VT, unparameterized, null_type, M, C> : 
    public typed_input_action_interface<VT>,
    private ref_member<M>,
    private callback<C>
  {
  public:
    action_impl (automaton* automaton,
	M& m,
	const automaton* owner,
	const C& c) :
      typed_input_action_interface<VT> (automaton, owner),
      ref_member<M> (m),
      callback<C> (c)
    { }

    bool involves_parameter (const void* parameter) const {
      return false;
    }

    void decompose () {
      this->m_callback ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void operator() (const VT t) {
      this->m_member (t);
    }

  };

  template <class VT, class PT, class M, class C>
  class action_impl<input_category, valued, VT, parameterized, PT, M, C> :
    public typed_input_action_interface<VT>,
    private parameter_impl<PT>,
    private ref_member<M>,
    private callback<C>
  {
  public:
    action_impl (automaton* automaton,
	PT* p,
	M& m,
	const automaton* owner,
	const C& c) :
      typed_input_action_interface<VT> (automaton, owner),
      parameter_impl<PT> (p),
      ref_member<M> (m),
      callback<C> (c)
    { }

    bool involves_parameter (const void* parameter) const {
      return this->m_parameter == parameter;
    }

    void decompose () {
      this->m_callback ();
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void operator() (const VT t) {
      this->m_member (t, this->m_parameter);
    }

  };

  template <class M>
  class action_impl<output_category, unvalued, null_type, unparameterized, null_type, M, null_type> :
    public untyped_output_action_interface,
    private ref_member<M>
  {
  public:
    action_impl (automaton* automaton,
	M& m) :
      untyped_output_action_interface (automaton),
      ref_member<M> (m)
    { }
    
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

  template <class PT, class M>
  class action_impl<output_category, unvalued, null_type, parameterized, PT, M, null_type> :
    public untyped_output_action_interface,
    private parameter_impl<PT>,
    private ref_member<M>
  {
  public:
    action_impl (automaton* automaton,
	PT* p,
	M& m) :
      untyped_output_action_interface (automaton),
      parameter_impl<PT> (p),
      ref_member<M> (m)
    { }
    
    bool involves_parameter (const void* parameter) const {
      return this->m_parameter == parameter;
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    bool operator() () {
      return this->m_member (this->m_parameter);
    }

    void execute () {
      (*this) ();
    }
    
  };

  template <class VT, class M>
  class action_impl<output_category, valued, VT, unparameterized, null_type, M, null_type> :
    public typed_output_action_interface<VT>,
    private ref_member<M>
  {
  public:
    action_impl (automaton* automaton,
	M& m) :
      typed_output_action_interface<VT> (automaton),
      ref_member<M> (m)
    { }
    
    bool involves_parameter (const void* parameter) const {
      return false;
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

  template <class VT, class PT, class M>
  class action_impl<output_category, valued, VT, parameterized, PT, M, null_type> :
    public typed_output_action_interface<VT>,
    private parameter_impl<PT>,
    private ref_member<M>
  {
  public:
    action_impl (automaton* automaton,
	PT* p,
	M& m) :
      typed_output_action_interface<VT> (automaton),
      parameter_impl<PT> (p),
      ref_member<M> (m)
    { }

    bool involves_parameter (const void* parameter) const {
      return this->m_parameter == parameter;
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    std::pair<bool, VT> operator() () {
      return this->m_member (this->m_parameter); 
    }

    void execute () {
      (*this) ();
    }

  };

  template <class M>
  class action_impl<internal_category, unvalued, null_type, unparameterized, null_type, M, null_type> :
    public independent_action_interface,
    private ref_member<M>
  {
  public:
    action_impl (automaton* automaton,
	M& m) :
      independent_action_interface (automaton),
      ref_member<M> (m)
    { }
    
    void execute () {
      this->m_member ();
    }
  };

  template <class PT, class M>
  class action_impl<internal_category, unvalued, null_type, parameterized, PT, M, null_type> :
    public independent_action_interface,
    private parameter_impl<PT>,
    private ref_member<M>
  {
  public:
    action_impl (automaton* automaton,
	PT* p,
	M& m) :
      independent_action_interface (automaton),
      parameter_impl<PT> (p),
      ref_member<M> (m)
    { }
    
    void execute () {
      this->m_member (this->m_parameter);
    }
    
  };

  template <class VT, class M>
  class action_impl<event_category, valued, VT, unparameterized, null_type, M, null_type> :
    public independent_action_interface,
    private ref_member<M>
  {
  private:
    VT m_t;

  public:
    action_impl (automaton* automaton,
	M& m,
	const VT& t) :
      independent_action_interface (automaton),
      ref_member<M> (m),
      m_t (t)
    { }

    void execute () {
      this->m_member (m_t);
    }    
  };

  template <class M>
  class action_impl<anonymous_internal_category, unvalued, null_type, unparameterized, null_type, M, null_type> :
    public independent_action_interface,
    private copy_member<M>
  {
    // TODO:  We are copying.  Should we take a reference?
  public:
    action_impl (automaton* automaton,
		 const M& m) :
      independent_action_interface (automaton),
      copy_member<M> (m)
    { }
    
    void execute () {
      this->m_member ();
    }
  };
  
  template <class Member, class Callback = null_type>
  class action :
    public action_impl<typename Member::action_category,
		       typename Member::value_status,
		       typename Member::value_type,
		       typename Member::parameter_status,
		       typename Member::parameter_type,
		       Member,
		       Callback>
  {
  public:
    typedef typename Member::action_category action_category;
    typedef typename Member::value_status value_status;
    typedef typename Member::value_type value_type;
    typedef typename Member::parameter_status parameter_status;
    typedef typename Member::parameter_type parameter_type;

    action (automaton* automaton,
	    Member& member) :
      action_impl<action_category,
		  value_status,
		  value_type,
		  parameter_status,
		  parameter_type,
		  Member,
		  Callback> (automaton, member)
    { }

    action (automaton* automaton,
	    parameter_type* parameter,
	    Member& member) :
      action_impl<action_category,
		  value_status,
		  value_type,
		  parameter_status,
		  parameter_type,
		  Member,
		  Callback> (automaton, parameter, member)
    { }

    action (automaton* automaton,
	    Member& member,
	    const automaton* owner,
	    const Callback& callback) :
      action_impl<action_category,
		  value_status,
		  value_type,
		  parameter_status,
		  parameter_type,
		  Member,
		  Callback> (automaton, member, owner, callback)
    { }      

    action (automaton* automaton,
	    parameter_type* parameter,
	    Member& member,
	    const automaton* owner,
	    const Callback& callback) :
      action_impl<action_category,
		  value_status,
		  value_type,
		  parameter_status,
		  parameter_type,
		  Member,
		  Callback> (automaton, parameter, member, owner, callback)
    { }

    action (automaton* automaton,
	    const Member& member) :
      action_impl<action_category,
		  value_status,
		  value_type,
		  parameter_status,
		  parameter_type,
		  Member,
		  Callback> (automaton, member)
    { }
  };

}

#endif
