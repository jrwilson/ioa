#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>

#include <system.hpp>
#include "automaton1.hpp"

struct dummy_create_listener
{
  bool m_creator_dne;
  bool m_created;
  bool m_instance_exists;
  ioa::generic_automaton_handle m_automaton;

  void instance_exists (const ioa::automaton_interface*) {
    m_instance_exists = true;
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    m_automaton = automaton;
    m_created = true;
  }

  void automaton_dne (const ioa::generic_automaton_handle&,
		      ioa::automaton_interface*) {
    m_creator_dne = true;
  }

  void instance_exists (const ioa::generic_automaton_handle& automaton,
			const ioa::automaton_interface*) {
    m_instance_exists = true;
  }
  
  void automaton_created (const ioa::generic_automaton_handle& parent,
			  const ioa::generic_automaton_handle& automaton) {
    m_automaton = automaton;
    m_created = true;
  }
  
  dummy_create_listener () :
    m_creator_dne (false),
    m_created (false),
    m_instance_exists (false)
  { }
};

struct dummy_declare_listener
{
  bool m_automaton_dne;
  bool m_declared;
  bool m_parameter_exists;
  ioa::generic_parameter_handle m_parameter;

  void automaton_dne (const ioa::generic_automaton_handle&,
		      void*) {
    m_automaton_dne = true;
  }

  void parameter_exists (const ioa::generic_automaton_handle&,
			 const ioa::generic_parameter_handle&) {
    m_parameter_exists = true;
  }

  void parameter_declared (const ioa::generic_automaton_handle&,
			   const ioa::generic_parameter_handle& parameter) {
    m_declared = true;
    m_parameter = parameter;
  }

  dummy_declare_listener () :
    m_automaton_dne (false),
    m_declared (false),
    m_parameter_exists (true)
  { }
};

struct dummy_bind_listener
{
  bool m_binder_automaton_dne;
  bool m_output_automaton_dne;
  bool m_input_automaton_dne;
  bool m_output_parameter_dne;
  bool m_input_parameter_dne;
  bool m_binding_exists;
  bool m_input_action_unavailable;
  bool m_output_action_unavailable;
  bool m_bound;

  template <class OM, class IM>
  void automaton_dne (const ioa::action<OM>&,
		      const ioa::action<IM>&,
		      const ioa::generic_automaton_handle&) {
    m_binder_automaton_dne = true;
  }
  
  template <class OM, class IM>
  void output_automaton_dne (const ioa::action<OM>&,
			     const ioa::action<IM>&,
			     const ioa::generic_automaton_handle&) {
    m_output_automaton_dne = true;
  }
  
  template <class OM, class IM>
  void input_automaton_dne (const ioa::action<OM>&,
			    const ioa::action<IM>&,
			    const ioa::generic_automaton_handle&) {
    m_input_automaton_dne = true;
  }
  
  template <class OM, class IM>
  void output_parameter_dne (const ioa::action<OM>&,
			     const ioa::action<IM>&,
			     const ioa::generic_automaton_handle&) {
    m_output_parameter_dne = true;
  }
  
  template <class OM, class IM>
  void input_parameter_dne (const ioa::action<OM>&,
			    const ioa::action<IM>&,
			    const ioa::generic_automaton_handle&) {
    m_input_parameter_dne = true;
  }
  
  template <class OM, class IM>
  void binding_exists (const ioa::action<OM>&,
		       const ioa::action<IM>&,
		       const ioa::generic_automaton_handle&) {
    m_binding_exists = true;
  }
  
  template <class OM, class IM>
  void input_action_unavailable (const ioa::action<OM>&,
				 const ioa::action<IM>&,
				 const ioa::generic_automaton_handle&) {
    m_input_action_unavailable= true;
  }
  
  template <class OM, class IM>
  void output_action_unavailable (const ioa::action<OM>&,
				  const ioa::action<IM>&,
				  const ioa::generic_automaton_handle&) {
    m_output_action_unavailable = true;
  }
  
  template <class OM, class IM>
  void bound (const ioa::action<OM>&,
	      const ioa::action<IM>&,
	      const ioa::generic_automaton_handle&) {
    m_bound = true;
  }

  dummy_bind_listener () :
    m_binder_automaton_dne (false),
    m_output_automaton_dne (false),
    m_input_automaton_dne (false),
    m_output_parameter_dne (false),
    m_input_parameter_dne (false),
    m_binding_exists (false),
    m_input_action_unavailable (false),
    m_output_action_unavailable (false),
    m_bound (false)
  { }
};

struct dummy_unbind_success_listener
{
  bool m_unbound;

  template <class OM, class IM>
  void unbound (const ioa::action<OM>&,
		const ioa::action<IM>&,
		const ioa::generic_automaton_handle&) {
    m_unbound = true;
  }

  dummy_unbind_success_listener () :
    m_unbound (false)
  { }
};

struct dummy_unbind_failure_listener
{
  bool m_binder_automaton_dne;
  bool m_output_automaton_dne;
  bool m_input_automaton_dne;
  bool m_output_parameter_dne;
  bool m_input_parameter_dne;
  bool m_binding_dne;
  bool m_any;

  template <class OM, class IM>
  void automaton_dne (const ioa::action<OM>&,
		      const ioa::action<IM>&,
		      const ioa::generic_automaton_handle&) {
    m_binder_automaton_dne = true;
    m_any = true;
  }
  
  template <class OM, class IM>
  void output_automaton_dne (const ioa::action<OM>&,
			     const ioa::action<IM>&,
			     const ioa::generic_automaton_handle&) {
    m_output_automaton_dne = true;
    m_any = true;
  }
  
  template <class OM, class IM>
  void input_automaton_dne (const ioa::action<OM>&,
			    const ioa::action<IM>&,
			    const ioa::generic_automaton_handle&) {
    m_input_automaton_dne = true;
    m_any = true;
  }
  
  template <class OM, class IM>
  void output_parameter_dne (const ioa::action<OM>&,
			     const ioa::action<IM>&,
			     const ioa::generic_automaton_handle&) {
    m_output_parameter_dne = true;
    m_any = true;
  }
  
  template <class OM, class IM>
  void input_parameter_dne (const ioa::action<OM>&,
			    const ioa::action<IM>&,
			    const ioa::generic_automaton_handle&) {
    m_input_parameter_dne = true;
    m_any = true;
  }
  
  template <class OM, class IM>
  void binding_dne (const ioa::action<OM>&,
		    const ioa::action<IM>&,
		    const ioa::generic_automaton_handle&) {
    m_binding_dne = true;
    m_any = true;
  }

  dummy_unbind_failure_listener () :
    m_binder_automaton_dne (false),
    m_output_automaton_dne (false),
    m_input_automaton_dne (false),
    m_output_parameter_dne (false),
    m_input_parameter_dne (false),
    m_binding_dne (false),
    m_any (false)
  { }
};

struct dummy_rescind_success_listener
{
  bool m_parameter_rescinded;

  void parameter_rescinded (const ioa::generic_automaton_handle&,
  			    const ioa::generic_parameter_handle&) {
    m_parameter_rescinded = true;
  }

  dummy_rescind_success_listener () :
    m_parameter_rescinded (false)
  { }

};

struct dummy_rescind_failure_listener
{
  bool m_automaton_dne;
  bool m_parameter_dne;
  bool m_any;

  void automaton_dne (const ioa::generic_automaton_handle& automaton,
		      const ioa::generic_parameter_handle& parameter) {
    m_automaton_dne = true;
    m_any = true;
  }

  void parameter_dne (const ioa::generic_automaton_handle& automaton,
		      const ioa::generic_parameter_handle& parameter) {
    m_parameter_dne = true;
    m_any = true;
  }

  dummy_rescind_failure_listener () :
    m_automaton_dne (false),
    m_parameter_dne (false),
    m_any (false)
  { }
};

struct dummy_destroy_success_listener
{
  bool m_automaton_destroyed;

  void automaton_destroyed (const ioa::generic_automaton_handle&) {
    m_automaton_destroyed = true;
  }

  void automaton_destroyed (const ioa::generic_automaton_handle&,
  			    const ioa::generic_automaton_handle&) {
    m_automaton_destroyed = true;
  }

  dummy_destroy_success_listener () :
    m_automaton_destroyed (false)
  { }
};

struct dummy_destroy_failure_listener
{
  bool m_destroyer_dne;
  bool m_destroyer_not_creator;
  bool m_automaton_dne;
  bool m_any;

  void target_automaton_dne (const ioa::generic_automaton_handle&) {
    m_any = true;
    m_automaton_dne = true;
  }

  void automaton_dne (const ioa::generic_automaton_handle&,
		      const ioa::generic_automaton_handle&) {
    m_destroyer_dne = true;
    m_any = true;
  }

  void target_automaton_dne (const ioa::generic_automaton_handle&,
			     const ioa::generic_automaton_handle&) {
    m_any = true;
    m_automaton_dne = true;
  }

  void destroyer_not_creator (const ioa::generic_automaton_handle&,
			      const ioa::generic_automaton_handle&) {
    m_any = true;
    m_destroyer_not_creator = true;
  }

  dummy_destroy_failure_listener () :
    m_destroyer_dne (false),
    m_destroyer_not_creator (false),
    m_automaton_dne (false),
    m_any (false)
  { }
};

template <class T>
ioa::automaton_handle<T> create (ioa::system& system,
				 T* instance,
				 dummy_destroy_success_listener& dsl) {
  dummy_create_listener listener;
  system.create (instance, listener, dsl);
  BOOST_CHECK (listener.m_created);
  BOOST_CHECK_EQUAL (listener.m_automaton.value (), instance);
  return ioa::cast_automaton<T> (listener.m_automaton);
}

template <class T>
ioa::automaton_handle<T> create (ioa::system& system,
				 const ioa::generic_automaton_handle& creator,
				 T* instance,
				 dummy_destroy_success_listener& dsl) {
  dummy_create_listener listener;
  system.create (creator, instance, listener, dsl);
  BOOST_CHECK (listener.m_created);
  BOOST_CHECK_EQUAL (listener.m_automaton.value (), instance);
  return ioa::cast_automaton<T> (listener.m_automaton);
}

template <class T>
ioa::parameter_handle<T> declare (ioa::system& system,
				  const ioa::generic_automaton_handle& automaton,
				  T* parameter,
				  dummy_rescind_success_listener& rsl) {
  dummy_declare_listener listener;
  system.declare (automaton, parameter, listener, rsl);
  BOOST_CHECK (listener.m_declared);
  BOOST_CHECK_EQUAL (listener.m_parameter.value (), parameter);
  return ioa::cast_parameter<T> (listener.m_parameter);
}

template <class OM, class IM>
void bind (ioa::system& system,
	   const ioa::action<OM>& output_action,
	   const ioa::action<IM>& input_action,
	   const ioa::generic_automaton_handle& binder,
	   dummy_unbind_success_listener& usl) {
  dummy_bind_listener listener;
  system.bind (output_action, input_action, binder, listener, usl);
  BOOST_CHECK (listener.m_bound);
}

template <class OM, class IM>
void bind (ioa::system& system,
	   const ioa::action<OM>& output_action,
	   const ioa::action<IM>& input_action,
	   dummy_unbind_success_listener& usl) {
  dummy_bind_listener listener;
  system.bind (output_action, input_action, listener, usl);
  BOOST_CHECK (listener.m_bound);
}

void rescind (ioa::system& system,
	      const ioa::generic_automaton_handle& automaton,
	      const ioa::generic_parameter_handle& parameter) {
  dummy_rescind_failure_listener rfl;
  system.rescind (automaton, parameter, rfl);
  BOOST_CHECK (!rfl.m_any);
}

void destroy (ioa::system& system,
	      const ioa::generic_automaton_handle& automaton)
{
  dummy_destroy_failure_listener listener;
  system.destroy (automaton, listener);
  BOOST_CHECK (!listener.m_any);
}

class dummy_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_handle (const ioa::generic_automaton_handle&) { }
  void clear_current_handle (void) { }
};

struct dummy_execute_listener
{
  bool m_automaton_dne;
  bool m_parameter_dne;

  void automaton_dne () { m_automaton_dne = true; }
  void parameter_dne () { m_parameter_dne = true; }

  dummy_execute_listener () :
    m_automaton_dne (false),
    m_parameter_dne (false)
  { }
};

BOOST_AUTO_TEST_SUITE(system_suite)

BOOST_AUTO_TEST_CASE(ctor)
{
  ioa::automaton_interface* x = new ioa::automaton_interface ();
  ioa::generic_automaton_handle handle (x);
  ioa::automaton_record_interface automaton (handle);
  BOOST_CHECK_EQUAL (x, automaton.get_instance ());
}

BOOST_AUTO_TEST_CASE(declare_parameter)
{
  int parameter;
  ioa::generic_automaton_handle handle (new ioa::automaton_interface ());
  ioa::automaton_record_interface automaton (handle);

  BOOST_CHECK (!automaton.parameter_exists (&parameter));
  dummy_rescind_success_listener rsl;
  automaton.declare_parameter (&parameter, rsl);
  BOOST_CHECK (automaton.parameter_exists (&parameter));
}

BOOST_AUTO_TEST_CASE (system_create_creator_dne)
{
  ioa::system system;
  dummy_create_listener create_listener1;
  dummy_destroy_success_listener dsl1;
  automaton1* instance = new automaton1 ();
  system.create (instance, create_listener1, dsl1);
  BOOST_CHECK (create_listener1.m_created);
  ioa::automaton_handle<automaton1> creator_handle = ioa::cast_automaton<automaton1> (create_listener1.m_automaton);
  destroy (system, creator_handle);
  dummy_create_listener create_listener2;
  dummy_destroy_success_listener dsl2;
  system.create (creator_handle, new automaton1 (), create_listener2, dsl2);
  BOOST_CHECK (create_listener2.m_creator_dne);
}

BOOST_AUTO_TEST_CASE (system_create_exists)
{
  ioa::system system;
  automaton1* a = new automaton1 ();
  dummy_create_listener create_listener1;
  dummy_destroy_success_listener dsl1;
  system.create (a, create_listener1, dsl1);
  BOOST_CHECK (create_listener1.m_created);
  dummy_create_listener create_listener2;
  dummy_destroy_success_listener dsl2;
  system.create (a, create_listener2, dsl2);
  BOOST_CHECK (create_listener2.m_instance_exists);
}

BOOST_AUTO_TEST_CASE (system_create_success)
{
  ioa::system system;
  automaton1* a = new automaton1 ();
  dummy_destroy_success_listener dsl;
  dummy_create_listener create_listener;
  system.create (a, create_listener, dsl);
  BOOST_CHECK (create_listener.m_created);
}

BOOST_AUTO_TEST_CASE (system_declare_automaton_dne)
{
  ioa::system system;
  int parameter;
  
  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, new automaton1 (), dsl);
  destroy (system, handle);
  dummy_declare_listener declare_listener;
  dummy_rescind_success_listener rsl;
  system.declare (handle, &parameter, declare_listener, rsl);
  BOOST_CHECK (declare_listener.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_declare_exists)
{
  ioa::system system;
  int parameter;
  
  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, new automaton1 (), dsl);
  dummy_declare_listener declare_listener1;
  dummy_rescind_success_listener rsl1;
  system.declare (handle, &parameter, declare_listener1, rsl1);
  BOOST_CHECK (declare_listener1.m_declared);
  dummy_declare_listener declare_listener2;
  dummy_rescind_success_listener rsl2;
  system.declare (handle, &parameter, declare_listener2, rsl2);
  BOOST_CHECK (declare_listener2.m_parameter_exists);
}

BOOST_AUTO_TEST_CASE (system_declare_success)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, new automaton1 (), dsl);
  dummy_declare_listener declare_listener;
  dummy_rescind_success_listener rsl;
  system.declare (handle, &parameter, declare_listener, rsl);
  BOOST_CHECK (declare_listener.m_declared);
  BOOST_CHECK_EQUAL (declare_listener.m_parameter.value (), &parameter);
}

BOOST_AUTO_TEST_CASE (system_bind_binder_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  destroy (system, binder);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener, usl);

  BOOST_CHECK (bind_listener.m_binder_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_output_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);
  destroy (system, output);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);
  
  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener, usl);
  BOOST_CHECK (bind_listener.m_output_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_input_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  destroy (system, input);
  
  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener, usl);
  BOOST_CHECK (bind_listener.m_input_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  rescind (system, output, param);

  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       bind_listener, usl);
  BOOST_CHECK (bind_listener.m_output_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_input_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, input, &parameter, rsl);

  rescind (system, input, param);

  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::p_uv_input, param),
	       bind_listener, usl);
  BOOST_CHECK (bind_listener.m_input_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_exists)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);
  
  dummy_bind_listener bind_listener1;
  dummy_unbind_success_listener usl1;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1, usl1);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  dummy_unbind_success_listener usl2;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener2, usl2);
  BOOST_CHECK (bind_listener2.m_binding_exists);
}

BOOST_AUTO_TEST_CASE (system_bind_input_action_unavailable)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output1_instance = new automaton1 ();
  automaton1* output2_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output1_dsl;
  ioa::automaton_handle<automaton1> output1 = create (system, output1_instance, output1_dsl);

  dummy_destroy_success_listener output2_dsl;
  ioa::automaton_handle<automaton1> output2 = create (system, output2_instance, output2_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_bind_listener bind_listener1;
  dummy_unbind_success_listener usl1;
  system.bind (ioa::make_action (output1, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1, usl1);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  dummy_unbind_success_listener usl2;
  system.bind (ioa::make_action (output2, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener2,usl2);
  BOOST_CHECK (bind_listener2.m_input_action_unavailable);
}

BOOST_AUTO_TEST_CASE (system_bind_output_action_unavailable)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, input, &parameter, rsl);

  dummy_bind_listener bind_listener1;
  dummy_unbind_success_listener usl1;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1, usl1);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  dummy_unbind_success_listener usl2;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::p_uv_input, param),
	       bind_listener2, usl2);
  BOOST_CHECK (bind_listener2.m_output_action_unavailable);
}

BOOST_AUTO_TEST_CASE (system_bind_success)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       bind_listener, usl);
  BOOST_CHECK (bind_listener.m_bound);

  dummy_scheduler scheduler;
  dummy_execute_listener listener;
  system.execute (ioa::make_action (output, &automaton1::p_uv_output, param), scheduler, listener);
  BOOST_CHECK (input_instance->up_uv_input.state);
}

BOOST_AUTO_TEST_CASE (system_unbind_binder_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);
  
  dummy_unbind_success_listener usl;
  bind (system,
	ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder,
	usl);

  destroy (system, binder);

  dummy_unbind_failure_listener unbind_listener;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 binder,
		 unbind_listener);
  BOOST_CHECK (unbind_listener.m_binder_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_output_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);
  
  dummy_unbind_success_listener usl;
  bind (system, ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder,
	usl);

  destroy (system, output);

  dummy_unbind_failure_listener unbind_listener;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output), 
		 ioa::make_action (input, &automaton1::up_uv_input),
		 binder,
		 unbind_listener);
  BOOST_CHECK (unbind_listener.m_output_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_input_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);
  
  dummy_unbind_success_listener usl;
  bind (system, ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder,
	usl);

  destroy (system, input);

  dummy_unbind_failure_listener unbind_listener;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 binder,
		 unbind_listener);
  BOOST_CHECK (unbind_listener.m_input_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  rescind (system, output, param);

  dummy_unbind_failure_listener unbind_listener;
  system.unbind (ioa::make_action (output, &automaton1::p_uv_output, param),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 unbind_listener);
  BOOST_CHECK (unbind_listener.m_output_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_input_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, input, &parameter, rsl);

  rescind (system, input, param);

  dummy_unbind_failure_listener unbind_listener;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output), 
		 ioa::make_action (input, &automaton1::p_uv_input, param),
		 unbind_listener);
  BOOST_CHECK (unbind_listener.m_input_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_exists)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);
  
  dummy_unbind_failure_listener unbind_listener;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
		 ioa::make_action (input, &automaton1::up_uv_input), binder,
		 unbind_listener);
  BOOST_CHECK (unbind_listener.m_binding_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_success)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  dummy_unbind_success_listener usl;
  bind (system,
	ioa::make_action (output, &automaton1::p_uv_output, param),
	ioa::make_action (input, &automaton1::up_uv_input),
	usl);

  dummy_unbind_failure_listener ufl;
  system.unbind (ioa::make_action (output, &automaton1::p_uv_output, param),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 ufl);
  // No failures.
  BOOST_CHECK (!ufl.m_any);
  // Success.
  BOOST_CHECK (usl.m_unbound);
}

BOOST_AUTO_TEST_CASE (system_rescind_automaton_dne)
{
  ioa::system system;
  int parameter;
  
  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> automaton = create (system, new automaton1 (), dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, automaton, &parameter, rsl);

  destroy (system, automaton);

  dummy_rescind_failure_listener listener;
  system.rescind (automaton, param, listener);
  BOOST_CHECK (listener.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_rescind_exists)
{
  ioa::system system;
  int parameter;
  
  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> automaton = create (system, new automaton1 (), dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, automaton, &parameter, rsl);

  dummy_rescind_failure_listener rfl1;
  system.rescind (automaton, param, rfl1);
  BOOST_CHECK (!rfl1.m_any);

  dummy_rescind_failure_listener rfl2;
  system.rescind (automaton, param, rfl2);
  BOOST_CHECK (rfl2.m_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_rescind_success)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance, binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, input_instance, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, binder, &parameter, rsl);

  dummy_unbind_success_listener usl1;
  bind (system,
	ioa::make_action (binder, &automaton1::p_uv_output, param),
	ioa::make_action (input, &automaton1::up_uv_input),
	usl1);

  dummy_unbind_success_listener usl2;
  bind (system,
	ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (binder, &automaton1::p_uv_input, param),
	usl2);

  dummy_rescind_failure_listener rfl;
  system.rescind (binder, param, rfl);
  // No failures.
  BOOST_CHECK (!rfl.m_any);
  // Success.
  BOOST_CHECK (rsl.m_parameter_rescinded);
  BOOST_CHECK (usl1.m_unbound);
  BOOST_CHECK (usl2.m_unbound);
}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_dne)
{
  ioa::system system;
  dummy_destroy_success_listener parent_dsl;
  ioa::automaton_handle<automaton1> parent_handle = create (system, new automaton1 (), parent_dsl);
  dummy_destroy_success_listener child_dsl;
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, new automaton1 (), child_dsl);

  destroy (system, parent_handle);
  
  dummy_destroy_failure_listener listener2;
  system.destroy (parent_handle, child_handle, listener2);
  BOOST_CHECK (listener2.m_destroyer_dne);
}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_not_creator)
{
   ioa::system system;

  dummy_destroy_success_listener parent_dsl;
  ioa::automaton_handle<automaton1> parent_handle = create (system, new automaton1 (), parent_dsl);

  dummy_destroy_success_listener child_dsl;
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, new automaton1 (), child_dsl);

  dummy_destroy_success_listener third_party_dsl;
  ioa::automaton_handle<automaton1> third_party_handle = create (system, new automaton1 (), third_party_dsl);

   dummy_destroy_failure_listener listener;
   system.destroy (third_party_handle, child_handle, listener);
   BOOST_CHECK (listener.m_destroyer_not_creator);
}

BOOST_AUTO_TEST_CASE (system_destroy_exists)
{
  ioa::system system;

  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> automaton = create (system, new automaton1 (), dsl);

  destroy (system, automaton);
  dummy_destroy_failure_listener listener2;
  system.destroy (automaton, listener2);
  BOOST_CHECK (listener2.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_destroy_success)
{
  ioa::system system;
  int parameter;

  automaton1* alpha_instance = new automaton1 ();
  automaton1* beta_instance = new automaton1 ();
  automaton1* gamma_instance = new automaton1 ();

  dummy_destroy_success_listener alpha_dsl;
  ioa::automaton_handle<automaton1> alpha = create (system, alpha_instance, alpha_dsl);

  dummy_destroy_success_listener beta_dsl;
  ioa::automaton_handle<automaton1> beta = create (system, alpha, beta_instance, beta_dsl);

  dummy_destroy_success_listener gamma_dsl;
  ioa::automaton_handle<automaton1> gamma = create (system, beta, gamma_instance, gamma_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, beta, &parameter, rsl);

  dummy_unbind_success_listener usl1;
  bind (system,
	ioa::make_action (alpha, &automaton1::up_uv_output),
	ioa::make_action (beta, &automaton1::up_uv_input),
	alpha,
	usl1);
  dummy_unbind_success_listener usl2;
  bind (system,
	ioa::make_action (beta, &automaton1::p_uv_output, param),
	ioa::make_action (gamma, &automaton1::up_uv_input),
	usl2);
  dummy_unbind_success_listener usl3;
  bind (system,
	ioa::make_action (gamma, &automaton1::up_uv_output),
	ioa::make_action (beta, &automaton1::p_uv_input, param),
	usl3);

  dummy_destroy_failure_listener dfl;
  system.destroy (beta, dfl);
  // No failures.
  BOOST_CHECK (!dfl.m_any);
  // Success.
  BOOST_CHECK (beta_dsl.m_automaton_destroyed);
  BOOST_CHECK (gamma_dsl.m_automaton_destroyed);
  BOOST_CHECK (usl1.m_unbound);
  BOOST_CHECK (usl2.m_unbound);
  BOOST_CHECK (usl3.m_unbound);
}

BOOST_AUTO_TEST_CASE (system_execute_output_automaton_dne)
{
  ioa::system system;
  automaton1* output_instance = new automaton1 ();

  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  destroy (system, output);

  dummy_scheduler scheduler;
  dummy_execute_listener listener;
  system.execute (ioa::make_action (output, &automaton1::up_uv_output), scheduler, listener);
  BOOST_CHECK (listener.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_execute_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* output_instance = new automaton1 ();

  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  rescind (system, output, param);

  dummy_scheduler scheduler;
  dummy_execute_listener listener;
  system.execute (ioa::make_action (output, &automaton1::p_uv_output, param), scheduler, listener);
  BOOST_CHECK (listener.m_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_execute_output_success)
{
  ioa::system system;
  automaton1* output_instance = new automaton1 ();

  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, output_instance, output_dsl);

  dummy_scheduler scheduler;
  dummy_execute_listener listener;
  system.execute (ioa::make_action (output, &automaton1::up_uv_output), scheduler, listener);
  BOOST_CHECK (output_instance->up_uv_output.state);
}

BOOST_AUTO_TEST_CASE (system_deliver_event_automaton_dne)
{
  ioa::system system;
  automaton1* instance = new automaton1 ();

  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, instance, dsl);

  destroy (system, handle);

  dummy_scheduler scheduler;
  dummy_execute_listener listener1;
  system.execute (ioa::make_action (handle, &automaton1::uv_event), scheduler, listener1);

  dummy_execute_listener listener2;
  system.execute (ioa::make_action (handle, &automaton1::v_event, 9845), scheduler, listener2);
  BOOST_CHECK (listener2.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_deliver_event_success)
{
  ioa::system system;
  automaton1* instance = new automaton1 ();

  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, instance, dsl);

  dummy_scheduler scheduler;
  dummy_execute_listener listener1;
  system.execute (ioa::make_action (handle, &automaton1::uv_event), scheduler, listener1);
  BOOST_CHECK (instance->uv_event.state);

  dummy_execute_listener listener2;
  system.execute (ioa::make_action (handle, &automaton1::v_event, 9845), scheduler, listener2);
  BOOST_CHECK (instance->v_event.state);
  BOOST_CHECK_EQUAL (instance->v_event.last_value, 9845);
}

BOOST_AUTO_TEST_SUITE_END()
