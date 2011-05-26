#ifndef __action_hpp__
#define __action_hpp__

#include <set>
#include "automaton_handle.hpp"

namespace ioa {

  /* Represents the absence of a type. */
  class null_type { };

  /*
    Action categories.

    Input, outputs, and internal actions come directly from the I/O automata formalism.
    Events are asynchronous messages from other automata.
    Events allow arbitrary automata to coordinate.
   */
  struct input_category { };
  struct output_category { };
  struct internal_category { };
  struct event_category { };

  /* Indicates if an input, outputs, or events has an associated value. */
  struct unvalued { };
  struct valued { };

  /* Indicates if an input, output, or internal action has an associated parameter.
     Parameters are designed to solve the problem of fan-in as an input can only be composed with one output.
     We can compose an input with multiple outputs by declaring a parameter for each output.
     This idea can be extended to outputs and internal actions to capture the notion of a session where the parameter indicates that all interactions with another automaton are identified using a parameter.
  */
  struct unparameterized { };
  struct parameterized { };

  /* These are helpers that define action traits. */

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
    Actions (or Action Descriptors)

    An action descriptor contains the ID of the associated automaton, a pointer to a member, a parameter ID (optional), and an event value (optional).
    Actions are flyweights that allow us to talk about actions that need to be scheduled or bound without having a pointer or reference to the actual automaton.
    To do something with an action, e.g., execute it, a reference to a member is required.

    A useful property exhibited in many systems that allow dynamic configuration is sensing the status of an input or output.
    For example, a monitor might complain that it is not plugged into a video source.
    Useful behavior can be associated with events that indicate a change in status.
    In our system, we will deliver messages to input and output actions when they are bound and unbound.
  */


  class action_interface
  {
  public:
    virtual ~action_interface () { }
    virtual const aid_t get_aid () const {
      return -1;
    }
    virtual const void* get_member_ptr () const {
      return 0;
    }
    virtual const size_t get_pid () const {
      return 0;
    }
    
    virtual bool operator== (const action_interface& x) const {
      return
	get_aid () == x.get_aid () &&
	get_member_ptr () == x.get_member_ptr () &&
	get_pid () == x.get_pid ();
    }

    bool operator!= (const action_interface& x) const {
      return !(*this == x);
    }
	
  };

  template <class I, class M>
  struct action_core :
    public action_interface
  {
    const automaton_handle<I> automaton;
    M I::*member_ptr;

    action_core (const automaton_handle<I>& a,
		 M I::*ptr) :
      automaton (a),
      member_ptr (ptr)
    { }

    const aid_t get_aid () const {
      return automaton.aid ();
    }

    const void* get_member_ptr () const {
      I* i = 0;
      return &((*i).*(member_ptr));
    }
  };

  template <class I, class M, class PT>
  struct parameter_core :
    public action_core<I, M>
  {
    PT parameter;

    parameter_core (const automaton_handle<I>& a,
		    M I::*ptr,
		    PT param) :
      action_core<I, M> (a, ptr),
      parameter (param)
    { }

    const size_t get_pid () const {
      return size_t (parameter);
    }
  };


  template <class I, class M>
  struct unparameterized_bound :
    public action_core<I, M>
  {
    unparameterized_bound (const automaton_handle<I>& a,
			   M I::*ptr) :
      action_core<I, M> (a, ptr)
    { }

    void bound (I& i) const {
      (i.*this->member_ptr).bound ();
    }

    void unbound (I& i) const {
      (i.*this->member_ptr).unbound ();
    }
  };

  template <class I, class M, class PT>
  struct parameterized_bound :
    public parameter_core<I, M, PT>
  {
    parameterized_bound (const automaton_handle<I>& a,
			 M I::*ptr,
			 PT param) :
      parameter_core<I, M, PT> (a, ptr, param)
    { }

    void bound (I& i) const {
      (i.*this->member_ptr).bound (this->parameter);
    }

    void unbound (I& i) const {
      (i.*this->member_ptr).unbound (this->parameter);
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
  template <class K, class I, class M, class VS, class VT, class PS, class PT> class action_impl;

  template <class I, class M>
  class action_impl<input_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public unparameterized_bound<I, M>
  {
  public:

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr) :
      unparameterized_bound<I, M> (a, ptr)
    { }

    void operator() (I& i) const {
      (i.*this->member_ptr) ();
    }

  };

  template <class I, class M, class PT>
  class action_impl<input_category, I, M, unvalued, null_type, parameterized, PT> :
    public parameterized_bound<I, M, PT>
  {
  public:

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr,
		 PT parameter) :
      parameterized_bound<I, M, PT> (a, ptr, parameter)
    { }

    void operator() (I& i) const {
      (i.*this->member_ptr) (this->parameter);
    }
  };

  template <class I, class M, class VT>
  class action_impl<input_category, I, M, valued, VT, unparameterized, null_type> :
    public unparameterized_bound<I, M>
  {
  public:

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr) :
      unparameterized_bound<I, M> (a, ptr)
    { }

    void operator() (I& i, const VT& v) const {
      (i.*this->member_ptr) (v);
    }
  };

  template <class I, class M, class VT, class PT>
  class action_impl<input_category, I, M, valued, VT, parameterized, PT> :
    public parameterized_bound<I, M, PT>
  {
  public:

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr,
		 PT param) :
      parameterized_bound<I, M, PT> (a, ptr, param)
    { }

    void operator() (I& i, const VT& v) const {
      (i.*this->member_ptr) (v, this->parameter);
    }
  };

  template <class I, class M>
  class action_impl<output_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public unparameterized_bound<I, M>
  {
  public:

    action_impl (const automaton_handle<I>& a,
  		 M I::*ptr) :
      unparameterized_bound<I, M> (a, ptr)
    { }

    bool operator() (I& i) const {
      return (i.*this->member_ptr) ();
    }

    void execute (I& i) const {
      (*this) (i);
    }
  };

  template <class I, class M, class PT>
  class action_impl<output_category, I, M, unvalued, null_type, parameterized, PT> :
    public parameterized_bound<I, M, PT>
  {
  public:

    action_impl (const automaton_handle<I>& a,
  		 M I::*ptr,
  		 PT parameter) :
      parameterized_bound<I, M, PT> (a, ptr, parameter)
    { }

    bool operator() (I& i) const {
      return (i.*this->member_ptr) (this->parameter);
    }

    void execute (I& i) const {
      (*this) (i);
    }
  };

  template <class I, class M, class VT>
  class action_impl<output_category, I, M, valued, VT, unparameterized, null_type> :
    public unparameterized_bound<I, M>
  {
  public:

    action_impl (const automaton_handle<I>& a,
  		 M I::*ptr) :
      unparameterized_bound<I, M> (a, ptr)
    { }

    std::pair<bool, VT> operator() (I& i) const {
      return (i.*this->member_ptr) ();
    }

    void execute (I& i) const {
      (*this) (i);
    }
  };

  template <class I, class M, class VT, class PT>
  class action_impl<output_category, I, M, valued, VT, parameterized, PT> :
    public parameterized_bound<I, M, PT>
  {
  public:

    action_impl (const automaton_handle<I>& a,
  		 M I::*ptr,
  		 PT param) :
      parameterized_bound<I, M, PT> (a, ptr, param)
    { }

    std::pair<bool, VT> operator() (I& i) const {
      return (i.*this->member_ptr) (this->parameter);
    }

    void execute (I& i) const {
      (*this) (i);
    }
  };

  template <class I, class M>
  class action_impl<internal_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public action_core<I, M>
  {
  public:

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr) :
      action_core<I, M> (a, ptr)
    { }

    void execute (I& i) const {
      (i.*this->member_ptr) ();
    }

  };

  template <class I, class M, class PT>
  class action_impl<internal_category, I, M, unvalued, null_type, parameterized, PT> :
    public parameter_core<I, M, PT>
  {
  public:

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr,
		 PT param) :
      parameter_core<I, M, PT> (a, ptr, param)
    { }

    void execute (I& i) const {
      (i.*this->member_ptr) (this->parameter);
    }

  };

  template <class I, class M>
  class action_impl<event_category, I, M, unvalued, null_type, unparameterized, null_type> :
    public action_core<I, M>
  {
  public:
    
    action_impl (const automaton_handle<I>& a,
		 M I::*ptr) :
      action_core<I, M> (a, ptr)
    { }
    
    void execute (I& i) const {
      (i.*this->member_ptr) ();
    }

    virtual bool operator== (const action_interface& x) const {
      return false;
    }

  };

  template <class I, class M, class VT>
  class action_impl<event_category, I, M, valued, VT, unparameterized, null_type> :
    public action_core<I, M>
  {
  private:
    const VT m_value;

  public:
    
    action_impl (const automaton_handle<I>& a,
		 M I::*ptr,
		 const VT& value) :
      action_core<I, M> (a, ptr),
      m_value (value)
    { }
    
    void execute (I& i) const {
      (i.*this->member_ptr) (m_value);
    }

    virtual bool operator== (const action_interface& x) const {
      return false;
    }

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
    	    const parameter_type& p,
	    parameterized /* */) :
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
    	    const value_type& v,
	    unparameterized /* */) :
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

  template <class I, class M, class A>
  action<I, M>
  make_action (const automaton_handle<I>& handle,
	       M I::*member_ptr,
	       const A& a) {
    return action<I, M> (handle, member_ptr, a, typename M::parameter_status ());
  }

}

#endif
