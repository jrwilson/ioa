#ifndef __ioa_hpp__
#define __ioa_hpp__

#include <memory>
#include <set>
#include <queue>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include "automaton.hpp"
#include "composition_manager.hpp"

namespace ioa {

  template <class C, class T, void (C::*member)(const T)>
  class input_wrapper {
  private:
    C& m_c;
    
  public:
    typedef T type;

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
  class void_input_wrapper {
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
  class output_wrapper {
  private:
    C& m_c;
    
  public:
    typedef typed type_status;
    typedef T type;

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
  class void_output_wrapper {
  private:
    C& m_c;
    
  public:
    typedef untyped type_status;

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
  class internal_wrapper {
  private:
    C& m_c;
    
  public:
    internal_wrapper (C& c)
      : m_c (c) { }
    
    void operator() () {
      (m_c.*member) ();
    }
  };

  typedef enum {
    CHILD_CREATED,
    AUTOMATON_EXISTS
  } create_result_type;

  template <class Instance>
  struct create_result {
    create_result_type type;
    automaton_handle<Instance> handle;
    create_result (const create_result_type type,
		   const automaton_handle<Instance>& handle) :
      type (type),
      handle (handle)
    { }
  };

  template <class Instance>
  create_result<Instance> make_create_result (const create_result_type type,
					      const automaton_handle<Instance>& handle) {
    return create_result<Instance> (type, handle);
  }
  
  typedef enum {
    OUTPUT_INVALID,
    INPUT_INVALID,
    COMPOSITION_EXISTS,
    INPUT_UNAVAILABLE,
    OUTPUT_UNAVAILABLE,
    COMPOSED
  } compose_result_type;

  struct compose_result {
    compose_result_type type;
    compose_result (const compose_result_type type) :
      type (type)
    { }
  };
  
  compose_result make_compose_result (const compose_result_type type) {
    return compose_result (type);
  }
  
  struct root_automaton {
  };

  template <class Instance>
  struct root_callback {
    void operator() (const create_result<Instance>& r) {
      switch (r.type) {
      case CHILD_CREATED:
	// Good.
	break;
      case AUTOMATON_EXISTS:
	// Bad.
	abort ();
	break;
      }
    }
  };

  class system : public boost::shared_mutex {
  private:
    std::set<abstract_automaton*> m_automata;
    composition_manager m_composition_manager;

    struct automaton_instance_equal {
      void* m_instance;
      automaton_instance_equal(void* instance)
	: m_instance(instance) { }

      bool operator()(const abstract_automaton* aa) const {
	return m_instance == aa->get_instance();
      }
    };

  public:
    const automaton_handle<root_automaton> root_handle;

    system()
      : root_handle(new automaton<root_automaton>()) { }

    ~system() {
      BOOST_FOREACH(abstract_automaton* aa, m_automata) {
	delete aa;
      }
    }

    // Modifying methods.
    template<class Parent, class Child, class Callback>
    void create (const automaton_handle<Parent>& parent,
		 Child* child,
		 Callback& callback) {
      boost::unique_lock<boost::shared_mutex> lock(*this);
      if (parent.valid ()) {
	std::set<abstract_automaton*>::const_iterator pos =
	  std::find_if (m_automata.begin (),
			m_automata.end (),
			automaton_instance_equal (child));
	if(pos == m_automata.end ()) {
	  automaton<Child>* c = new automaton<Child> (child);
	  m_automata.insert (c);
	  const automaton_handle<Child> h (c);
	  callback (make_create_result (CHILD_CREATED, h));
	}
	else {
	  const automaton_handle<Child> h;
	  callback (make_create_result (AUTOMATON_EXISTS, h));
	}
      }
      else {
	delete child;
      }
    }

    // template<class Callback, class Instance>
    // void declare(Callback& callback,
    // 		 const automaton_handle<Instance>& handle,
    // 		 const void* parameter) {
    //   boost::unique_lock<boost::shared_mutex> lock(*this);
    //   if (handle.valid()) {
    // 	automaton<Instance>* automaton = handle.get_automaton();
    // 	if(!automaton->is_declared(parameter)) {
    // 	  automaton->declare(parameter);
    // 	  callback.parameter_declared(handle, parameter);
    // 	}
    // 	else {
    // 	  callback.parameter_exists(handle, parameter);
    // 	}
    //   }
    // }

    template<class Instance,
	     class OutputInstance, class OutputAction,
	     class InputInstance, class InputAction,
	     class Callback,
	     class Scheduler>
    void compose (const automaton_handle<Instance>& handle,
		  const automaton_handle<OutputInstance>& output_handle,
		  const OutputAction& output_action,
		  const automaton_handle<InputInstance>& input_handle,
		  const InputAction& input_action,
		  Callback& callback,
		  Scheduler& scheduler) {
      // Lock the system.
      boost::unique_lock<boost::shared_mutex> lock (*this);
      if (handle.valid ()) {
      	if (!output_handle.valid ()) {
      	  callback (make_compose_result (OUTPUT_INVALID));
      	}
      	else if(!input_handle.valid ()) {
      	  callback (make_compose_result (INPUT_INVALID));
      	}
      	else {
	  BOOST_ASSERT (handle.get_automaton () == input_action.get_owner ());
	  BOOST_ASSERT (output_handle.get_automaton () == output_action.get_automaton ());
	  BOOST_ASSERT (input_handle.get_automaton () == input_action.get_automaton ());

      	  if (m_composition_manager.composed (output_action, input_action)) {
      	    callback (make_compose_result (COMPOSITION_EXISTS));
      	  }
      	  else if (!m_composition_manager.input_available (input_action)) {
      	    callback (make_compose_result (INPUT_UNAVAILABLE));
      	  }
      	  else if(!m_composition_manager.output_available (output_action, input_action.get_automaton ())) {
      	    callback (make_compose_result (OUTPUT_UNAVAILABLE));
      	  }
      	  else {
      	    m_composition_manager.compose (output_action, input_action, scheduler);
      	    callback (make_compose_result (COMPOSED));
      	  }
      	}
      }
    }

    // template<class Callback, class Instance,
    // 	     class OutputInstance, class OutputMember,
    // 	     class InputInstance, class InputMember>
    // void decompose(Callback& callback,
    // 		   const automaton_handle<Instance>& handle,
    // 		   const automaton_handle<OutputInstance>& output_handle,
    // 		   OutputMember OutputInstance::*output_member,
    // 		   const automaton_handle<InputInstance>& input_handle,
    // 		   const InputMember InputInstance::*input_member) {
    //   // Lock the system.
    //   boost::unique_lock<boost::shared_mutex> lock(*this);
    //   if(handle.valid()) {
    // 	automaton<Instance>* owner = handle.get_instance();
    // 	automaton<OutputInstance>* output_automaton = output_handle.get_instance();
    // 	automaton<InputInstance>* input_automaton = input_handle.get_instance();
    // 	if(!m_composition_manager.composed(output_automaton, output_member, input_automaton, input_member)) {
    // 	  if(m_composition_manager.composed(output_automaton, output_member, input_automaton, input_member)) {
    // 	    // Decompose.
    // 	    BOOST_ASSERT(false);
    // 	  }
    // 	  else {
    // 	    callback.not_owner();
    // 	  }
    // 	}
    // 	else {
    // 	  callback.not_composed();
    // 	}
    //   }      
    // }

    // void rescind();
    // void destroy();

    // Non-modifying methods.
    template <class Instance, class Scheduler>
    void execute_local (const automaton_handle<Instance>& handle,
			local_action_interface& action,
			Scheduler& scheduler) {
      // Lock the system so automata can't disappear.
      boost::shared_lock<boost::shared_mutex> lock (*this);
      if (handle.valid ()) {
	BOOST_ASSERT (handle.get_automaton () == action.get_automaton ());
	m_composition_manager.execute (action, scheduler);
      }
    }

  };

  class simple_scheduler;
  
  class runnable {
  protected:
    simple_scheduler& m_scheduler;
  public:
    runnable (simple_scheduler& scheduler)
      : m_scheduler (scheduler)
    { }
    virtual ~runnable() { }
    virtual void operator() () = 0;
    virtual void print_on (std::ostream& output) const = 0;
  };
  
  static std::ostream& operator<<(std::ostream& output, const runnable& runnable) {
    runnable.print_on (output);
    return output;
  }
   
  class simple_scheduler {
  private:

 
    template<class Instance, class Action>
    class execute_local : public runnable {
    private:
      automaton_handle<Instance> m_handle;
      Action m_action;
      
    public:
      execute_local (simple_scheduler& scheduler,
		      const automaton_handle<Instance>& handle,
		      const Action& action)
	: runnable (scheduler),
	  m_handle (handle),
	  m_action (action) { }
      
      void operator() () {
	m_scheduler.m_system.execute_local (m_handle, m_action, m_scheduler);
      }

      void print_on (std::ostream& output) const {
	output << "action " << m_action;
      }
    };

    template <class Instance, class Callback>
    class create_parent_callback {
    private:
      create_result<Instance> m_result;
      Callback m_callback;
    public:
      create_parent_callback (const create_result<Instance>& result,
			      Callback& callback) :
	m_result (result),
	m_callback (callback)
      { }

      void operator() () {
	m_callback (m_result);
      }
    };

    template <class Parent, class Child, class Callback>
    class create_callback1 {

    private:
      simple_scheduler& m_scheduler;
      automaton_handle<Parent> m_parent_handle;
      Callback m_callback;

    public:
      create_callback1 (simple_scheduler& scheduler,
			const automaton_handle<Parent>& parent_handle,
			Callback& callback) :
	m_scheduler (scheduler),
	m_parent_handle (parent_handle),
	m_callback (callback) { }

      void operator() (const create_result<Child>& r) {
	switch (r.type) {
	case CHILD_CREATED:
	  // Start the child.
	  m_scheduler.schedule_free_input (r.handle, &Child::init, r.handle);
	  break;
	case AUTOMATON_EXISTS:
	  break;
	}

	create_parent_callback<Child, Callback> cb (r, m_callback);
	m_scheduler.schedule_callback<Parent, create_parent_callback<Child, Callback> > (m_parent_handle, cb);
      }

    };

    template<class Parent, class Child, class Callback>
    class create : public runnable {
    private:
      automaton_handle<Parent> m_parent_handle;
      Child* m_child;
      Callback m_callback;

    public:
      create (simple_scheduler& scheduler,
	      const ioa::automaton_handle<Parent>& parent_handle,
	      Child* child,
	      const Callback& callback)
	: runnable (scheduler),
	  m_parent_handle (parent_handle),
	  m_child (child),
	  m_callback (callback)
      { }

      void operator() () {
	m_scheduler.m_system.create (m_parent_handle, m_child, m_callback);
      }

      void print_on (std::ostream& output) const {
	output << "create parent=" << m_parent_handle.get_automaton ()->get_typed_instance () << " child=" << m_child;
      }
    };

    template <class Callback>
    class compose_owner_callback {
    private:
      compose_result m_result;
      Callback m_callback;
    public:
      compose_owner_callback (const compose_result& result,
			      Callback& callback) :
	m_result (result),
	m_callback (callback)
      { }
      
      void operator() () {
	m_callback (m_result);
      }
    };

    template <class Instance, class Member>
    class compose_action_callback {
    private:
      automaton_handle<Instance> m_handle;
      Member Instance::*m_member_ptr;
      bool m_value;

    public:
      compose_action_callback (const automaton_handle<Instance> handle,
			       Member Instance::*member_ptr,
			       bool value) :
	m_handle (handle),
	m_member_ptr (member_ptr),
	m_value (value)
      { }

      void operator() () {
	Instance* i = m_handle.get_automaton ()->get_typed_instance ();
	Member& ref = (*i).*m_member_ptr;
	ref.composed (m_value);
      }
    };

    template <class Instance, class OutputInstance, class OutputMember, class InputInstance, class InputMember, class Callback>
    class compose_callback1 {

    private:
      simple_scheduler& m_scheduler;
      automaton_handle<Instance> m_handle;
      automaton_handle<OutputInstance> m_output_handle;
      OutputMember OutputInstance::*m_output_member_ptr;
      automaton_handle<InputInstance> m_input_handle;
      InputMember InputInstance::*m_input_member_ptr;
      Callback m_callback;

    public:
      compose_callback1 (simple_scheduler& scheduler,
 			 const automaton_handle<Instance>& handle,
			 const automaton_handle<OutputInstance>& output_handle,
			 OutputMember OutputInstance::*output_member_ptr,
			 const automaton_handle<InputInstance>& input_handle,
			 InputMember InputInstance::*input_member_ptr,
			 Callback& callback) :
      	m_scheduler (scheduler),
      	m_handle (handle),
	m_output_handle (output_handle),
	m_output_member_ptr (output_member_ptr),
	m_input_handle (input_handle),
	m_input_member_ptr (input_member_ptr),
	m_callback (callback)
      { }

      void operator() (const compose_result& r) {
    	switch (r.type) {
	case OUTPUT_INVALID:
	case INPUT_INVALID:
	case COMPOSITION_EXISTS:
	case INPUT_UNAVAILABLE:
	case OUTPUT_UNAVAILABLE:
	  {
	    compose_owner_callback<Callback> owner_cb (r, m_callback);
	    m_scheduler.schedule_callback<Instance, compose_owner_callback<Callback> > (m_handle, owner_cb);
	  }
	  break;
	case COMPOSED:
	  {
	    compose_owner_callback<Callback> owner_cb (r, m_callback);
	    m_scheduler.schedule_callback<Instance, compose_owner_callback<Callback> > (m_handle, owner_cb);

	    compose_action_callback<OutputInstance, OutputMember> output_cb (m_output_handle, m_output_member_ptr, true);
	    m_scheduler.schedule_callback<OutputInstance, compose_action_callback<OutputInstance, OutputMember> > (m_output_handle, output_cb);

	    compose_action_callback<InputInstance, InputMember> input_cb (m_input_handle, m_input_member_ptr, true);
	    m_scheduler.schedule_callback<InputInstance, compose_action_callback<InputInstance, InputMember> > (m_input_handle, input_cb);
	  }
	  break;
    	}

    	// create_parent_callback<Child, Callback> cb (r, m_callback);
    	// m_scheduler.schedule_callback<Parent, create_parent_callback<Child, Callback> > (m_parent_handl
										   // e, cb);
      }

      void decomposed () {
	std::cout << __func__ << std::endl;
      }

    };

    template <class Instance,
	      class OutputInstance, class OutputAction,
	      class InputInstance, class InputAction,
	      class Callback>
    class compose : public runnable {
    private:
      automaton_handle<Instance> m_handle;
      automaton_handle<OutputInstance> m_output_handle;
      OutputAction m_output_action;
      ioa::automaton_handle<InputInstance> m_input_handle;
      InputAction m_input_action;
      Callback m_callback;

    public:
      compose (simple_scheduler& scheduler,
	       const automaton_handle<Instance>& handle,
	       const automaton_handle<OutputInstance>& output_handle,
	       const OutputAction& output_action,
	       const ioa::automaton_handle<InputInstance>& input_handle,
	       const InputAction& input_action,
	       const Callback& callback)
	: runnable (scheduler),
	  m_handle (handle),
	  m_output_handle (output_handle),
	  m_output_action (output_action),
	  m_input_handle (input_handle),
	  m_input_action (input_action),
	  m_callback (callback)
      { }

      void operator() () {
	m_scheduler.m_system.compose<Instance, OutputInstance, OutputAction, InputInstance, InputAction, Callback> (m_handle, m_output_handle, m_output_action, m_input_handle, m_input_action, m_callback, m_scheduler);
      }

      void print_on (std::ostream& output) const {
	output << "compose owner=" << m_handle.get_automaton ()->get_typed_instance ();
      }

    };

    template <class Instance, class Member>
    runnable* make_execute_output (simple_scheduler& scheduler,
				   const automaton_handle<Instance>& handle,
				   Member Instance::*member_ptr,
				   untyped /* */) {
      unparameterized_untyped_output_action<Instance, Member> output_action (handle.get_automaton (), member_ptr);
      return new execute_local<Instance, unparameterized_untyped_output_action<Instance, Member> > (scheduler, handle, output_action);
    }

    template <class Instance, class Member>
    runnable* make_execute_output (simple_scheduler& scheduler,
				   const automaton_handle<Instance>& handle,
				   Member Instance::*member_ptr,
				   typed /* */) {
      unparameterized_typed_output_action<Instance, Member> output_action (handle.get_automaton (), member_ptr);
      return new execute_local<Instance, unparameterized_typed_output_action<Instance, Member> > (scheduler, handle, output_action);
    }

    template <class Instance, class Member>
    runnable* make_execute_internal (simple_scheduler& scheduler,
				     const automaton_handle<Instance>& handle,
				     Member Instance::*member_ptr) {
      unparameterized_internal_action<Instance, Member> action (handle.get_automaton (), member_ptr);
      return new execute_local<Instance, unparameterized_internal_action<Instance, Member> > (scheduler, handle, action);
    }

    template <class Instance, class Member, class T>
    runnable* make_execute_free_input (simple_scheduler& scheduler,
				       const automaton_handle<Instance>& handle,
				       Member Instance::*member_ptr,
				       const T& t) {
      free_input_action<Instance, Member, T> action (handle.get_automaton (), member_ptr, t);
      return new execute_local<Instance, free_input_action<Instance, Member, T> > (scheduler, handle, action);
    }

    template <class Instance, class Callback>
    runnable* make_execute_callback (simple_scheduler& scheduler,
				     const automaton_handle<Instance>& handle,
				     const Callback& callback) {
      callback_action<Instance, Callback> action (handle.get_automaton (), callback);
      return new execute_local<Instance, callback_action<Instance, Callback> > (scheduler, handle, action);
    }

    template <class Parent, class Child, class Callback>
    runnable* make_create (simple_scheduler& scheduler,
			   const automaton_handle<Parent>& handle,
			   Child* child,
			   Callback& callback) {
      return new create<Parent, Child, Callback> (scheduler, handle, child, callback);
    }

    template <class Instance,
	      class OutputInstance, class OutputMember,
	      class InputInstance, class InputMember,
	      class Callback>
    runnable* make_compose (simple_scheduler& scheduler,
			    const automaton_handle<Instance>& handle,
			    const automaton_handle<OutputInstance>& output_handle,
			    OutputMember OutputInstance::*output_member_ptr,
			    const automaton_handle<InputInstance>& input_handle,
			    InputMember InputInstance::*input_member_ptr,
			    untyped /* */,
			    Callback& callback) {
      unparameterized_untyped_output_action<OutputInstance, OutputMember> output_action (output_handle.get_automaton (), output_member_ptr);
      unparameterized_untyped_input_action<InputInstance, InputMember, Callback> input_action (input_handle.get_automaton (), input_member_ptr, handle.get_automaton (), callback);

      return new compose<Instance, OutputInstance, unparameterized_untyped_output_action<OutputInstance, OutputMember>, InputInstance, unparameterized_untyped_input_action<InputInstance, InputMember, Callback>, Callback> (scheduler, handle, output_handle, output_action, input_handle, input_action, callback);
    }

    template <class Instance,
	      class OutputInstance, class OutputMember,
	      class InputInstance, class InputMember,
	      class Callback>
    runnable* make_compose (simple_scheduler& scheduler,
			    const automaton_handle<Instance>& handle,
			    const automaton_handle<OutputInstance>& output_handle,
			    OutputMember OutputInstance::*output_member_ptr,
			    const automaton_handle<InputInstance>& input_handle,
			    InputMember InputInstance::*input_member_ptr,
			    typed /* */,
			    Callback& callback) {
      unparameterized_typed_output_action<OutputInstance, OutputMember> output_action (output_handle.get_automaton (), output_member_ptr);
      unparameterized_typed_input_action<InputInstance, InputMember, Callback> input_action (input_handle.get_automaton (), input_member_ptr, handle.get_automaton (), callback);
      
      return new compose<Instance, OutputInstance, unparameterized_typed_output_action<OutputInstance, OutputMember>, InputInstance, unparameterized_typed_input_action<InputInstance, InputMember, Callback>, Callback> (scheduler, handle, output_handle, output_action, input_handle, input_action, callback);
    }

    template <class Instance, class Member, class T>
    void schedule_free_input (const automaton_handle<Instance>& handle,
			      Member Instance::*member_ptr,
			      const T& t) {
      m_runq.push_back (make_execute_free_input (*this, handle, member_ptr, t));
    }

    template <class Instance, class Callback>
    void schedule_callback (const automaton_handle<Instance>& handle,
			    Callback& callback) {
      m_runq.push_back (make_execute_callback (*this, handle, callback));
    }

    template <class Instance>
    automaton_handle<Instance> get_current_handle () {
      automaton<Instance>* current_automaton = static_cast<automaton<Instance>* > (m_current_automaton);
    return automaton_handle<Instance> (current_automaton);
    }

    std::list<runnable*> m_runq;
    system m_system;
    abstract_automaton* m_current_automaton;

    void dump_runq () {
      std::cout << "runq (" << m_runq.size () << "):" << std::endl;
      BOOST_FOREACH (const runnable* r, m_runq) {
	std::cout << "\t" << *r << std::endl;
      }
    }

  public:
    void set_current_automaton (abstract_automaton* a) {
      m_current_automaton = a;
    }

    template <class Instance>
    void run (Instance* instance) {
      // Pretend the root is running.
      automaton_handle<ioa::root_automaton> handle = m_system.root_handle;
      set_current_automaton (handle.get_automaton ());

      root_callback<Instance> cb;
      schedule_create<root_automaton, Instance, root_callback<Instance> > (instance, cb);

      while (!m_runq.empty ()) {
	// dump_runq ();
	runnable* r = m_runq.front ();
	m_runq.pop_front ();
	(*r) ();
	delete r;
      }
    }

    template <class Instance, class Member>
    void schedule_output (Member Instance::*member_ptr) {
      m_runq.push_back (make_execute_output (*this,
					     get_current_handle<Instance> (),
					     member_ptr,
					     typename Member::type_status ()));
    }
    
    template <class Instance, class Member>
    void schedule_internal (Member Instance::*member_ptr) {
      m_runq.push_back (make_execute_internal (*this,
					       get_current_handle<Instance> (),
					       member_ptr));
    }

    template <class Parent, class Child, class Callback>
    void schedule_create (Child* child, Callback& callback) {
      create_callback1<Parent, Child, Callback> cb (*this,
						    get_current_handle<Parent> (),
						    callback);
      m_runq.push_back (make_create (*this,
				     get_current_handle<Parent> (),
				     child,
				     cb));
    }

    template <class Instance,
	      class OutputInstance, class OutputMember,
	      class InputInstance, class InputMember,
	      class Callback>
    void schedule_compose (const automaton_handle<OutputInstance>& output_handle,
			   OutputMember OutputInstance::*output_member_ptr,
			   const automaton_handle<InputInstance>& input_handle,
			   InputMember InputInstance::*input_member_ptr,
			   Callback& callback) {
      compose_callback1<Instance,
			OutputInstance,
			OutputMember,
			InputInstance,
			InputMember,
			Callback> cb (*this,
				      get_current_handle<Instance> (),
				      output_handle,
				      output_member_ptr,
				      input_handle,
				      input_member_ptr,
				      callback);
      m_runq.push_back (make_compose (*this,
				      get_current_handle<Instance> (),
				      output_handle,
				      output_member_ptr,
				      input_handle,
				      input_member_ptr,
				      typename OutputMember::type_status (),
				      cb));
    }
  };


  simple_scheduler scheduler;
}

#endif
