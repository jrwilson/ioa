#ifndef __ioa_hpp__
#define __ioa_hpp__

#include <memory>
#include <set>
#include <queue>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include "scheduler.hpp"

namespace ioa {

  template <class C, class T, void (C::*member)(const T)>
  class input_wrapper :
    public input,
    public value<T>,
    public no_parameter
 {
  private:
    C& m_c;
    
  public:
    input_wrapper (C& c)
      : m_c (c) { }
    
    void operator() (const T t) {
      (m_c.*member) (t);
    }

    void composed (bool c) {
      std::cout << "input composed" << std::endl;
    }
  };

  // TODO: Combine with previous.
  template <class C, void (C::*member)()>
  class void_input_wrapper :
    public input,
    public no_value,
    public no_parameter
  {
  private:
    C& m_c;
    
  public:
    void_input_wrapper (C& c)
      : m_c (c) { }
    
    void operator() () {
      (m_c.*member) ();
    }

    void composed (bool c) {
      std::cout << "input composed" << std::endl;
    }
  };
  
  template <class C, class T, std::pair<bool, T> (C::*member)(void)>
  class output_wrapper :
    public output,
    public value<T>,
    public no_parameter
 {
  private:
    C& m_c;
    
  public:
    output_wrapper (C& c) :
      m_c (c) { }
    
    std::pair<bool, T> operator() () {
      return (m_c.*member) ();
    }

    void composed (bool c) {
      std::cout << "output composed" << std::endl;
    }
  };

  template <class C, bool (C::*member)(void)>
  class void_output_wrapper :
    public output,
    public no_value,
    public no_parameter
  {
  private:
    C& m_c;
    
  public:
    void_output_wrapper (C& c) :
      m_c (c) { }
    
    bool operator() () {
      return (m_c.*member) ();
    }

    void composed (bool c) {
      std::cout << "output composed" << std::endl;
    }
  };

  template <class C, void (C::*member)()>
  class internal_wrapper :
    public internal,
    public no_parameter
 {
  private:
    C& m_c;
    
  public:
    internal_wrapper (C& c)
      : m_c (c) { }
    
    void operator() () {
      (m_c.*member) ();
    }
  };


  
  // template <class Instance>
  // struct root_callback {
  //   void operator() (const create_result<Instance>& r) {
  //     switch (r.type) {
  //     case AUTOMATON_CREATED:
  // 	// Good.
  // 	break;
  //     case AUTOMATON_EXISTS:
  // 	// Bad.
  // 	abort ();
  // 	break;
  //     }
  //   }
  // };

  // class simple_scheduler;
  
  // class runnable {
  // protected:
  //   simple_scheduler& m_scheduler;
  // public:
  //   runnable (simple_scheduler& scheduler)
  //     : m_scheduler (scheduler)
  //   { }
  //   virtual ~runnable() { }
  //   virtual void operator() () = 0;
  //   virtual void print_on (std::ostream& output) const = 0;
  // };
  
  // static std::ostream& operator<<(std::ostream& output, const runnable& runnable) {
  //   runnable.print_on (output);
  //   return output;
  // }
   
  // class simple_scheduler :
  //   public scheduler_interface {

  // private:
 
  //   template<class Instance, class Action>
  //   class execute_local : public runnable {
  //   private:
  //     automaton_handle<Instance> m_handle;
  //     Action m_action;
      
  //   public:
  //     execute_local (simple_scheduler& scheduler,
  // 		      const automaton_handle<Instance>& handle,
  // 		      const Action& action)
  // 	: runnable (scheduler),
  // 	  m_handle (handle),
  // 	  m_action (action) { }
      
  //     void operator() () {
  // 	m_scheduler.m_system.execute (m_handle, m_action);
  //     }

  //     void print_on (std::ostream& output) const {
  // 	output << "action " << m_action;
  //     }
  //   };

  //   template<class Instance, class Action>
  //   class execute_output : public runnable {
  //   private:
  //     automaton_handle<Instance> m_handle;
  //     Action m_action;
      
  //   public:
  //     execute_output (simple_scheduler& scheduler,
  // 		      const automaton_handle<Instance>& handle,
  // 		      const Action& action)
  // 	: runnable (scheduler),
  // 	  m_handle (handle),
  // 	  m_action (action) { }
      
  //     void operator() () {
  // 	m_scheduler.m_system.execute (m_handle, m_action);
  //     }

  //     void print_on (std::ostream& output) const {
  // 	output << "action " << m_action;
  //     }
  //   };

  //   template <class Instance, class Callback>
  //   class create_parent_callback :
  //     public anonymous_internal
  //   {
  //   private:
  //     create_result<Instance> m_result;
  //     Callback m_callback;
  //   public:
  //     create_parent_callback (const create_result<Instance>& result,
  // 			      Callback& callback) :
  // 	m_result (result),
  // 	m_callback (callback)
  //     { }

  //     void operator() () {
  // 	m_callback (m_result);
  //     }
  //   };

  //   template <class Parent, class Child, class Callback>
  //   class create_callback1 {

  //   private:
  //     simple_scheduler& m_scheduler;
  //     automaton_handle<Parent> m_parent_handle;
  //     Callback m_callback;

  //   public:
  //     create_callback1 (simple_scheduler& scheduler,
  // 			const automaton_handle<Parent>& parent_handle,
  // 			Callback& callback) :
  // 	m_scheduler (scheduler),
  // 	m_parent_handle (parent_handle),
  // 	m_callback (callback) { }

  //     void operator() (const create_result<Child>& r) {
  // 	switch (r.type) {
  // 	case AUTOMATON_CREATED:
  // 	  // Start the child.
  // 	  m_scheduler.schedule_internal (r.handle, &Child::init);
  // 	  break;
  // 	case AUTOMATON_EXISTS:
  // 	  break;
  // 	}

  // 	create_parent_callback<Child, Callback> cb (r, m_callback);
  // 	m_scheduler.schedule_callback<Parent, create_parent_callback<Child, Callback> > (m_parent_handle, cb);
  //     }

  //   };

  //   template<class Parent, class Child, class Callback>
  //   class create : public runnable {
  //   private:
  //     automaton_handle<Parent> m_parent_handle;
  //     Child* m_child;
  //     Callback m_callback;

  //   public:
  //     create (simple_scheduler& scheduler,
  // 	      const ioa::automaton_handle<Parent>& parent_handle,
  // 	      Child* child,
  // 	      const Callback& callback)
  // 	: runnable (scheduler),
  // 	  m_parent_handle (parent_handle),
  // 	  m_child (child),
  // 	  m_callback (callback)
  //     { }

  //     void operator() () {
  // 	m_scheduler.m_system.create (m_parent_handle, m_child, m_callback);
  //     }

  //     void print_on (std::ostream& output) const {
  // 	output << "create parent=" << m_parent_handle.get_automaton ()->get_typed_instance () << " child=" << m_child;
  //     }
  //   };

  //   template <class Callback>
  //   class compose_owner_callback :
  //     public anonymous_internal
  //   {
  //   private:
  //     compose_result m_result;
  //     Callback m_callback;
  //   public:
  //     compose_owner_callback (const compose_result& result,
  // 			      Callback& callback) :
  // 	m_result (result),
  // 	m_callback (callback)
  //     { }
      
  //     void operator() () {
  // 	m_callback (m_result);
  //     }
  //   };

  //   template <class Instance, class Member>
  //   class compose_action_callback :
  //     public anonymous_internal
  //   {
  //   private:
  //     automaton_handle<Instance> m_handle;
  //     Member Instance::*m_member_ptr;
  //     bool m_value;

  //   public:
  //     compose_action_callback (const automaton_handle<Instance> handle,
  // 			       Member Instance::*member_ptr,
  // 			       bool value) :
  // 	m_handle (handle),
  // 	m_member_ptr (member_ptr),
  // 	m_value (value)
  //     { }

  //     void operator() () {
  // 	Instance* i = m_handle.get_automaton ()->get_typed_instance ();
  // 	Member& ref = (*i).*m_member_ptr;
  // 	ref.composed (m_value);
  //     }
  //   };

  //   template <class Instance, class OutputInstance, class OutputMember, class InputInstance, class InputMember, class Callback>
  //   class compose_callback1 {

  //   private:
  //     simple_scheduler& m_scheduler;
  //     automaton_handle<Instance> m_handle;
  //     automaton_handle<OutputInstance> m_output_handle;
  //     OutputMember OutputInstance::*m_output_member_ptr;
  //     automaton_handle<InputInstance> m_input_handle;
  //     InputMember InputInstance::*m_input_member_ptr;
  //     Callback m_callback;

  //   public:
  //     compose_callback1 (simple_scheduler& scheduler,
  // 			 const automaton_handle<Instance>& handle,
  // 			 const automaton_handle<OutputInstance>& output_handle,
  // 			 OutputMember OutputInstance::*output_member_ptr,
  // 			 const automaton_handle<InputInstance>& input_handle,
  // 			 InputMember InputInstance::*input_member_ptr,
  // 			 Callback& callback) :
  //     	m_scheduler (scheduler),
  //     	m_handle (handle),
  // 	m_output_handle (output_handle),
  // 	m_output_member_ptr (output_member_ptr),
  // 	m_input_handle (input_handle),
  // 	m_input_member_ptr (input_member_ptr),
  // 	m_callback (callback)
  //     { }

  //     void operator() (const compose_result& r) {
  //   	switch (r.type) {
  // 	case OUTPUT_INVALID:
  // 	case INPUT_INVALID:
  // 	case COMPOSITION_EXISTS:
  // 	case INPUT_UNAVAILABLE:
  // 	case OUTPUT_UNAVAILABLE:
  // 	  {
  // 	    compose_owner_callback<Callback> owner_cb (r, m_callback);
  // 	    m_scheduler.schedule_callback<Instance, compose_owner_callback<Callback> > (m_handle, owner_cb);
  // 	  }
  // 	  break;
  // 	case COMPOSED:
  // 	  {
  // 	    compose_owner_callback<Callback> owner_cb (r, m_callback);
  // 	    m_scheduler.schedule_callback<Instance, compose_owner_callback<Callback> > (m_handle, owner_cb);

  // 	    compose_action_callback<OutputInstance, OutputMember> output_cb (m_output_handle, m_output_member_ptr, true);
  // 	    m_scheduler.schedule_callback<OutputInstance, compose_action_callback<OutputInstance, OutputMember> > (m_output_handle, output_cb);

  // 	    compose_action_callback<InputInstance, InputMember> input_cb (m_input_handle, m_input_member_ptr, true);
  // 	    m_scheduler.schedule_callback<InputInstance, compose_action_callback<InputInstance, InputMember> > (m_input_handle, input_cb);
  // 	  }
  // 	  break;
  // 	case DECOMPOSED:
  // 	  BOOST_ASSERT (false);
  // 	  break;
  //   	}

  //     }

  //     void operator() () {
  // 	compose_result r = make_compose_result (DECOMPOSED);
  // 	compose_owner_callback<Callback> owner_cb (r, m_callback);
  // 	m_scheduler.schedule_callback<Instance, compose_owner_callback<Callback> > (m_handle, owner_cb);
	
  // 	compose_action_callback<OutputInstance, OutputMember> output_cb (m_output_handle, m_output_member_ptr, false);
  // 	m_scheduler.schedule_callback<OutputInstance, compose_action_callback<OutputInstance, OutputMember> > (m_output_handle, output_cb);
	
  // 	compose_action_callback<InputInstance, InputMember> input_cb (m_input_handle, m_input_member_ptr, false);
  // 	m_scheduler.schedule_callback<InputInstance, compose_action_callback<InputInstance, InputMember> > (m_input_handle, input_cb);
  //     }

  //   };

  //   template <class OutputAction,
  // 	      class InputAction,
  // 	      class Callback>
  //   class compose : public runnable {
  //   private:
  //     OutputAction m_output_action;
  //     InputAction m_input_action;
  //     Callback m_callback;

  //   public:
  //     compose (simple_scheduler& scheduler,
  // 	       const OutputAction& output_action,
  // 	       const InputAction& input_action,
  // 	       const Callback& callback)
  // 	: runnable (scheduler),
  // 	  m_output_action (output_action),
  // 	  m_input_action (input_action),
  // 	  m_callback (callback)
  //     { }

  //     void operator() () {
  // 	m_scheduler.m_system.compose<OutputAction, InputAction, Callback> (m_output_action, m_input_action, m_callback);
  //     }

  //     void print_on (std::ostream& output) const {
  // 	output << "compose owner=";
  //     }

  //   };

  //   template <class Instance, class Member>
  //   Member& ptr_to_member (const automaton_handle<Instance>& handle,
  // 			   Member Instance::*member_ptr) {
  //     Instance* instance = handle.get_automaton ()->get_typed_instance ();
  //     Member& member = (*instance).*member_ptr;
  //     return member;
  //   }

  //   template <class Instance, class Member>
  //   runnable* make_execute_output (simple_scheduler& scheduler,
  // 				   const automaton_handle<Instance>& handle,
  // 				   Member Instance::*member_ptr,
  // 				   unvalued /* */) {
  //     action<Member> output_action (handle,
  // 				    ptr_to_member (handle, member_ptr));
  //     return new execute_output<Instance, action<Member> > (scheduler, handle, output_action);
  //   }

  //   template <class Instance, class Member>
  //   runnable* make_execute_output (simple_scheduler& scheduler,
  // 				   const automaton_handle<Instance>& handle,
  // 				   Member Instance::*member_ptr,
  // 				   valued /* */) {
  //     action<Member> output_action (handle,
  // 				    ptr_to_member (handle, member_ptr));
  //     return new execute_output<Instance, action<Member> > (scheduler, handle, output_action);
  //   }

  //   template <class Instance, class Member>
  //   runnable* make_execute_internal (simple_scheduler& scheduler,
  // 				     const automaton_handle<Instance>& handle,
  // 				     Member Instance::*member_ptr) {
  //     action<Member> ac (handle,
  // 			 ptr_to_member (handle, member_ptr));
  //     return new execute_local<Instance, action<Member> > (scheduler, handle, ac);
  //   }
    
  //   template <class Instance, class Member>
  //   runnable* make_execute_free_input (simple_scheduler& scheduler,
  // 				       const automaton_handle<Instance>& handle,
  // 				       Member Instance::*member_ptr,
  // 				       const typename Member::value_type& t) {
  //     action<Member> ac (handle,
  // 			 ptr_to_member (handle, member_ptr),
  // 			 t);
  //     return new execute_local<Instance, action<Member> > (scheduler, handle, ac);
  //   }
    
  //   template <class Instance, class Callback>
  //   runnable* make_execute_callback (simple_scheduler& scheduler,
  // 				     const automaton_handle<Instance>& handle,
  // 				     const Callback& callback) {
  //     action<Callback> ac (handle,
  // 			   callback);
  //     return new execute_local<Instance, action<Callback> > (scheduler, handle, ac);
  //   }

  //   template <class Parent, class Child, class Callback>
  //   runnable* make_create (simple_scheduler& scheduler,
  // 			   const automaton_handle<Parent>& handle,
  // 			   Child* child,
  // 			   Callback& callback) {
  //     return new create<Parent, Child, Callback> (scheduler, handle, child, callback);
  //   }

  //   template <class Instance,
  // 	      class OutputInstance, class OutputMember,
  // 	      class InputInstance, class InputMember,
  // 	      class Callback>
  //   runnable* make_compose (simple_scheduler& scheduler,
  // 			    const automaton_handle<Instance>& handle,
  // 			    const automaton_handle<OutputInstance>& output_handle,
  // 			    OutputMember OutputInstance::*output_member_ptr,
  // 			    const automaton_handle<InputInstance>& input_handle,
  // 			    InputMember InputInstance::*input_member_ptr,
  // 			    Callback& callback) {
  //     action<OutputMember> output_action (output_handle,
  // 					  ptr_to_member (output_handle, output_member_ptr));
  //     action<InputMember, Callback> input_action (input_handle,
  // 						  ptr_to_member (input_handle, input_member_ptr),
  // 						  handle,
  // 						  callback);

  //     return new compose<action<OutputMember>, action<InputMember, Callback>, Callback> (scheduler, output_action, input_action, callback);
  //   }

  //   template <class Instance, class Member>
  //   void schedule_free_input (const automaton_handle<Instance>& handle,
  // 			      Member Instance::*member_ptr,
  // 			      const typename Member::value_type& t) {
  //     m_runq.push_back (make_execute_free_input (*this,
  // 						 handle,
  // 						 member_ptr,
  // 						 t));
  //   }

  //   template <class Instance, class Callback>
  //   void schedule_callback (const automaton_handle<Instance>& handle,
  // 			    Callback& callback) {
  //     m_runq.push_back (make_execute_callback (*this,
  // 					       handle,
  // 					       callback));
  //   }

  //   template <class Instance, class Member>
  //   void schedule_internal (const automaton_handle<Instance>& handle,
  // 			    Member Instance::*member_ptr) {
  //     m_runq.push_back (make_execute_internal (*this,
  // 					       handle,
  // 					       member_ptr));
  //   }

  //   template <class Instance>
  //   automaton_handle<Instance> get_current_handle () {
  //     typed_automaton<Instance>* current_automaton = static_cast<typed_automaton<Instance>* > (m_current_automaton);
  //   return automaton_handle<Instance> (current_automaton);
  //   }

  //   std::list<runnable*> m_runq;
  //   system m_system;
  //   automaton* m_current_automaton;

  //   void dump_runq () {
  //     std::cout << "runq (" << m_runq.size () << "):" << std::endl;
  //     BOOST_FOREACH (const runnable* r, m_runq) {
  // 	std::cout << "\t" << *r << std::endl;
  //     }
  //   }

  // public:
  //   void set_current_automaton (automaton* a) {
  //     m_current_automaton = a;
  //   }

  //   template <class Instance>
  //   void run (Instance* instance) {
  //     // Pretend the root is running.
  //     automaton_handle<ioa::root_automaton> handle = m_system.root_handle;
  //     set_current_automaton (handle.get_automaton ());

  //     root_callback<Instance> cb;
  //     schedule_create<root_automaton, Instance, root_callback<Instance> > (instance, cb);

  //     while (!m_runq.empty ()) {
  //     	// dump_runq ();
  //     	runnable* r = m_runq.front ();
  //     	m_runq.pop_front ();
  //     	(*r) ();
  //     	delete r;
  //     }
  //   }

  //   template <class Instance, class Member>
  //   void schedule_output (Member Instance::*member_ptr) {
  //     m_runq.push_back (make_execute_output (*this,
  // 					     get_current_handle<Instance> (),
  // 					     member_ptr,
  // 					     typename Member::value_status ()));
  //   }
    
  //   template <class Instance, class Member>
  //   void schedule_internal (Member Instance::*member_ptr) {
  //     m_runq.push_back (make_execute_internal (*this,
  // 					       get_current_handle<Instance> (),
  // 					       member_ptr));
  //   }

  //   template <class Parent, class Child, class Callback>
  //   void schedule_create (Child* child, Callback& callback) {
  //     create_callback1<Parent, Child, Callback> cb (*this,
  // 						    get_current_handle<Parent> (),
  // 						    callback);
  //     m_runq.push_back (make_create (*this,
  //     				     get_current_handle<Parent> (),
  //     				     child,
  //     				     cb));
  //   }

  //   template <class Instance,
  // 	      class OutputInstance, class OutputMember,
  // 	      class InputInstance, class InputMember,
  // 	      class Callback>
  //   void schedule_compose (const automaton_handle<OutputInstance>& output_handle,
  // 			   OutputMember OutputInstance::*output_member_ptr,
  // 			   const automaton_handle<InputInstance>& input_handle,
  // 			   InputMember InputInstance::*input_member_ptr,
  // 			   Callback& callback) {
  //     compose_callback1<Instance,
  // 			OutputInstance,
  // 			OutputMember,
  // 			InputInstance,
  // 			InputMember,
  // 			Callback> cb (*this,
  // 				      get_current_handle<Instance> (),
  // 				      output_handle,
  // 				      output_member_ptr,
  // 				      input_handle,
  // 				      input_member_ptr,
  // 				      callback);
  //     m_runq.push_back (make_compose (*this,
  // 				      get_current_handle<Instance> (),
  // 				      output_handle,
  // 				      output_member_ptr,
  // 				      input_handle,
  // 				      input_member_ptr,
  // 				      cb));
  //   }
  // };


  // simple_scheduler scheduler;

  // scheduler_interface& s = scheduler;

  // scheduler_interface& get_scheduler () {
  //   return s;
  // }

}

#endif
