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
  private:
    const aid_t m_aid;
    const void* const m_instance;
    const void* const m_member_ptr;
    const pid_t m_pid;

  public:

    action_interface (const aid_t aid,
		      const void* instance,
		      const void* member_ptr,
		      const pid_t pid) :
      m_aid (aid),
      m_instance (instance),
      m_member_ptr (member_ptr),
      m_pid (pid)
    { }
    
    action_interface (const aid_t aid,
		      const void* instance,
		      const void* member_ptr) :
      m_aid (aid),
      m_instance (instance),
      m_member_ptr (member_ptr),
      m_pid (-1)
    { }
    
    virtual ~action_interface() { }

    const aid_t get_aid () const {
      return m_aid;
    }

    const void* get_instance () const {
      return m_instance;
    }

    const pid_t get_pid () const {
      return m_pid;
    }

    virtual bool operator== (const action_interface& x) const {
      return m_member_ptr == x.m_member_ptr &&
	m_pid == x.m_pid;
    }
    
    bool operator!= (const action_interface& x) const {
      return !(*this == x);
    }
  };

  class input_action_interface :
    public action_interface
  {
  public:
    input_action_interface (const aid_t aid,
			    const void* instance,
			    const void* member_ptr,
			    const pid_t pid) :
      action_interface (aid, instance, member_ptr, pid)
    { }

    input_action_interface (const aid_t aid,
			    const void* instance,
			    const void* member_ptr) :
      action_interface (aid, instance, member_ptr)
    { }

    virtual ~input_action_interface () { }
  };

  class unvalued_input_action_interface :
    public input_action_interface
  {
  public:
    unvalued_input_action_interface (const aid_t aid,
				     const void* instance,
				     const void* member_ptr,
				     const pid_t pid) :
      input_action_interface (aid, instance, member_ptr, pid)
    { }
    
    unvalued_input_action_interface (const aid_t aid,
				     const void* instance,
				     const void* member_ptr) :
      input_action_interface (aid, instance, member_ptr)
    { }
    
    virtual ~unvalued_input_action_interface () { }
    
    virtual void operator() () const = 0;
  };

  template <class T>
  class valued_input_action_interface :
    public input_action_interface
  {
  public:
    valued_input_action_interface (const aid_t aid,
				   const void* instance,
				   const void* member_ptr,
				   const pid_t pid) :
      input_action_interface (aid, instance, member_ptr, pid)
    { }

    valued_input_action_interface (const aid_t aid,
				   const void* instance,
				   const void* member_ptr) :
      input_action_interface (aid, instance, member_ptr)
    { }

    virtual ~valued_input_action_interface () { }
    
    virtual void operator() (const T t) const = 0;
  };

  class output_action_interface :
    public action_interface
  {
  public:
    output_action_interface (const aid_t aid,
			     const void* instance,
			     const void* member_ptr,
			     const pid_t pid) :
      action_interface (aid, instance, member_ptr, pid)
    { }
    
    output_action_interface (const aid_t aid,
			     const void* instance,
			     const void* member_ptr) :
      action_interface (aid, instance, member_ptr)
    { }
    
    virtual ~output_action_interface () { }
  };

  class unvalued_output_action_interface :
    public output_action_interface
  {
  public:
    unvalued_output_action_interface (const aid_t aid,
				      const void* instance,
				      const void* member_ptr,
				      const pid_t pid) :
      output_action_interface (aid, instance, member_ptr, pid)
    { }
    
    unvalued_output_action_interface (const aid_t aid,
				      const void* instance,
				      const void* member_ptr) :
      output_action_interface (aid, instance, member_ptr)
    { }

    virtual ~unvalued_output_action_interface () { }

    virtual bool operator() () const = 0;
  };

  template <class T>
  class valued_output_action_interface :
    public output_action_interface
  {
  public:
    valued_output_action_interface (const aid_t aid,
				    const void* instance,
				    const void* member_ptr,
				    const pid_t pid) :
      output_action_interface (aid, instance, member_ptr, pid)
    { }
    
    valued_output_action_interface (const aid_t aid,
				    const void* instance,
				    const void* member_ptr) :
      output_action_interface (aid, instance, member_ptr)
    { }
    
    virtual ~valued_output_action_interface () { }
    
    virtual std::pair<bool, T> operator() () const = 0;
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

  template <class I, class M>
  struct action_core
  {
    const automaton_handle<I> automaton;
    M I::*member_ptr;

    action_core (const automaton_handle<I>& a,
		 M I::*ptr) :
      automaton (a),
      member_ptr (ptr)
    { }

  };

  /*
    K - category
    M - member
    VS - value status
    VT - value type
    PS - parameter status
    PT - parameter type
  */
  template <class K, class I, class M, class VS, class VT, class PS, class PT> class action_impl;

  template <class K, class I, class M, class VS, class VT>
  class action_impl<K, I, M, VS, VT, unparameterized, null_type> :
    public action_core<I, M>
  {
  public:

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr) :
      action_core<I, M> (a, ptr)
    { }
    
  };

  template <class K, class I, class M, class VS, class VT, class PT>
  class action_impl<K, I, M, VS, VT, parameterized, PT> :
    public action_core<I, M>
  {
  public:
    const parameter_handle<PT> parameter;
    
    action_impl (const automaton_handle<I>& a,
		 M I::*ptr,
		 const parameter_handle<PT>& p) :
      action_core<I, M> (a, ptr),
      parameter (p)
    { }
    
  };

  template <class I, class M, class VT>
  class action_impl<event_category, I, M, valued, VT, unparameterized, null_type> :
    public action_core<I, M>
  {
  public:
    const VT value;

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr,
		 const VT& v) :
      action_core<I, M> (a, ptr),
      value (v)
    { }
    
  };

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

    action (const automaton_handle<I>& a,
  	    Member I::*ptr) :
      action_impl <action_category,
		   I,
		   Member,
		   value_status,
		   value_type,
		   parameter_status,
		   parameter_type> (a, ptr)
    { }

    action (const automaton_handle<I>& a,
    	    Member I::*ptr,
    	    const parameter_handle<parameter_type>& p) :
      action_impl <action_category,
		   I,
		   Member,
		   value_status,
		   value_type,
		   parameter_status,
		   parameter_type> (a, ptr, p)
    { }
    
    action (const automaton_handle<I>& a,
    	    Member I::*ptr,
    	    const value_type& v) :
      action_impl <action_category,
		   I,
		   Member,
		   value_status,
		   value_type,
		   parameter_status,
		   parameter_type> (a, ptr, v)
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

  template <class M>
  class unparameterized_bound
  {
  private:
    M& m_member;

  public:
    unparameterized_bound (M& member) :
      m_member (member)
    { }

    void bound () const {
      m_member.bound ();
    }

    void unbound () const {
      m_member.unbound ();
    }
  };

  template <class M, class P>
  class parameterized_bound
  {
  private:
    M& m_member;
    P* m_parameter;

  public:
    parameterized_bound (M& member,
			 P* parameter) :
      m_member (member),
      m_parameter (parameter)
    { }
    
    void bound () const {
      m_member.bound (m_parameter);
    }

    void unbound () const {
      m_member.unbound (m_parameter);
    }
  };

  /*
    K - category
    M - member
    VS - value status
    VT - value type
    PS - parameter status
    PT - parameter type
  */
  template <class K, class I, class M, class VS, class VT, class PS, class PT> class concrete_action_impl;

  template <class I, class M>
  class concrete_action_impl<input_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public action<I, M>,
    public unvalued_input_action_interface,
    public unparameterized_bound<M>
  {
  private:
    M& m_member;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance) :
      action<I, M> (ac),
      unvalued_input_action_interface (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr))),
      unparameterized_bound<M> ((*instance).*(ac.member_ptr)),
      m_member ((*instance).*(ac.member_ptr))
    { }

    void operator() () const {
      m_member ();
    }
  };

  template <class I, class M, class PT>
  class concrete_action_impl<input_category, I, M, unvalued, null_type, parameterized, PT> : 
    public action<I, M>,
    public unvalued_input_action_interface,
    public parameterized_bound<M, PT>
  {
  private:
    M& m_member;
    PT* m_parameter;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance,
			  PT* parameter) :
      action<I, M> (ac),
      unvalued_input_action_interface (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr)), ac.parameter.pid),
      parameterized_bound<M, PT> ((*instance).*(ac.member_ptr), parameter),
      m_member ((*instance).*(ac.member_ptr)),
      m_parameter (parameter)
    { }

    void operator() () const {
      m_member (m_parameter);
    }

  };

  template <class I, class M, class VT>
  class concrete_action_impl<input_category, I, M, valued, VT, unparameterized, null_type> : 
    public action<I, M>,
    public valued_input_action_interface<VT>,
    public unparameterized_bound<M>
  {
  private:
    M& m_member;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance) :
      action<I, M> (ac),
      valued_input_action_interface<VT> (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr))),
      unparameterized_bound<M> ((*instance).*(ac.member_ptr)),
      m_member ((*instance).*(ac.member_ptr))
    { }

    void operator() (const VT t) const {
      m_member (t);
    }

  };

  template <class I, class M, class VT, class PT>
  class concrete_action_impl<input_category, I, M, valued, VT, parameterized, PT> :
    public action<I, M>,
    public valued_input_action_interface<VT>,
    public parameterized_bound<M, PT>
  {
  private:
    M& m_member;
    PT* m_parameter;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance,
			  PT* parameter) :
      action<I, M> (ac),
      valued_input_action_interface<VT> (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr)), ac.parameter.pid),
      parameterized_bound<M, PT> ((*instance).*(ac.member_ptr), parameter),
      m_member ((*instance).*(ac.member_ptr)),
      m_parameter (parameter)
    { }

    void operator() (const VT t) const {
      m_member (t, m_parameter);
    }

  };

  template <class I, class M>
  class concrete_action_impl<output_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public action<I, M>,
    public unvalued_output_action_interface,
    public unparameterized_bound<M>
  {
  private:
    M& m_member;
    
  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance) :
      action<I, M> (ac),
      unvalued_output_action_interface (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr))),
      unparameterized_bound<M> ((*instance).*(ac.member_ptr)),
      m_member ((*instance).*(ac.member_ptr))
    { }

    bool operator() () const {
      return m_member ();
    }

    void execute () const {
      m_member ();
    }

  };

  template <class I, class M, class PT>
  class concrete_action_impl<output_category, I, M, unvalued, null_type, parameterized, PT> :
    public action<I, M>,
    public unvalued_output_action_interface,
    public parameterized_bound<M, PT>
  {
  private:
    M& m_member;
    PT* m_parameter;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance,
			  PT* parameter) :
      action<I, M> (ac),
      unvalued_output_action_interface (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr)), ac.parameter.pid),
      parameterized_bound<M, PT> ((*instance).*(ac.member_ptr), parameter),
      m_member ((*instance).*(ac.member_ptr)),
      m_parameter (parameter)
    { }

    bool operator() () const {
      return m_member (m_parameter);
    }

    void execute () const {
      m_member (m_parameter);
    }

  };

  template <class I, class M, class VT>
  class concrete_action_impl<output_category, I, M, valued, VT, unparameterized, null_type> :
    public action<I, M>,
    public valued_output_action_interface<VT>,
    public unparameterized_bound<M>
  {
  private:
    M& m_member;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance) :
      action<I, M> (ac),
      valued_output_action_interface<VT> (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr))),
      unparameterized_bound<M> ((*instance).*(ac.member_ptr)),
      m_member ((*instance).*(ac.member_ptr))
    { }

    std::pair<bool, VT> operator() () const {
      return m_member ();
    }

    void execute () const {
      m_member ();
    }

  };

  template <class I, class M, class VT, class PT>
  class concrete_action_impl<output_category, I, M, valued, VT, parameterized, PT> :
    public action<I, M>,
    public valued_output_action_interface<VT>,
    public parameterized_bound<M, PT>
  {
  private:
    M& m_member;
    PT* m_parameter;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance,
			  PT* parameter) :
      action<I, M> (ac),
      valued_output_action_interface<VT> (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr)), ac.parameter.pid),
      parameterized_bound<M, PT> ((*instance).*(ac.member_ptr), parameter),
      m_member ((*instance).*(ac.member_ptr)),
      m_parameter (parameter)
    { }

    std::pair<bool, VT> operator() () const {
      return m_member (m_parameter); 
    }

    void execute () const {
      m_member (m_parameter);
    }

  };

  template <class I, class M>
  class concrete_action_impl<internal_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public action<I, M>,
    public action_interface
  {
  private:
    M& m_member;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance) :
      action<I, M> (ac),
      action_interface (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr))),
      m_member ((*instance).*(ac.member_ptr))
    { }

    void execute () const {
      m_member ();
    }
  };

  template <class I, class M, class PT>
  class concrete_action_impl<internal_category, I, M, unvalued, null_type, parameterized, PT> :
    public action<I, M>,
    public action_interface
  {
  private:
    M& m_member;
    PT* m_parameter;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance,
			  PT* parameter) :
      action<I, M> (ac),
      action_interface (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr)), ac.parameter.pid),
      m_member ((*instance).*(ac.member_ptr)),
      m_parameter (parameter)
    { }

    void execute () const {
      m_member (m_parameter);
    }
    
  };

  template <class I, class M>
  class concrete_action_impl<event_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public action<I, M>,
    public action_interface
  {
  private:
    M& m_member;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance) :
      action<I, M> (ac),
      action_interface (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr))),
      m_member ((*instance).*(ac.member_ptr))
    { }

    void execute () const {
      m_member ();
    }    

    bool operator== (const action_interface&) const {
      return false;
    }

  };

  template <class I, class M, class VT>
  class concrete_action_impl<event_category, I, M, valued, VT, unparameterized, null_type> :
    public action<I, M>,
    public action_interface
  {
  private:
    M& m_member;

  public:
    concrete_action_impl (const action<I, M>& ac,
			  I* instance) :
      action<I, M> (ac),
      action_interface (ac.automaton.aid, instance, &((*instance).*(ac.member_ptr))),
      m_member ((*instance).*(ac.member_ptr))
    { }

    void execute () const {
      this->m_member (this->value);
    }    

    bool operator== (const action_interface&) const {
      return false;
    }

  };
  
  template <class I, class Member>
  class concrete_action :
    public concrete_action_impl<typename Member::action_category,
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

    concrete_action (const action<I, Member>& ac,
		     I* instance) :
      concrete_action_impl<action_category,
			   I,
			   Member,
			   value_status,
			   value_type,
			   parameter_status,
			   parameter_type> (ac, instance)
    { }

    concrete_action (const action<I, Member>& ac,
		     I* instance,
		     parameter_type* parameter) :
      concrete_action_impl<action_category,
			   I,
			   Member,
			   value_status,
			   value_type,
			   parameter_status,
			   parameter_type> (ac, instance, parameter)
    { }
    
  };

}

#endif
