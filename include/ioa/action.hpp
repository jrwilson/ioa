#ifndef __action_hpp__
#define __action_hpp__

#include <cstddef>
#include <utility>
#include <ioa/automaton_handle.hpp>

namespace ioa {

  /* Represents the absence of a type. */
  class null_type { };

  /*
    Action categories.

    Input, outputs, and internal actions come directly from the I/O automata formalism.
    Functions are actions designed to facilitate the request-response semantics inherent in systems with dynamic resource allocation.
    To illustrate, consider a VFS automaton that creates File automata for manipulation by other automata.
    The problem is how to allow an arbitrary automaton to request a new File automaton from the VFS automaton similar to executing the open function in traditional UNIX.
      - Poll-binding - The automaton could try to bind to the required actions of the VFS automaton until it succeeds.  Once bound, the two automata execute a protocol to create a new File automataon.  This approach has a number of problems including binding order, releasing the VFS through unbinding for other automata, and efficiency.
      - Bind with external parameter - The automaton could specify a parameter (such as its aid) when binding to the VFS.  This approach is also not safe because it relies on convention.
      - Synchronous call - The automaton executed a synchronous method on the VFS to open the file.  The method returns the handle to the new automaton.  The synchronous call 1) breaks the locking scheme as the automata involved in an action must be know a priori and 2) fails because the creation of new automata is asynchronous.
      - Scheduled synchronous call - Solves the first issue with the synchronous call approach but still fails since the creation of new automata is asynchronous.
      - Events - The VFS automaton receives an open event.  After processing, the VFS sends an event containing the aid of the new File automaton.  This approach is attractive in its generality and ease of implementation.  However it is complicated for the user.
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

  template <typename T>
  struct value {
    typedef valued value_status;
    typedef T value_type;
  };

  struct no_parameter {
    typedef unparameterized parameter_status;
    typedef null_type parameter_type;
  };

  template <typename T>
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

    An action descriptor contains the ID of the associated automaton, a pointer to a member, and a parameter ID (optional).
    Actions are flyweights that allow us to talk about actions that need to be scheduled or bound without having a pointer or reference to the actual automaton.
    To do something with an action, e.g., execute it, a reference to a member is required.

    A useful property exhibited in many systems that allow dynamic configuration is sensing the status of an input or output.
    For example, a monitor might complain that it is not plugged into a video source.
    Useful behavior can be associated with event indicating a change in status.
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
      return automaton;
    }

    const void* get_member_ptr () const {
      I* i = 0;
      return &((*i).*(member_ptr));
    }
  };

  template <class I, class M, typename PT>
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

  template <class I, class M, typename PT>
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
  template <class K, class I, class M, class VS, typename VT, class PS, typename PT> class action_impl;

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

  template <class I, class M, typename PT>
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

  template <class I, class M, typename VT>
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

  template <class I, class M, typename VT, typename PT>
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
  };

  template <class I, class M, typename PT>
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
  };

  template <class I, class M, typename VT>
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
  };

  template <class I, class M, typename VT, typename PT>
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

    void operator() (I& i) const {
      (i.*this->member_ptr) ();
    }

  };

  template <class I, class M, typename PT>
  class action_impl<internal_category, I, M, unvalued, null_type, parameterized, PT> :
    public parameter_core<I, M, PT>
  {
  public:

    action_impl (const automaton_handle<I>& a,
		 M I::*ptr,
		 PT param) :
      parameter_core<I, M, PT> (a, ptr, param)
    { }

    void operator() (I& i) const {
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

    void operator() (I& i) const {
      (i.*this->member_ptr) ();
    }

    // Events are unique.
    virtual bool operator== (const action_interface& x) const {
      return false;
    }

  };

  template <class I, class M, typename VT>
  class action_impl<event_category, I, M, valued, VT, unparameterized, null_type> :
    public action_core<I, M>
  {
  private:
    VT m_value;

  public:

    action_impl (const automaton_handle<I>& a,
  		 M I::*ptr,
  		 const VT& value) :
      action_core<I, M> (a, ptr),
      m_value (value)
    { }

    void operator() (I& i) const {
      (i.*this->member_ptr) (m_value);
    }

    // Events are unique.
    virtual bool operator== (const action_interface& x) const {
      return false;
    }

  };

  template <class I, class M>
  class action :
    public action_impl<typename M::action_category,
		       I,
		       M,
		       typename M::value_status,
		       typename M::value_type,
		       typename M::parameter_status,
		       typename M::parameter_type>
  {
  public:
    typedef typename M::action_category action_category;
    typedef typename M::value_status value_status;
    typedef typename M::value_type value_type;
    typedef typename M::parameter_status parameter_status;
    typedef typename M::parameter_type parameter_type;

    action (const automaton_handle<I>& a,
  	    M I::*ptr) :
      action_impl <action_category,
		   I,
		   M,
		   value_status,
		   value_type,
		   parameter_status,
		   parameter_type> (a, ptr)
    { }

    action (const automaton_handle<I>& a,
    	    M I::*ptr,
    	    const parameter_type& p,
	    parameterized /* */) :
      action_impl <action_category,
		   I,
		   M,
		   value_status,
		   value_type,
		   parameter_status,
		   parameter_type> (a, ptr, p)
    { }
    
    action (const automaton_handle<I>& a,
    	    M I::*ptr,
    	    const value_type& v,
	    unparameterized /* */) :
      action_impl <action_category,
		   I,
		   M,
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

  template <class I, class M, typename A>
  action<I, M>
  make_action (const automaton_handle<I>& handle,
	       M I::*member_ptr,
	       const A& a) {
    return action<I, M> (handle, member_ptr, a, typename M::parameter_status ());
  }

}

#endif
