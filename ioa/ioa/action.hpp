#ifndef __action_hpp__
#define __action_hpp__

#include <set>
#include "automaton.hpp"

namespace ioa {

  class scheduler_interface {
  public:
    virtual ~scheduler_interface () { }
    virtual void set_current_automaton (automaton*) = 0;
  };

  scheduler_interface& get_scheduler ();

  // I need something like "action traits."

  struct input { };
  struct output { };
  // struct internal { };
  // struct free_input { };
  // struct callback { };

  struct untyped { };
  struct typed { };

  struct unparameterized { };
  struct parameterized { };

  // template <class Direction, class ValueStatus, class ValueType, class ParameterStatus, class ParameterType>
  // struct action_trait {
  //   typedef Direction direction;
  //   typedef ValueStatus value_status;
  //   typedef ValueType value_type;
  //   typedef ParameterStatus parameter_status;
  //   typedef ParameterType parameter_type;
  // };

  // Action.
  class action {

  private:
    automaton* m_automaton;
    
  public:
    action (automaton* automaton) :
      m_automaton (automaton)
    { }
  
    virtual ~action () { }

    automaton* get_automaton () const {
      return m_automaton;
    }
        
  };

  std::ostream& operator<<(std::ostream& output, const action&);

  class local_action :
    public action {
    
  public:
    local_action (automaton* automaton) :
      action (automaton)
    { }

    virtual void execute () = 0;
    
  };

  class output_action :
    public local_action {

  public:
    output_action (automaton* automaton) :
      local_action (automaton)
    { }

    bool operator== (const output_action& oa) const {
      return get_member_ptr () == oa.get_member_ptr ();
    }

    virtual const void* get_member_ptr () const = 0;

    virtual bool involves_parameter (const void*) const = 0;

  };

  class input_action :
    public action {

  private:
    const automaton* m_owner;

  public:
    input_action (automaton* automaton,
		  const automaton* owner) :
      action (automaton),
      m_owner (owner)
    { }
    
    virtual ~input_action () { }

    virtual void decompose () = 0;

    const automaton* get_owner () const {
      return m_owner;
    }

    bool operator== (const input_action& ia) const {
      return get_member_ptr () == ia.get_member_ptr ();
    }

    virtual const void* get_member_ptr () const = 0;

    virtual bool involves_parameter (const void*) const = 0;

  };

  class internal_action :
    public local_action {

  public:
    internal_action (automaton* automaton) :
      local_action (automaton)
    { }

  };

  // Introduce typing to output and input actions.
  struct no_value {
    typedef untyped value_status;
  };

  template <class Member>
  struct value {
    typedef typed value_status;
    typedef typename Member::value_type value_type;
  };

  class untyped_output_action :
    public output_action {

  public:
    untyped_output_action (automaton* automaton) :
      output_action (automaton)
    { }

    virtual ~untyped_output_action () { }

    virtual bool operator() () = 0;

  };

  template <class T>
  class typed_output_action :
    public output_action {

  public:
    typed_output_action (automaton* automaton) :
      output_action (automaton)
    { }

    virtual ~typed_output_action () { }

    virtual std::pair<bool, T> operator() () = 0;

  };

  class untyped_input_action :
    public input_action {

  public:
    untyped_input_action (automaton* automaton,
			  const automaton* owner) :
      input_action (automaton, owner)
    { }

    virtual ~untyped_input_action () { }

    virtual void operator() () = 0;

  };

  template <class T>
  class typed_input_action :
    public input_action {

  public:
    typed_input_action (automaton* automaton,
			const automaton* owner) :
      input_action (automaton, owner)
    { }

    virtual ~typed_input_action () { }
  
    virtual void operator() (const T t) = 0;

  };

  // Member.
  template <class Member>
  class member {

  protected:
    Member& m_member;

  public:
    member (Member& m) :
      m_member (m)
    { }
    
  };

  // Callback
  template <class Callback>
  class callback {

  protected:
    Callback m_callback;

  public:
    callback (Callback& callback) :
      m_callback (callback)
    { }

  };

  // No parameter.
  struct no_parameter {
    typedef unparameterized parameter_status;
  };

  // Parameter.
  template <class Member>
  class parameter {
  public:
    typedef parameterized parameter_status;
    typedef typename Member::parameter_type parameter_type;

  protected:
    parameter_type* m_parameter;

  public:
    parameter (parameter_type* parameter) :
      m_parameter (parameter)
    { }
  };

  // Output action implementations.
  // template <class Member>
  // class unparameterized_untyped_output_action :
  //   public untyped_output_action,
  //   public member<Member>,
  //   public no_value,
  //   public no_parameter {

  // public:
  //   unparameterized_untyped_output_action (automaton* automaton,
  // 					   Member& m) :
  //     untyped_output_action (automaton),
  //     member<Member> (m)
  //   { }
    
  //   bool involves_parameter (const void*) const {
  //     return false;
  //   }

  //   const void* get_member_ptr () const {
  //     return &this->m_member;
  //   }

  //   bool operator() () {
  //     return this->m_member ();
  //   }

  //   void execute () {
  //     (*this) ();
  //   }
  // };

  template <class Member>
  class parameterized_untyped_output_action :
    public untyped_output_action,
    public member<Member>,
    public no_value,
    public parameter<Member> {

  public:
    typedef typename parameter<Member>::parameter_type parameter_type;

    parameterized_untyped_output_action (automaton* automaton,
					 Member& m,
					 parameter_type* p) :
      untyped_output_action (automaton),
      member<Member> (m),
      parameter<Member> (p)
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

  template <class Member>
  class unparameterized_typed_output_action :
    public member<Member>,
    public value<Member>,
    public no_parameter,
    public typed_output_action<typename value<Member>::value_type> {

  public:
    typedef typename value<Member>::value_type value_type;

    unparameterized_typed_output_action (automaton* automaton,
					 Member& m) :
      member<Member> (m),
      typed_output_action<value_type> (automaton)
    { }
    
    bool involves_parameter (const void* parameter) const {
      return false;
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    std::pair<bool, value_type> operator() () {
      return this->m_member ();
    }

    void execute () {
      (*this) ();
    }

  };

  template <class Member>
  class parameterized_typed_output_action :
    public member<Member>,
    public value<Member>,
    public parameter<Member>,
    public typed_output_action<typename value<Member>::value_type> {

  public:
    typedef typename value<Member>::value_type value_type;
    typedef typename parameter<Member>::parameter_type parameter_type;

    parameterized_typed_output_action (automaton* automaton,
				       Member& m,
				       parameter_type* p) :
      member<Member> (m),
      parameter<Member> (p),
      typed_output_action<value_type> (automaton)
    { }

    bool involves_parameter (const void* parameter) const {
      return this->m_parameter == parameter;
    }

    const void* get_member_ptr () const {
      return &this->m_member;
    }

    std::pair<bool, value_type> operator() () {
      return this->m_member (this->m_parameter); 
    }

    void execute () {
      (*this) ();
    }

  };

  // Input action implementations.
  template <class Member, class Callback>
  class unparameterized_untyped_input_action : 
    public untyped_input_action,
    public member<Member>,
    public callback<Callback>,
    public no_value,
    public no_parameter {

  public:
    unparameterized_untyped_input_action (automaton* automaton,
					  Member& m,
					  const automaton* owner,
					  Callback& c) :
      untyped_input_action (automaton, owner),
      member<Member> (m),
      callback<Callback> (c)
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

  template <class Member, class Callback>
  class parameterized_untyped_input_action : 
    public untyped_input_action,
    public member<Member>,
    public no_value,
    public parameter<Member>,
    public callback<Callback> {

  public:
    typedef typename parameter<Member>::parameter_type parameter_type;

    parameterized_untyped_input_action (automaton* automaton,
					Member& m,
					const automaton* owner,
					Callback& c,
					parameter_type* p) :
      untyped_input_action (automaton, owner),
      member<Member> (m),
      parameter<Member> (p),
      callback<Callback> (c)
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

  template <class Member, class Callback>
  class unparameterized_typed_input_action : 
    public member<Member>,
    public callback<Callback>,
    public value<Member>,
    public no_parameter,
    public typed_input_action<typename value<Member>::value_type> {

  public:
    typedef typename value<Member>::value_type value_type;

    unparameterized_typed_input_action (automaton* automaton,
					Member& m,
					const automaton* owner,
					Callback& c) :
      member<Member> (m),
      callback<Callback> (c),
      typed_input_action<value_type> (automaton, owner)
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

    void operator() (const typename Member::value_type t) {
      this->m_member (t);
    }

  };

  template <class Member, class Callback>
  class parameterized_typed_input_action : 
    public member<Member>,
    public callback<Callback>,
    public value<Member>,
    public parameter<Member>,
    public typed_input_action<typename value<Member>::value_type> {

  public:
    typedef typename value<Member>::value_type value_type;
    typedef typename parameter<Member>::parameter_type parameter_type;

    parameterized_typed_input_action (automaton* automaton,
				      Member& m,
				      const automaton* owner,
				      Callback& c,
				      parameter_type* p) :
      member<Member> (m),
      callback<Callback> (c),
      parameter<Member> (p),
      typed_input_action<value_type> (automaton, owner)
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

    void operator() (const typename Member::value_type t) {
      this->m_member (t, this->m_parameter);
    }

  };

  // Internal action implementations.
  template <class Member>
  class unparameterized_internal_action :
    public internal_action,
    public member<Member>,
    public no_parameter {

  public:

    unparameterized_internal_action (automaton* automaton,
				     Member& m) :
      internal_action (automaton),
      member<Member> (m)
    { }
    
    const void* get_member_ptr () const {
      return &this->m_member;
    }

    void execute () {
      this->m_member ();
    }
  };

  template <class Member>
  class parameterized_internal_action :
    public internal_action,
    public member<Member>,
    public parameter<Member> {

  public:
    typedef typename parameter<Member>::parameter_type parameter_type;

    parameterized_internal_action (automaton* automaton,
				   Member& m,
				   parameter_type* p) :
      internal_action (automaton),
      member<Member> (m),
      parameter<Member> (p)
    { }

    void execute () {
      this->m_member (this->m_parameter);
    }
    
  };

  // Free input implementation.
  template <class Member>
  class free_input_action :
    public internal_action,
    public member<Member> {

  public:
    typedef typename Member::value_type value_type;

  private:
    value_type m_t;
    
  public:
    free_input_action (automaton* automaton,
		       Member& m,
		       const value_type& t) :
      internal_action (automaton),
      member<Member> (m),
      m_t (t)
    { }

    void execute () {
      this->m_member (m_t);
    }
    
  };

  // Callback implementation.
  template <class Callback>
  class callback_action :
    public internal_action {

  private:
    Callback m_callback;
    
  public:
    callback_action (automaton* automaton,
		     const Callback& callback) :
      internal_action (automaton),
      m_callback (callback)
    { }

    void execute () {
      m_callback ();
    }
    
  };

  struct action_compare {
    bool operator() (const action* x,
		     const action* y) {
      return x->get_automaton() < y->get_automaton();
    }
  };

  class macro_action {

  public:
    virtual ~macro_action () { }
    virtual void execute () const = 0;
    virtual bool involves_output (const output_action& output_action) const = 0;
    virtual bool involves_input (const input_action& input_action) const = 0;
    virtual bool involves_input_check_owner (const input_action& input_action) const = 0;
    virtual bool involves_automaton (const automaton* automaton) const = 0;

    virtual bool empty () const = 0;
    virtual void decompose (const input_action& input_action) = 0;
    virtual void decompose (const automaton*, const void* parameter) = 0;
    virtual void decompose (const automaton*) = 0;
 };

  template <class OutputAction, class InputAction>
  class macro_action_base :
    public macro_action {

  protected:
    OutputAction* m_output_action;
    typedef std::list<InputAction*> list_type;
    list_type m_input_actions;

  public:
    template <class T>
    macro_action_base (const T& output_action) :
      m_output_action (new T (output_action))
    { }

    ~macro_action_base () {
      for(typename list_type::const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	(*pos)->decompose();
  	delete (*pos);
      }
      delete m_output_action;
    }

    bool involves_output (const output_action& output_action) const {
      return *m_output_action == output_action;
    }

    bool involves_input (const input_action& input_action) const {
      for(typename list_type::const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
  	if((*(*pos)) == input_action) {
  	  return true;
  	}
      }
      return false;
    }

    bool involves_input_check_owner (const input_action& input_action) const {
      for(typename list_type::const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
  	if((*(*pos)) == input_action && (*(*pos)).get_owner () == input_action.get_owner ()) {
  	  return true;
  	}
      }
      return false;
    }

    bool involves_automaton (const automaton* automaton) const {
      if (m_output_action->get_automaton () == automaton) {
	return true;
      }
      for (typename list_type::const_iterator pos = m_input_actions.begin();
	   pos != m_input_actions.end();
	   ++pos) {
  	if((*(*pos)).get_automaton () == automaton) {
  	  return true;
  	}
      }
      return false;
    }
    
    template <class T>
    void add_input (const T& input_action) {
      InputAction* ia = new T (input_action);
      list_type temp;
      temp.push_back (ia);
      m_input_actions.merge (temp, action_compare ());
    }

    bool empty () const {
      return m_input_actions.empty ();
    }

    void decompose (const input_action& input_action) {
      typename list_type::iterator pos;
      for (pos = m_input_actions.begin ();
	   pos != m_input_actions.end ();
	   ++pos) {
      	if((*(*pos)) == input_action && (*(*pos)).get_owner () == input_action.get_owner ()) {
      	  break;
      	}
      }

      if (pos != m_input_actions.end ()) {
      	(*pos)->decompose ();
      	delete (*pos);
      	m_input_actions.erase (pos);
      }
    }

    void decompose (const automaton* automaton,
		    const void* parameter) {
      if (m_output_action->get_automaton () == automaton &&
	  m_output_action->involves_parameter (parameter)) {
	typename list_type::const_iterator pos;
	for (pos = m_input_actions.begin ();
	     pos != m_input_actions.end ();
	     ++pos) {
	  (*pos)->decompose ();
	  delete (*pos);
	}
	m_input_actions.clear ();
      }
      else {
	typename list_type::iterator pos = m_input_actions.begin ();
	while (pos != m_input_actions.end ()) {
	  if((*pos)->get_automaton () == automaton &&
	     (*pos)->involves_parameter (parameter)) {
	    (*pos)->decompose ();
	    delete (*pos);
	    pos = m_input_actions.erase (pos);
	  }
	  else {
	    ++pos;
	  }
	}
      }
    }

    void decompose (const automaton* automaton) {
      if (m_output_action->get_automaton () == automaton) {
	typename list_type::const_iterator pos;
	for (pos = m_input_actions.begin ();
	     pos != m_input_actions.end ();
	     ++pos) {
	  (*pos)->decompose ();
	  delete (*pos);
	}
	m_input_actions.clear ();
      }
      else {
	typename list_type::iterator pos = m_input_actions.begin ();
	while (pos != m_input_actions.end ()) {
	  if((*pos)->get_automaton () == automaton ||
	     (*pos)->get_owner () == automaton) {
	    (*pos)->decompose ();
	    delete (*pos);
	    pos = m_input_actions.erase (pos);
	  }
	  else {
	    ++pos;
	  }
	}
      }
    }

    virtual void execute_core () const = 0;

    void execute () const {
      // Acquire locks (in order).
      bool locked_output = false;
      automaton* ao = m_output_action->get_automaton();
      
      for(typename list_type::const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	automaton* ai = (*pos)->get_automaton();
  	if(!locked_output && ao < ai) {
  	  ao->lock();
  	  locked_output = true;
  	}
  	ai->lock();
      }

      // Execute.
      execute_core ();
      
      // Release locks.
      locked_output = false;
      for(typename list_type::const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	automaton* ai = (*pos)->get_automaton();
  	if(!locked_output && ao < ai) {
  	  ao->unlock();
  	  locked_output = true;
  	}
  	ai->unlock();
      }
    }
  };

  class untyped_macro_action :
    public macro_action_base<untyped_output_action, untyped_input_action> {

  public:
    template <class T>
    untyped_macro_action (const T& output) :
      macro_action_base<untyped_output_action, untyped_input_action> (output)
    { }

    void execute_core () const {
      scheduler_interface& scheduler = get_scheduler ();
      scheduler.set_current_automaton (m_output_action->get_automaton ());
      bool t = (*m_output_action) ();
      if (t) {
	for(list_type::const_iterator pos = m_input_actions.begin();
	    pos != m_input_actions.end();
	    ++pos) {
	  scheduler.set_current_automaton ((*pos)->get_automaton ());
	  (*(*pos)) ();
	}
      }
      scheduler.set_current_automaton (0);
    }

  };

  template <class T>
  class typed_macro_action :
    public macro_action_base<typed_output_action<T>, typed_input_action<T> > {
  public:
    template <class U>
    typed_macro_action (const U& output) :
      macro_action_base<typed_output_action<T>, typed_input_action<T> > (output)
    { }

    void execute_core () const {
      scheduler_interface& scheduler = get_scheduler ();
      scheduler.set_current_automaton (this->m_output_action->get_automaton ());
      std::pair<bool, T> t = (*this->m_output_action) ();
      if (t.first) {
	for (typename macro_action_base<typed_output_action<T>, typed_input_action<T> >::list_type::const_iterator pos = this->m_input_actions.begin();
	     pos != this->m_input_actions.end();
	     ++pos) {
	  scheduler.set_current_automaton ((*pos)->get_automaton ());
	  (*(*pos)) (t.second);
	}
      }
      scheduler.set_current_automaton (0);
    }

  };

  template <class K, class VS, class PS, class M> class ac;

  template <class VS, class PS, class M> class ac<input, VS, PS, M> :
    public input_action {
    
  public:
    ac (automaton* automaton,
	const automaton* owner) :
      input_action (automaton, owner)
    { }

    const void* get_member_ptr () const {
      return 0;
    }

    bool involves_parameter (const void* parameter) const {
      return false;
    }

    void decompose () {
    }

  };
  
  template <class M> class ac<output, untyped, unparameterized, M> :
    public member<M>,
    public untyped_output_action {

  public:
    typedef untyped value_status;

    ac (automaton* automaton,
	M& m) :
      member<M> (m),
      untyped_output_action (automaton)
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

  ac<input, untyped, unparameterized, int> ac_input_untyped_unparameterized (0, 0);
  ac<input, untyped, parameterized, int> ac_input_untyped_parameterized (0, 0);
  ac<input, typed, unparameterized, int> ac_input_typed_unparameterized (0, 0);
  ac<input, typed, parameterized, int> ac_input_typed_parameterized (0, 0);

  struct x {
    bool operator() () {
      return false;
    }
  };
  x t;

  ac<output, untyped, unparameterized, x> ac_output_untyped_unparameterized (0, t);
  //ac<output, untyped, parameterized, int> ac_output_untyped_parameterized (0);
  //ac<output, typed, unparameterized, int> ac_output_typed_unparameterized (0);
  //ac<output, typed, parameterized, int> ac_output_typed_parameterized (0);

  template <class Member>
  class unparameterized_untyped_output_action :
    public ac<output, untyped, unparameterized, Member> {

  public:
    unparameterized_untyped_output_action (automaton* automaton,
					   Member& member) :
      ac<output, untyped, unparameterized, Member> (automaton, member)
    { }
  };

}

#endif
