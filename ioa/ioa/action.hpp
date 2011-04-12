#ifndef __action_hpp__
#define __action_hpp__

#include <set>
#include "automaton.hpp"

namespace ioa {

  struct input { };
  struct output { };
  struct internal { };
  struct free_input { };

  struct untyped { };
  struct typed { };

  struct unparameterized { };
  struct parameterized { };

  template <class Direction, class TypeStatus, class Type, class ParameterStatus, class Parameter>
  struct action_trait {
    typedef Direction direction;
    typedef TypeStatus type_status;
    typedef Type type;
    typedef ParameterStatus parameter_status;
    typedef Parameter parameter;
  };

  // Interfaces.
  class action_interface {
  public:
    virtual ~action_interface () { }

    virtual abstract_automaton* get_automaton () const = 0;
    
    virtual const void* get_member_ptr () const = 0;

    virtual void print_on (std::ostream&) const = 0;

    bool operator== (const action_interface& ai) const {
      return get_member_ptr () == ai.get_member_ptr ();
    }
    
  };

  std::ostream& operator<<(std::ostream& output, const action_interface&);

  class local_action_interface :
    public action_interface {
    
  public:
    virtual void execute () = 0;
    
  };

  class input_action_interface :
    public action_interface {

  public:
    virtual ~input_action_interface () { }

    virtual void decompose () = 0;

    virtual const abstract_automaton* get_owner () const = 0;
  };

  class untyped_output_action_interface :
    public local_action_interface {

  public:
    virtual ~untyped_output_action_interface () { }

    virtual bool operator() () = 0;

  };

  template <class T>
  class typed_output_action_interface :
    public local_action_interface {

  public:
    virtual ~typed_output_action_interface () { }

    virtual std::pair<bool, T> operator() () = 0;

  };

  class untyped_input_action_interface :
    public input_action_interface {

  public:
    virtual ~untyped_input_action_interface () { }

    virtual void operator() () = 0;

  };

  template <class T>
  class typed_input_action_interface :
    public input_action_interface {

  public:
    virtual ~typed_input_action_interface () { }
  
    virtual void operator() (const T t) = 0;

  };

  // Action.
  template <class Instance, class Member>
  class action {

  private:
    automaton<Instance>* m_automaton;
    Member& m_member;

  public:
    action (automaton<Instance>* automaton,
	    Member Instance::*member) :
      m_automaton (automaton),
      m_member ((m_automaton->get_typed_instance ())->*member)
    { }
    
    abstract_automaton* get_automaton () const {
      return m_automaton;
    }

    Member& get_member () const {
      return m_member;
    }
    
  };

  // Local action.
  template <class Instance, class Member>
  class local_action :
    public action<Instance, Member> {

  public:
    local_action (automaton<Instance>* automaton,
		  Member Instance::*member) :
      action<Instance, Member> (automaton, member)
    { }
  };

  // Input action.
  template <class Instance, class Member, class Callback>
  class input_action :
    public action<Instance, Member> {

  private:
    const abstract_automaton* m_owner;
    Callback m_callback;

  public:
    input_action (automaton<Instance>* automaton,
  		  Member Instance::*member,
  		  const abstract_automaton* owner,
  		  Callback& callback) :
      action<Instance, Member> (automaton, member),
      m_owner (owner),
      m_callback (callback)
    { }

    const abstract_automaton* get_owner () const {
      return m_owner;
    }

    void decompose () {
      m_callback.decomposed ();
    }
  };

  // Parameter helper.
  template <class Parameter>
  class parameter {
    
  private:
    Parameter* m_parameter;
    
  public:
    parameter (Parameter* parameter) :
      m_parameter (parameter)
    { }

    Parameter* get_parameter () const {
      return m_parameter;
    }
  };

  // Output action implementations.
  template <class Instance, class Member>
  class unparameterized_untyped_output_action :
    public untyped_output_action_interface {
  private:
    local_action<Instance, Member> m_action;
  public:
    typedef untyped type_status;

    unparameterized_untyped_output_action (automaton<Instance>* automaton,
					   Member Instance::*member) :
      m_action (automaton, member)
    { }
    
    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "up_ut_output";
    }

    bool operator() () {
      return (m_action.get_member ()) ();
    }

    void execute () {
      (m_action.get_member ()) ();
    }
  };

  template <class Instance, class Member, class Parameter>
  class parameterized_untyped_output_action :
    public untyped_output_action_interface {

  private:
    local_action<Instance, Member> m_action;
    parameter<Parameter> m_parameter;
    
  public:
    typedef untyped type_status;

    parameterized_untyped_output_action (automaton<Instance>* automaton,
					 Member Instance::*member,
					 Parameter* parameter) :
      m_action (automaton, member),
      m_parameter (parameter)
    { }

    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "p_ut_output";
    }

    bool operator() () {
      return (m_action.get_member ()) (m_parameter.get_parameter ());
    }

    void execute () {
      (m_action.get_member ()) (m_parameter.get_parameter ());
    }
    
  };

  template <class Instance, class Member>
  class unparameterized_typed_output_action :
    public typed_output_action_interface<typename Member::type> {

  private:
    local_action<Instance, Member> m_action;

  public:
    typedef typed type_status;
    typedef typename Member::type type;

    unparameterized_typed_output_action (automaton<Instance>* automaton,
					 Member Instance::*member) :
      m_action (automaton, member)
    { }
    
    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "up_t_output";
    }

    std::pair<bool, type> operator() () {
      return (m_action.get_member ()) ();
    }

    void execute () {
      (m_action.get_member ()) ();
    }

  };

  template <class Instance, class Member, class Parameter>
  class parameterized_typed_output_action :
    public typed_output_action_interface<typename Member::type> {

  private:
    local_action<Instance, Member> m_action;
    parameter<Parameter> m_parameter;

  public:
    typedef typed type_status;
    typedef typename Member::type type;

    parameterized_typed_output_action (automaton<Instance>* automaton,
				       Member Instance::*member,
				       Parameter* parameter) :
      m_action (automaton, member),
      m_parameter (parameter)
    { }

    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "p_t_output";
    }

    std::pair<bool, type> operator() () {
      return (m_action.get_member ()) (m_parameter.get_parameter ());
    }

    void execute () {
      (m_action.get_member ()) (m_parameter.get_parameter ());
    }

  };

  // Input action implementations.
  template <class Instance, class Member, class Callback>
  class unparameterized_untyped_input_action : 
    public untyped_input_action_interface {

  private:
    input_action<Instance, Member, Callback> m_action;

  public:
    unparameterized_untyped_input_action (automaton<Instance>* automaton,
					  Member Instance::*member,
					  const abstract_automaton* owner,
					  Callback& callback) :
      m_action (automaton, member, owner, callback)
    { }

    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "up_ut_input";
    }

    void decompose () {
      m_action.decompose ();
    }

    const abstract_automaton* get_owner () const {
      return m_action.get_owner ();
    }

    void operator() () {
      (m_action.get_member ()) ();
    }

  };

  template <class Instance, class Member, class Callback, class Parameter>
  class parameterized_untyped_input_action : 
    public untyped_input_action_interface {

  private:
    input_action<Instance, Member, Callback> m_action;
    parameter<Parameter> m_parameter;

  public:
    parameterized_untyped_input_action (automaton<Instance>* automaton,
					Member Instance::*member,
					const abstract_automaton* owner,
					Callback& callback,
					Parameter* parameter) :
      m_action (automaton, member, owner, callback),
      m_parameter (parameter)
    { }

    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "p_ut_input";
    }

    void decompose () {
      m_action.decompose ();
    }

    const abstract_automaton* get_owner () const {
      return m_action.get_owner ();
    }

    void operator() () {
      (m_action.get_member ()) (m_parameter.get_parameter ());
    }

  };

  template <class Instance, class Member, class Callback>
  class unparameterized_typed_input_action : 
    public typed_input_action_interface<typename Member::type> {

  private:
    input_action<Instance, Member, Callback> m_action;

  public:
    unparameterized_typed_input_action (automaton<Instance>* automaton,
					Member Instance::*member,
					const abstract_automaton* owner,
					Callback& callback) :
      m_action (automaton, member, owner, callback)
    { }

    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "up_t_input";
    }

    void decompose () {
      m_action.decompose ();
    }

    const abstract_automaton* get_owner () const {
      return m_action.get_owner ();
    }

    void operator() (const typename Member::type t) {
      (m_action.get_member ()) (t);
    }

  };

  template <class Instance, class Member, class Callback, class Parameter>
  class parameterized_typed_input_action : 
    public typed_input_action_interface<typename Member::type> {

  private:
    input_action<Instance, Member, Callback> m_action;
    parameter<Parameter> m_parameter;

  public:
    parameterized_typed_input_action (automaton<Instance>* automaton,
				      Member Instance::*member,
				      const abstract_automaton* owner,
				      Callback& callback,
				      Parameter* parameter) :
      m_action (automaton, member, owner, callback),
      m_parameter (parameter)
    { }

    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "p_t_input";
    }

    void decompose () {
      m_action.decompose ();
    }

    const abstract_automaton* get_owner () const {
      return m_action.get_owner ();
    }

    void operator() (const typename Member::type t) {
      (m_action.get_member ()) (t, m_parameter.get_parameter ());
    }

  };

  // Internal action implementations.
  template <class Instance, class Member>
  class unparameterized_internal_action :
    public local_action_interface {
  private:
    local_action<Instance, Member> m_action;
  public:
    unparameterized_internal_action (automaton<Instance>* automaton,
				     Member Instance::*member) :
      m_action (automaton, member)
    { }
    
    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "up_internal";
    }

    bool operator() () {
      return (m_action.get_member ()) ();
    }

    void execute () {
      (m_action.get_member ()) ();
    }
  };

  template <class Instance, class Member, class Parameter>
  class parameterized_internal_action :
    public local_action_interface {

  private:
    local_action<Instance, Member> m_action;
    parameter<Parameter> m_parameter;
    
  public:
    parameterized_internal_action (automaton<Instance>* automaton,
				   Member Instance::*member,
				   Parameter* parameter) :
      m_action (automaton, member),
      m_parameter (parameter)
    { }

    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    bool operator() () {
      return (m_action.get_member ()) (m_parameter.get_parameter ());
    }

    void execute () {
      (m_action.get_member ()) (m_parameter.get_parameter ());
    }
    
  };

  // Free input implementation.
  template <class Instance, class Member, class T>
  class free_input_action :
    public local_action_interface {

  private:
    local_action<Instance, Member> m_action;
    T m_t;
    
  public:
    free_input_action (automaton<Instance>* automaton,
		       Member Instance::*member,
		       const T& t) :
      m_action (automaton, member),
      m_t (t)
    { }

    abstract_automaton* get_automaton () const {
      return m_action.get_automaton ();
    }

    const void* get_member_ptr () const {
      return &m_action.get_member ();
    }

    void print_on (std::ostream& out) const {
      out << "free_input";
    }

    void execute () {
      (m_action.get_member ()) (m_t);
    }
    
  };

  // Callback implementation.
  template <class Instance, class Callback>
  class callback_action :
    public local_action_interface {

  private:
    automaton<Instance>* m_automaton;
    Callback m_callback;
    
  public:
    callback_action (automaton<Instance>* automaton,
		     const Callback& callback) :
      m_automaton (automaton),
      m_callback (callback)
    { }

    abstract_automaton* get_automaton () const {
      return m_automaton;
    }

    const void* get_member_ptr () const {
      return 0;
    }

    void print_on (std::ostream& out) const {
      out << "callback";
    }

    void execute () {
      m_callback ();
    }
    
  };

  struct action_compare {
    bool operator() (const action_interface* x,
		     const action_interface* y) {
      return x->get_automaton() < y->get_automaton();
    }
  };

  class macro_action_interface {

  public:
    virtual ~macro_action_interface () { }
    virtual void execute () const = 0;
    virtual bool involves_output (const local_action_interface& output_action) const = 0;
    virtual bool involves_input (const input_action_interface& input_action) const = 0;
    virtual bool involves_input_check_owner (const input_action_interface& input_action) const = 0;
    virtual bool involves_automaton (const abstract_automaton* automaton) const = 0;

    virtual bool empty () const = 0;
    virtual void decompose (const abstract_automaton* aa,
			    const void* input,
			    const abstract_automaton* owner) = 0;

 };

  template <class OutputAction, class InputAction>
  class macro_action_base :
    public macro_action_interface {

  protected:
    OutputAction* m_output_action;
    typedef std::set<InputAction*, action_compare > set_type;
    typedef typename set_type::const_iterator const_iterator;
    set_type m_input_actions;

  public:
    template <class T>
    macro_action_base (const T& output_action) :
      m_output_action (new T (output_action))
    { }

    ~macro_action_base () {
      for(const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	(*pos)->decompose();
  	delete (*pos);
      }
      delete m_output_action;
    }

    bool involves_output (const local_action_interface& output_action) const {
      return *m_output_action == output_action;
    }

    bool involves_input (const input_action_interface& input_action) const {
      for(const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
  	if((*(*pos)) == input_action) {
  	  return true;
  	}
      }
      return false;
    }

    bool involves_input_check_owner (const input_action_interface& input_action) const {
      for(const_iterator pos = m_input_actions.begin();
      	  pos != m_input_actions.end();
      	  ++pos) {
  	if((*(*pos)) == input_action && (*(*pos)).get_owner () == input_action.get_owner ()) {
  	  return true;
  	}
      }
      return false;
    }

    bool involves_automaton (const abstract_automaton* automaton) const {
      if (m_output_action->get_automaton () == automaton) {
	return true;
      }
      for (const_iterator pos = m_input_actions.begin();
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
      m_input_actions.insert (ia);
    }

    bool empty () const {
      return m_input_actions.empty ();
    }

    void decompose (const abstract_automaton* aa,
		    const void* input,
		    const abstract_automaton* owner) {
      BOOST_ASSERT (false);
      // const_iterator pos;
      // for(pos = m_input_actions.begin();
      // 	  pos != m_input_actions.end();
      // 	  ++pos) {
      // 	if((*pos)->is_action_owner(aa, input, owner)) {
      // 	  break;
      // 	}
      // }

      // if(pos != m_input_actions.end()) {
      // 	(*pos)->decompose();
      // 	delete (*pos);
      // 	m_input_actions.erase(pos);
      // }
    }

    virtual void execute_core () const = 0;

    void execute () const {
      // Acquire locks (in order).
      bool locked_output = false;
      abstract_automaton* ao = m_output_action->get_automaton();
      
      for(const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	abstract_automaton* ai = (*pos)->get_automaton();
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
      for(const_iterator pos = m_input_actions.begin();
  	  pos != m_input_actions.end();
  	  ++pos) {
  	abstract_automaton* ai = (*pos)->get_automaton();
  	if(!locked_output && ao < ai) {
  	  ao->unlock();
  	  locked_output = true;
  	}
  	ai->unlock();
      }
    }
  };

  template <class Scheduler>
  class untyped_macro_action :
    public macro_action_base<untyped_output_action_interface, untyped_input_action_interface> {

  private:
    Scheduler& m_scheduler;

  public:
    template <class T>
    untyped_macro_action (const T& output,
			  Scheduler& scheduler) :
      macro_action_base<untyped_output_action_interface, untyped_input_action_interface> (output),
      m_scheduler (scheduler)
    { }

    void execute_core () const {
      m_scheduler.set_current_automaton (m_output_action->get_automaton ());
      bool t = (*m_output_action) ();
      if (t) {
	for(const_iterator pos = m_input_actions.begin();
	    pos != m_input_actions.end();
	    ++pos) {
	  m_scheduler.set_current_automaton ((*pos)->get_automaton ());
	  (*(*pos)) ();
	}
      }
      m_scheduler.set_current_automaton (0);
    }

  };

  template <class T, class Scheduler>
  class typed_macro_action :
    public macro_action_base<typed_output_action_interface<T>, typed_input_action_interface<T> > {

  private:
    Scheduler& m_scheduler;

  public:
    template <class U>
    typed_macro_action (const U& output,
			Scheduler& scheduler) :
      macro_action_base<typed_output_action_interface<T>, typed_input_action_interface<T> > (output),
      m_scheduler (scheduler)
    { }

    void execute_core () const {
      m_scheduler.set_current_automaton (this->m_output_action->get_automaton ());
      std::pair<bool, T> t = (*this->m_output_action) ();
      if (t.first) {
	for (typename macro_action_base<typed_output_action_interface<T>, typed_input_action_interface<T> >::const_iterator pos = this->m_input_actions.begin();
	     pos != this->m_input_actions.end();
	     ++pos) {
	  m_scheduler.set_current_automaton ((*pos)->get_automaton ());
	  (*(*pos)) (t.second);
	}
      }
      m_scheduler.set_current_automaton (0);
    }

  };

}

#endif