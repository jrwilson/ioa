#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>

#include <system.hpp>
#include "automaton1.hpp"
#include "instance_holder.hpp"

class empty_class
{
};

class dummy_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_aid (const ioa::aid_t) { }
  void clear_current_aid (void) { }
};

template <class T>
struct dummy_create_listener
{
  bool m_creator_dne;
  bool m_created;
  bool m_instance_exists;
  ioa::automaton_handle<T> m_automaton;

  void instance_exists (const T*) {
    m_instance_exists = true;
  }

  void automaton_created (const ioa::automaton_handle<T>& automaton) {
    m_automaton = automaton;
    m_created = true;
  }

  template <class I, class D>
  void automaton_dne (const ioa::automaton_handle<I>&,
		      D&) {
    m_creator_dne = true;
  }

  template <class I, class D>
  void instance_exists (const ioa::automaton_handle<I>& automaton,
			const T*,
			D&) {
    m_instance_exists = true;
  }

  template <class I, class D>
  void automaton_created (const ioa::automaton_handle<I>& parent,
			  const ioa::automaton_handle<T>& automaton,
			  D&) {
    m_automaton = automaton;
    m_created = true;
  }
  
  dummy_create_listener () :
    m_creator_dne (false),
    m_created (false),
    m_instance_exists (false)
  { }
};

template <class T>
struct dummy_declare_listener
{
  bool m_automaton_dne;
  bool m_declared;
  bool m_parameter_exists;
  ioa::parameter_handle<T> m_parameter;

  template <class I, class D>
  void automaton_dne (const ioa::automaton_handle<I>&,
		      T*,
		      D&) {
    m_automaton_dne = true;
  }

  template <class I, class D>
  void parameter_exists (const ioa::automaton_handle<I>&,
			 D&) {
    m_parameter_exists = true;
  }

  template <class I, class D>
  void parameter_declared (const ioa::automaton_handle<I>&,
			   const ioa::parameter_handle<T>& parameter,
			   D&) {
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

  template <class OI, class OM, class II, class IM, class I>
  void automaton_dne (const ioa::action<OI, OM>&,
		      const ioa::action<II, IM>&,
		      const ioa::automaton_handle<I>&) {
    m_binder_automaton_dne = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void output_automaton_dne (const ioa::action<OI, OM>&,
			     const ioa::action<II, IM>&,
			     const ioa::automaton_handle<I>&,
			     D&) {
    m_output_automaton_dne = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void input_automaton_dne (const ioa::action<OI, OM>&,
			    const ioa::action<II, IM>&,
			    const ioa::automaton_handle<I>&,
			    D&) {
    m_input_automaton_dne = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void output_parameter_dne (const ioa::action<OI, OM>&,
			     const ioa::action<II, IM>&,
			     const ioa::automaton_handle<I>&,
			     D&) {
    m_output_parameter_dne = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void input_parameter_dne (const ioa::action<OI, OM>&,
			    const ioa::action<II, IM>&,
			    const ioa::automaton_handle<I>&,
			    D&) {
    m_input_parameter_dne = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void binding_exists (const ioa::action<OI, OM>&,
		       const ioa::action<II, IM>&,
		       const ioa::automaton_handle<I>&,
		       D&) {
    m_binding_exists = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void input_action_unavailable (const ioa::action<OI, OM>&,
				 const ioa::action<II, IM>&,
				 const ioa::automaton_handle<I>&,
				 D&) {
    m_input_action_unavailable= true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void output_action_unavailable (const ioa::action<OI, OM>&,
				  const ioa::action<II, IM>&,
				  const ioa::automaton_handle<I>&,
				  D&) {
    m_output_action_unavailable = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void bound (const ioa::action<OI, OM>&,
	      const ioa::action<II, IM>&,
	      const ioa::automaton_handle<I>&,
	      D&) {
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

  template <class OI, class OM, class II, class IM, class I, class D>
  void unbound (const ioa::action<OI, OM>&,
		const ioa::action<II, IM>&,
		const ioa::automaton_handle<I>&,
		D&) {
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

  template <class OI, class OM, class II, class IM, class I>
  void automaton_dne (const ioa::action<OI, OM>&,
		      const ioa::action<II, IM>&,
		      const ioa::automaton_handle<I>&) {
    m_binder_automaton_dne = true;
    m_any = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void output_automaton_dne (const ioa::action<OI, OM>&,
			     const ioa::action<II, IM>&,
			     const ioa::automaton_handle<I>&,
			     D&) {
    m_output_automaton_dne = true;
    m_any = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void input_automaton_dne (const ioa::action<OI, OM>&,
			    const ioa::action<II, IM>&,
			    const ioa::automaton_handle<I>&,
			    D&) {
    m_input_automaton_dne = true;
    m_any = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void output_parameter_dne (const ioa::action<OI, OM>&,
			     const ioa::action<II, IM>&,
			     const ioa::automaton_handle<I>&,
			     D&) {
    m_output_parameter_dne = true;
    m_any = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void input_parameter_dne (const ioa::action<OI, OM>&,
			    const ioa::action<II, IM>&,
			    const ioa::automaton_handle<I>&,
			    D&) {
    m_input_parameter_dne = true;
    m_any = true;
  }
  
  template <class OI, class OM, class II, class IM, class I, class D>
  void binding_dne (const ioa::action<OI, OM>&,
		    const ioa::action<II, IM>&,
		    const ioa::automaton_handle<I>&,
		    D&) {
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

  template <class I, class D>
  void parameter_rescinded (const ioa::automaton_handle<I>&,
			    D&) {
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

  template <class I, class P>
  void automaton_dne (const ioa::automaton_handle<I>& automaton,
		      const ioa::parameter_handle<P>& parameter) {
    m_automaton_dne = true;
    m_any = true;
  }

  template <class I, class D>
  void parameter_dne (const ioa::automaton_handle<I>& automaton,
		      D&) {
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

  template <class I>
  void automaton_destroyed (const ioa::automaton_handle<I>&) {
    m_automaton_destroyed = true;
  }

  template <class I, class D>
  void automaton_destroyed (const ioa::automaton_handle<I>&,
			    D&) {
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

  template <class I>
  void target_automaton_dne (const ioa::automaton_handle<I>&) {
    m_any = true;
    m_automaton_dne = true;
  }

  template <class I, class T, class D>
  void automaton_dne (const ioa::automaton_handle<I>&,
		      const ioa::automaton_handle<T>&,
		      D&) {
    m_destroyer_dne = true;
    m_any = true;
  }

  template <class I, class D>
  void target_automaton_dne (const ioa::automaton_handle<I>&,
			     D&) {
    m_any = true;
    m_automaton_dne = true;
  }

  template <class I, class D>
  void destroyer_not_creator (const ioa::automaton_handle<I>&,
			      D&) {
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

template <class G>
ioa::automaton_handle<typename G::result_type> create (ioa::system& system,
						       G generator,
						       dummy_destroy_success_listener& dsl) {
  dummy_create_listener<typename G::result_type> listener;
  dummy_scheduler s;
  system.create (generator, s, listener, dsl);
  BOOST_CHECK (listener.m_created);
  return listener.m_automaton;
}

template <class P, class G>
ioa::automaton_handle<typename G::result_type> create (ioa::system& system,
						       const ioa::automaton_handle<P>& creator,
						       G generator,
						       dummy_destroy_success_listener& dsl) {
  dummy_create_listener<typename G::result_type> listener;
  empty_class d;
  dummy_scheduler s;
  system.create (creator, generator, s, listener, dsl, d);
  BOOST_CHECK (listener.m_created);
  return listener.m_automaton;
}

template <class I, class T>
ioa::parameter_handle<T> declare (ioa::system& system,
				  const ioa::automaton_handle<I>& automaton,
				  T* parameter,
				  dummy_rescind_success_listener& rsl) {
  dummy_declare_listener<T> listener;
  empty_class d;
  system.declare (automaton, parameter, listener, rsl, d);
  BOOST_CHECK (listener.m_declared);
  return listener.m_parameter;
}

template <class OI, class OM, class II, class IM, class I>
void bind (ioa::system& system,
	   const ioa::action<OI, OM>& output_action,
	   const ioa::action<II, IM>& input_action,
	   const ioa::automaton_handle<I>& binder,
	   dummy_unbind_success_listener& usl) {
  dummy_bind_listener listener;
  empty_class d;
  system.bind (output_action, input_action, binder, listener, usl, d);
  BOOST_CHECK (listener.m_bound);
}

template <class OI, class OM, class II, class IM>
void bind (ioa::system& system,
	   const ioa::action<OI, OM>& output_action,
	   const ioa::action<II, IM>& input_action,
	   dummy_unbind_success_listener& usl) {
  dummy_bind_listener listener;
  empty_class d;
  system.bind (output_action, input_action, listener, usl, d);
  BOOST_CHECK (listener.m_bound);
}

template <class I, class P>
void rescind (ioa::system& system,
	      const ioa::automaton_handle<I>& automaton,
	      const ioa::parameter_handle<P>& parameter) {
  dummy_rescind_failure_listener rfl;
  empty_class d;
  system.rescind (automaton, parameter, rfl, d);
  BOOST_CHECK (!rfl.m_any);
}

template <class I>
void destroy (ioa::system& system,
	      const ioa::automaton_handle<I>& automaton)
{
  dummy_destroy_failure_listener listener;
  system.destroy (automaton, listener);
  BOOST_CHECK (!listener.m_any);
}

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

BOOST_AUTO_TEST_CASE (system_create_creator_dne)
{
  ioa::system system;
  dummy_create_listener<automaton1> create_listener1;
  dummy_destroy_success_listener dsl1;
  dummy_scheduler s;
  system.create (automaton1_generator (), s, create_listener1, dsl1);
  BOOST_CHECK (create_listener1.m_created);
  ioa::automaton_handle<automaton1> creator_handle = create_listener1.m_automaton;
  destroy (system, creator_handle);
  dummy_create_listener<automaton1> create_listener2;
  dummy_destroy_success_listener dsl2;
  empty_class d;
  system.create (creator_handle, automaton1_generator (), s, create_listener2, dsl2, d);
  BOOST_CHECK (create_listener2.m_creator_dne);
}

BOOST_AUTO_TEST_CASE (system_create_exists)
{
  ioa::system system;
  instance_holder<automaton1> holder (new automaton1 ());
  dummy_create_listener<automaton1> create_listener1;
  dummy_destroy_success_listener dsl1;
  dummy_scheduler s;
  system.create (holder, s, create_listener1, dsl1);
  BOOST_CHECK (create_listener1.m_created);
  dummy_create_listener<automaton1> create_listener2;
  dummy_destroy_success_listener dsl2;
  system.create (holder, s, create_listener2, dsl2);
  BOOST_CHECK (create_listener2.m_instance_exists);
}

BOOST_AUTO_TEST_CASE (system_create_success)
{
  ioa::system system;
  dummy_destroy_success_listener dsl;
  dummy_create_listener<automaton1> create_listener;
  dummy_scheduler s;
  system.create (automaton1_generator (), s, create_listener, dsl);
  BOOST_CHECK (create_listener.m_created);
}

BOOST_AUTO_TEST_CASE (system_declare_automaton_dne)
{
  ioa::system system;
  int parameter;
  
  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, automaton1_generator (), dsl);
  destroy (system, handle);
  dummy_declare_listener<int> declare_listener;
  dummy_rescind_success_listener rsl;
  empty_class d;
  system.declare (handle, &parameter, declare_listener, rsl, d);
  BOOST_CHECK (declare_listener.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_declare_exists)
{
  ioa::system system;
  int parameter;
  
  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, automaton1_generator (), dsl);
  dummy_declare_listener<int> declare_listener1;
  dummy_rescind_success_listener rsl1;
  empty_class d;
  system.declare (handle, &parameter, declare_listener1, rsl1, d);
  BOOST_CHECK (declare_listener1.m_declared);
  dummy_declare_listener<int> declare_listener2;
  dummy_rescind_success_listener rsl2;
  system.declare (handle, &parameter, declare_listener2, rsl2, d);
  BOOST_CHECK (declare_listener2.m_parameter_exists);
}

BOOST_AUTO_TEST_CASE (system_declare_success)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, automaton1_generator (), dsl);
  dummy_declare_listener<int> declare_listener;
  dummy_rescind_success_listener rsl;
  empty_class d;
  system.declare (handle, &parameter, declare_listener, rsl, d);
  BOOST_CHECK (declare_listener.m_declared);
}

BOOST_AUTO_TEST_CASE (system_bind_binder_automaton_dne)
{
  ioa::system system;
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  destroy (system, binder);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  empty_class d;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener, usl, d);

  BOOST_CHECK (bind_listener.m_binder_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_output_automaton_dne)
{
  ioa::system system;
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);
  destroy (system, output);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);
  
  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  empty_class d;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener, usl, d);
  BOOST_CHECK (bind_listener.m_output_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_input_automaton_dne)
{
  ioa::system system;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  destroy (system, input);
  
  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  empty_class d;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener, usl, d);
  BOOST_CHECK (bind_listener.m_input_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_output_parameter_dne)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  rescind (system, output, param);

  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  empty_class d;
  system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       bind_listener, usl, d);
  BOOST_CHECK (bind_listener.m_output_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_input_parameter_dne)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, input, &parameter, rsl);

  rescind (system, input, param);

  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  empty_class d;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::p_uv_input, param),
	       bind_listener, usl, d);
  BOOST_CHECK (bind_listener.m_input_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_exists)
{
  ioa::system system;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);
  
  dummy_bind_listener bind_listener1;
  dummy_unbind_success_listener usl1;
  empty_class d;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1, usl1, d);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  dummy_unbind_success_listener usl2;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener2, usl2, d);
  BOOST_CHECK (bind_listener2.m_binding_exists);
}

BOOST_AUTO_TEST_CASE (system_bind_input_action_unavailable)
{
  ioa::system system;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output1_dsl;
  ioa::automaton_handle<automaton1> output1 = create (system, automaton1_generator (), output1_dsl);

  dummy_destroy_success_listener output2_dsl;
  ioa::automaton_handle<automaton1> output2 = create (system, automaton1_generator (), output2_dsl);
  
  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  dummy_bind_listener bind_listener1;
  dummy_unbind_success_listener usl1;
  empty_class d;
  system.bind (ioa::make_action (output1, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1, usl1, d);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  dummy_unbind_success_listener usl2;
  system.bind (ioa::make_action (output2, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener2,usl2, d);
  BOOST_CHECK (bind_listener2.m_input_action_unavailable);
}

BOOST_AUTO_TEST_CASE (system_bind_output_action_unavailable)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, input, &parameter, rsl);

  dummy_bind_listener bind_listener1;
  dummy_unbind_success_listener usl1;
  empty_class d;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1, usl1, d);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  dummy_unbind_success_listener usl2;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::p_uv_input, param),
	       bind_listener2, usl2, d);
  BOOST_CHECK (bind_listener2.m_output_action_unavailable);
}

BOOST_AUTO_TEST_CASE (system_bind_success)
{
  ioa::system system;
  int parameter;

  automaton1* input_instance = new automaton1 ();
  instance_holder<automaton1> holder (input_instance);

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, holder, input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  dummy_bind_listener bind_listener;
  dummy_unbind_success_listener usl;
  empty_class d;
  system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       bind_listener, usl, d);
  BOOST_CHECK (bind_listener.m_bound);

  dummy_scheduler scheduler;
  dummy_execute_listener listener;
  system.execute (ioa::make_action (output, &automaton1::p_uv_output, param), scheduler, listener);
  BOOST_CHECK (input_instance->up_uv_input.state);
}

BOOST_AUTO_TEST_CASE (system_unbind_binder_automaton_dne)
{
  ioa::system system;
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);
  
  dummy_unbind_success_listener usl;
  bind (system,
	ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder,
	usl);

  destroy (system, binder);

  dummy_unbind_failure_listener unbind_listener;
  empty_class d;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 binder,
		 unbind_listener,
		 d);
  BOOST_CHECK (unbind_listener.m_binder_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_output_automaton_dne)
{
  ioa::system system;
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);
  
  dummy_unbind_success_listener usl;
  bind (system, ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder,
	usl);

  destroy (system, output);

  dummy_unbind_failure_listener unbind_listener;
  empty_class d;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output), 
		 ioa::make_action (input, &automaton1::up_uv_input),
		 binder,
		 unbind_listener,
		 d);
  BOOST_CHECK (unbind_listener.m_output_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_input_automaton_dne)
{
  ioa::system system;
  
  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);
  
  dummy_unbind_success_listener usl;
  bind (system, ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder,
	usl);

  destroy (system, input);

  dummy_unbind_failure_listener unbind_listener;
  empty_class d;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 binder,
		 unbind_listener,
		 d);
  BOOST_CHECK (unbind_listener.m_input_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_output_parameter_dne)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  rescind (system, output, param);

  dummy_unbind_failure_listener unbind_listener;
  empty_class d;
  system.unbind (ioa::make_action (output, &automaton1::p_uv_output, param),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 unbind_listener, d);
  BOOST_CHECK (unbind_listener.m_output_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_input_parameter_dne)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, input, &parameter, rsl);

  rescind (system, input, param);

  dummy_unbind_failure_listener unbind_listener;
  empty_class d;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output), 
		 ioa::make_action (input, &automaton1::p_uv_input, param),
		 unbind_listener, d);
  BOOST_CHECK (unbind_listener.m_input_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_exists)
{
  ioa::system system;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);
  
  dummy_unbind_failure_listener unbind_listener;
  empty_class d;
  system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
		 ioa::make_action (input, &automaton1::up_uv_input), binder,
		 unbind_listener,
		 d);
  BOOST_CHECK (unbind_listener.m_binding_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_success)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, output, &parameter, rsl);

  dummy_unbind_success_listener usl;
  bind (system,
	ioa::make_action (output, &automaton1::p_uv_output, param),
	ioa::make_action (input, &automaton1::up_uv_input),
	usl);

  dummy_unbind_failure_listener ufl;
  empty_class d;
  system.unbind (ioa::make_action (output, &automaton1::p_uv_output, param),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 ufl, d);
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
  ioa::automaton_handle<automaton1> automaton = create (system, automaton1_generator (), dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, automaton, &parameter, rsl);

  destroy (system, automaton);

  dummy_rescind_failure_listener listener;
  empty_class d;
  system.rescind (automaton, param, listener, d);
  BOOST_CHECK (listener.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_rescind_exists)
{
  ioa::system system;
  int parameter;
  
  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> automaton = create (system, automaton1_generator (), dsl);

  dummy_rescind_success_listener rsl;
  ioa::parameter_handle<int> param = declare (system, automaton, &parameter, rsl);

  dummy_rescind_failure_listener rfl1;
  empty_class d;
  system.rescind (automaton, param, rfl1, d);
  BOOST_CHECK (!rfl1.m_any);

  dummy_rescind_failure_listener rfl2;
  system.rescind (automaton, param, rfl2, d);
  BOOST_CHECK (rfl2.m_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_rescind_success)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener binder_dsl;
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), binder_dsl);
  
  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

  dummy_destroy_success_listener input_dsl;
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), input_dsl);

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
  empty_class d;
  system.rescind (binder, param, rfl, d);
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
  ioa::automaton_handle<automaton1> parent_handle = create (system, automaton1_generator (), parent_dsl);
  dummy_destroy_success_listener child_dsl;
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, automaton1_generator (), child_dsl);

  destroy (system, parent_handle);
  
  dummy_destroy_failure_listener listener2;
  empty_class d;
  system.destroy (parent_handle, child_handle, listener2, d);
  BOOST_CHECK (listener2.m_destroyer_dne);
}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_not_creator)
{
   ioa::system system;

  dummy_destroy_success_listener parent_dsl;
  ioa::automaton_handle<automaton1> parent_handle = create (system, automaton1_generator (), parent_dsl);

  dummy_destroy_success_listener child_dsl;
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, automaton1_generator (), child_dsl);

  dummy_destroy_success_listener third_party_dsl;
  ioa::automaton_handle<automaton1> third_party_handle = create (system, automaton1_generator (), third_party_dsl);

   dummy_destroy_failure_listener listener;
   empty_class d;
   system.destroy (third_party_handle, child_handle, listener, d);
   BOOST_CHECK (listener.m_destroyer_not_creator);
}

BOOST_AUTO_TEST_CASE (system_destroy_exists)
{
  ioa::system system;

  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> automaton = create (system, automaton1_generator (), dsl);

  destroy (system, automaton);
  dummy_destroy_failure_listener listener2;
  system.destroy (automaton, listener2);
  BOOST_CHECK (listener2.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_destroy_success)
{
  ioa::system system;
  int parameter;

  dummy_destroy_success_listener alpha_dsl;
  ioa::automaton_handle<automaton1> alpha = create (system, automaton1_generator (), alpha_dsl);

  dummy_destroy_success_listener beta_dsl;
  ioa::automaton_handle<automaton1> beta = create (system, alpha, automaton1_generator (), beta_dsl);

  dummy_destroy_success_listener gamma_dsl;
  ioa::automaton_handle<automaton1> gamma = create (system, beta, automaton1_generator (), gamma_dsl);

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

  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

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

  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), output_dsl);

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
  instance_holder<automaton1> holder (output_instance);

  dummy_destroy_success_listener output_dsl;
  ioa::automaton_handle<automaton1> output = create (system, holder, output_dsl);

  dummy_scheduler scheduler;
  dummy_execute_listener listener;
  system.execute (ioa::make_action (output, &automaton1::up_uv_output), scheduler, listener);
  BOOST_CHECK (output_instance->up_uv_output.state);
}

BOOST_AUTO_TEST_CASE (system_deliver_event_automaton_dne)
{
  ioa::system system;

  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, automaton1_generator (), dsl);

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
  instance_holder<automaton1> holder (instance);

  dummy_destroy_success_listener dsl;
  ioa::automaton_handle<automaton1> handle = create (system, holder, dsl);

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
