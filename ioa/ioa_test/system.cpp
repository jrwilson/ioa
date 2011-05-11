#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>

#include <system.hpp>
#include "automaton1.hpp"

struct dummy_create_listener :
  public ioa::create_listener_interface
{
  bool m_creator_dne;
  bool m_created;
  bool m_instance_exists;
  ioa::generic_automaton_handle m_automaton;

  void instance_exists (const void*) {
    m_instance_exists = true;
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    m_automaton = automaton;
    m_created = true;
  }

  void automaton_dne () {
    m_creator_dne = true;
  }

  void instance_exists (const ioa::generic_automaton_handle& automaton,
			const void*) {
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

template <class T>
ioa::automaton_handle<T> create (ioa::system& system,
				 T* instance) {
  dummy_create_listener listener;
  system.create (instance, listener);
  BOOST_CHECK (listener.m_created);
  BOOST_CHECK_EQUAL (listener.m_automaton.value (), instance);
  return ioa::cast_automaton<T> (listener.m_automaton);
}

template <class T>
ioa::automaton_handle<T> create (ioa::system& system,
				 const ioa::generic_automaton_handle& creator,
				 T* instance) {
  dummy_create_listener listener;
  system.create (creator, instance, listener);
  BOOST_CHECK (listener.m_created);
  BOOST_CHECK_EQUAL (listener.m_automaton.value (), instance);
  return ioa::cast_automaton<T> (listener.m_automaton);
}

struct dummy_declare_listener :
  public ioa::declare_listener_interface
{
  bool m_automaton_dne;
  bool m_declared;
  bool m_parameter_exists;
  ioa::generic_parameter_handle m_parameter;

  void automaton_dne () { m_automaton_dne = true; }

  void parameter_exists (const ioa::generic_automaton_handle&,
			 void*) {
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

template <class T>
ioa::parameter_handle<T> declare (ioa::system& system,
				  const ioa::generic_automaton_handle& automaton,
				  T* parameter) {
  dummy_declare_listener listener;
  system.declare (automaton, parameter, listener);
  BOOST_CHECK (listener.m_declared);
  BOOST_CHECK_EQUAL (listener.m_parameter.value (), parameter);
  return ioa::cast_parameter<T> (listener.m_parameter);
}

struct dummy_bind_listener :
  public ioa::bind_listener_interface
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

  void automaton_dne () { m_binder_automaton_dne = true; }
  void bind_output_automaton_dne (const ioa::generic_automaton_handle& binder) { m_output_automaton_dne = true; }
  void bind_input_automaton_dne (const ioa::generic_automaton_handle& binder) { m_input_automaton_dne = true; }
  void bind_output_parameter_dne (const ioa::generic_automaton_handle& binder) { m_output_parameter_dne = true; }
  void bind_input_parameter_dne (const ioa::generic_automaton_handle& binder) { m_input_parameter_dne = true; }
  void binding_exists (const ioa::generic_automaton_handle& binder) { m_binding_exists = true; }
  void input_action_unavailable (const ioa::generic_automaton_handle& binder) { m_input_action_unavailable= true; }
  void output_action_unavailable (const ioa::generic_automaton_handle& binder) { m_output_action_unavailable = true; }
  void bound (const ioa::generic_automaton_handle& binder) { m_bound = true; }

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

template <class OM, class IM>
void bind (ioa::system& system,
	   const ioa::action<OM>& output_action,
	   const ioa::action<IM>& input_action,
	   const ioa::generic_automaton_handle& binder) {
  dummy_bind_listener listener;
  system.bind (output_action, input_action, binder, listener);
  BOOST_CHECK (listener.m_bound);
}

template <class OM, class IM>
void bind (ioa::system& system,
	   const ioa::action<OM>& output_action,
	   const ioa::action<IM>& input_action) {
  dummy_bind_listener listener;
  system.bind (output_action, input_action, listener);
  BOOST_CHECK (listener.m_bound);
}

struct dummy_unbind_listener :
  public ioa::unbind_listener_interface
{
  bool m_binder_automaton_dne;
  bool m_output_automaton_dne;
  bool m_input_automaton_dne;
  bool m_output_parameter_dne;
  bool m_input_parameter_dne;
  bool m_binding_dne;
  bool m_unbound;

  void automaton_dne () { m_binder_automaton_dne = true; }
  void unbind_output_automaton_dne (const ioa::generic_automaton_handle& binder) { m_output_automaton_dne = true; }
  void unbind_input_automaton_dne (const ioa::generic_automaton_handle& binder) { m_input_automaton_dne = true; }
  void unbind_output_parameter_dne (const ioa::generic_automaton_handle& binder) { m_output_parameter_dne = true; }
  void unbind_input_parameter_dne (const ioa::generic_automaton_handle& binder) { m_input_parameter_dne = true; }
  void binding_dne (const ioa::generic_automaton_handle& binder) { m_binding_dne = true; }
  void unbound (const ioa::generic_automaton_handle& binder) { m_unbound = true; }

  dummy_unbind_listener () :
    m_binder_automaton_dne (false),
    m_output_automaton_dne (false),
    m_input_automaton_dne (false),
    m_output_parameter_dne (false),
    m_input_parameter_dne (false),
    m_binding_dne (false),
    m_unbound (false)
  { }
};

struct output_unbind_record
{
  ioa::generic_automaton_handle output_automaton;
  const void* output_member;
  ioa::generic_parameter_handle output_parameter;
  ioa::generic_automaton_handle input_automaton;
  const void* input_member;

  output_unbind_record (const ioa::generic_automaton_handle& output_automaton,
			   const void* output_member,
			   const ioa::generic_parameter_handle& output_parameter,
			   const ioa::generic_automaton_handle& input_automaton,
			   const void* input_member) :
    output_automaton (output_automaton),
    output_member (output_member),
    output_parameter (output_parameter),
    input_automaton (input_automaton),
    input_member (input_member)
  { }
  
  bool operator== (const output_unbind_record& odr) const {
    return output_automaton == odr.output_automaton &&
      output_member == odr.output_member &&
      output_parameter == odr.output_parameter &&
      input_automaton == odr.input_automaton &&
      input_member == odr.input_member;
  }
};

struct input_unbind_record
{
  ioa::generic_automaton_handle output_automaton;
  const void* output_member;
  ioa::generic_automaton_handle input_automaton;
  const void* input_member;
  ioa::generic_parameter_handle input_parameter;

  input_unbind_record (const ioa::generic_automaton_handle& output_automaton,
			  const void* output_member,
			  const ioa::generic_automaton_handle& input_automaton,
			  const void* input_member,
			  const ioa::generic_parameter_handle& input_parameter) :
    output_automaton (output_automaton),
    output_member (output_member),
    input_automaton (input_automaton),
    input_member (input_member),
    input_parameter (input_parameter)
  { }

  bool operator== (const input_unbind_record& odr) const {
    return output_automaton == odr.output_automaton &&
      output_member == odr.output_member &&
      input_automaton == odr.input_automaton &&
      input_member == odr.input_member &&
      input_parameter == odr.input_parameter;
  }
};

struct dummy_rescind_listener :
  public ioa::rescind_listener_interface
{
  bool m_automaton_dne;
  bool m_parameter_dne;
  bool m_rescinded;
  std::vector<output_unbind_record> output_records;
  std::vector<input_unbind_record> input_records;

  void automaton_dne () { m_automaton_dne = true; }
  void parameter_dne (const ioa::generic_automaton_handle& automaton,
		      const ioa::generic_parameter_handle& parameter) { m_parameter_dne = true; }
  void parameter_rescinded (const ioa::generic_automaton_handle& automaton,
			    void* parameter) { m_rescinded = true; }

  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member,
		const ioa::generic_automaton_handle& binder) {

  }

  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_parameter_handle& output_parameter,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member) {
    output_unbind_record odr (output_automaton, output_member, output_parameter, input_automaton, input_member);
    output_records.push_back (odr);
  }
  
  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member,
		const ioa::generic_parameter_handle& input_parameter) {
    input_unbind_record idr (output_automaton, output_member, input_automaton, input_member, input_parameter);
    input_records.push_back (idr);
  }

  dummy_rescind_listener () :
    m_automaton_dne (false),
    m_parameter_dne (false),
    m_rescinded (false)
  { }
};

void rescind (ioa::system& system,
	      const ioa::generic_automaton_handle& automaton,
	      const ioa::generic_parameter_handle& parameter) {
  dummy_rescind_listener listener;
  system.rescind (automaton, parameter, listener);
  BOOST_CHECK (listener.m_rescinded);
}

struct unbind_record
{
  ioa::generic_automaton_handle output_automaton;
  const void* output_member;
  ioa::generic_automaton_handle input_automaton;
  const void* input_member;
  ioa::generic_automaton_handle binder_automaton;

  unbind_record (const ioa::generic_automaton_handle& output_automaton,
		    const void* output_member,
		    const ioa::generic_automaton_handle& input_automaton,
		    const void* input_member,
		    const ioa::generic_automaton_handle& binder_automaton) :
    output_automaton (output_automaton),
    output_member (output_member),
    input_automaton (input_automaton),
    input_member (input_member),
    binder_automaton (binder_automaton)
  { }
  
  bool operator== (const unbind_record& odr) const {
    return output_automaton == odr.output_automaton &&
      output_member == odr.output_member &&
      input_automaton == odr.input_automaton &&
      input_member == odr.input_member &&
      binder_automaton == odr.binder_automaton;
  }
};

struct dummy_destroy_listener :
  public ioa::destroy_listener_interface
{
  bool m_destroyer_dne;
  bool m_destroyer_not_creator;
  bool m_automaton_dne;

  std::set<ioa::generic_automaton_handle> destroyed;
  std::vector<std::pair<ioa::generic_automaton_handle, ioa::generic_automaton_handle> > destroyed_records;
  std::vector<unbind_record> unbind_records;
  std::vector<output_unbind_record> output_records;
  std::vector<input_unbind_record> input_records;

  void automaton_dne () { m_destroyer_dne = true; }

  void target_automaton_dne (const ioa::generic_automaton_handle&) {
    m_automaton_dne = true;
  }

  void target_automaton_dne (const ioa::generic_automaton_handle&,
			     const ioa::generic_automaton_handle&) {
    m_automaton_dne = true;
  }

  void destroyer_not_creator (const ioa::generic_automaton_handle&,
			      const ioa::generic_automaton_handle&) {
    m_destroyer_not_creator = true;
  }

  void automaton_destroyed (const ioa::generic_automaton_handle& automaton) {
    destroyed.insert (automaton);
  }
  void automaton_destroyed (const ioa::generic_automaton_handle& parent,
			    const ioa::generic_automaton_handle& child) {
    destroyed_records.push_back (std::make_pair (parent, child));
  }

  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member,
		const ioa::generic_automaton_handle& binder_automaton) {
    unbind_record dr (output_automaton, output_member, input_automaton, input_member, binder_automaton);
    unbind_records.push_back (dr);
  }
  
  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_parameter_handle& output_parameter,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member) {
    output_unbind_record odr (output_automaton, output_member, output_parameter, input_automaton, input_member);
    output_records.push_back (odr);
  }

  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member,
		const ioa::generic_parameter_handle& input_parameter) {
    input_unbind_record idr (output_automaton, output_member, input_automaton, input_member, input_parameter);
    input_records.push_back (idr);
  }

  dummy_destroy_listener () :
    m_destroyer_dne (false),
    m_destroyer_not_creator (false),
    m_automaton_dne (false)
  { }
};

void
destroy (ioa::system& system,
	 const ioa::generic_automaton_handle& automaton)
{
  dummy_destroy_listener listener;
  system.destroy (automaton, listener);
  BOOST_CHECK_EQUAL (listener.destroyed.count (automaton), 1U);
}

class dummy_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_handle (const ioa::generic_automaton_handle&) { }
  void clear_current_handle (void) { }
};

struct dummy_execute_listener :
  public ioa::execute_listener_interface
{
  bool m_automaton_dne;
  bool m_parameter_dne;

  void automaton_dne () { m_automaton_dne = true; }
  void execute_parameter_dne () { m_parameter_dne = true; }

  dummy_execute_listener () :
    m_automaton_dne (false),
    m_parameter_dne (false)
  { }
};

BOOST_AUTO_TEST_SUITE(system_suite)

BOOST_AUTO_TEST_CASE (system_create_creator_dne)
{
  ioa::system system;
  dummy_create_listener create_listener1;
  automaton1* instance = new automaton1 ();
  system.create (instance, create_listener1);
  BOOST_CHECK (create_listener1.m_created);
  ioa::automaton_handle<automaton1> creator_handle = ioa::cast_automaton<automaton1> (create_listener1.m_automaton);
  destroy (system, creator_handle);
  dummy_create_listener create_listener2;
  system.create (creator_handle, new automaton1 (), create_listener2);
  BOOST_CHECK (create_listener2.m_creator_dne);
}

BOOST_AUTO_TEST_CASE (system_create_exists)
{
  ioa::system system;
  automaton1* a = new automaton1 ();
  dummy_create_listener create_listener1;
  system.create (a, create_listener1);
  BOOST_CHECK (create_listener1.m_created);
  dummy_create_listener create_listener2;
  system.create (a, create_listener2);
  BOOST_CHECK (create_listener2.m_instance_exists);
}

BOOST_AUTO_TEST_CASE (system_create_success)
{
  ioa::system system;
  automaton1* a = new automaton1 ();
  dummy_create_listener create_listener;
  system.create (a, create_listener);
  BOOST_CHECK (create_listener.m_created);
}

BOOST_AUTO_TEST_CASE (system_declare_automaton_dne)
{
  ioa::system system;
  int parameter;
  
  ioa::automaton_handle<automaton1> handle = create (system, new automaton1 ());
  destroy (system, handle);
  dummy_declare_listener declare_listener;
  system.declare (handle, &parameter, declare_listener);
  BOOST_CHECK (declare_listener.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_declare_exists)
{
  ioa::system system;
  int parameter;
  
  ioa::automaton_handle<automaton1> handle = create (system, new automaton1 ());
  dummy_declare_listener declare_listener1;
  system.declare (handle, &parameter, declare_listener1);
  BOOST_CHECK (declare_listener1.m_declared);
  dummy_declare_listener declare_listener2;
  system.declare (handle, &parameter, declare_listener2);
  BOOST_CHECK (declare_listener2.m_parameter_exists);
}

BOOST_AUTO_TEST_CASE (system_declare_success)
{
  ioa::system system;
  int parameter;

  ioa::automaton_handle<automaton1> handle = create (system, new automaton1 ());
  dummy_declare_listener declare_listener;
  system.declare (handle, &parameter, declare_listener);
  BOOST_CHECK (declare_listener.m_declared);
  BOOST_CHECK_EQUAL (declare_listener.m_parameter.value (), &parameter);
}

BOOST_AUTO_TEST_CASE (system_bind_binder_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  destroy (system, binder);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  dummy_bind_listener bind_listener;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener);

  BOOST_CHECK (bind_listener.m_binder_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_output_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  destroy (system, output);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);
  
  dummy_bind_listener bind_listener;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener);
  BOOST_CHECK (bind_listener.m_output_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_input_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  destroy (system, input);
  
  dummy_bind_listener bind_listener;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener);
  BOOST_CHECK (bind_listener.m_input_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  ioa::parameter_handle<int> param = declare (system, output, &parameter);

  rescind (system, output, param);

  dummy_bind_listener bind_listener;
  system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       bind_listener);
  BOOST_CHECK (bind_listener.m_output_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_input_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  ioa::parameter_handle<int> param = declare (system, input, &parameter);

  rescind (system, input, param);

  dummy_bind_listener bind_listener;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::p_uv_input, param),
	       bind_listener);
  BOOST_CHECK (bind_listener.m_input_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_exists)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);
  
  dummy_bind_listener bind_listener1;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener2);
  BOOST_CHECK (bind_listener2.m_binding_exists);
}

BOOST_AUTO_TEST_CASE (system_bind_input_action_unavailable)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output1_instance = new automaton1 ();
  automaton1* output2_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output1 = create (system, output1_instance);

  ioa::automaton_handle<automaton1> output2 = create (system, output2_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  dummy_bind_listener bind_listener1;
  system.bind (ioa::make_action (output1, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  system.bind (ioa::make_action (output2, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener2);
  BOOST_CHECK (bind_listener2.m_input_action_unavailable);
}

BOOST_AUTO_TEST_CASE (system_bind_output_action_unavailable)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  ioa::parameter_handle<int> param = declare (system, input, &parameter);

  dummy_bind_listener bind_listener1;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       binder,
	       bind_listener1);
  BOOST_CHECK (bind_listener1.m_bound);

  dummy_bind_listener bind_listener2;
  system.bind (ioa::make_action (output, &automaton1::up_uv_output),
	       ioa::make_action (input, &automaton1::p_uv_input, param),
	       bind_listener2);
  BOOST_CHECK (bind_listener2.m_output_action_unavailable);
}

BOOST_AUTO_TEST_CASE (system_bind_success)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  ioa::parameter_handle<int> param = declare (system, output, &parameter);

  dummy_bind_listener bind_listener;
  system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
	       ioa::make_action (input, &automaton1::up_uv_input),
	       bind_listener);
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
  
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);
  
  bind (system,
	ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder);

  destroy (system, binder);

  dummy_unbind_listener unbind_listener;
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
  
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);
  
  bind (system, ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder);

  destroy (system, output);

  dummy_unbind_listener unbind_listener;
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
  
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);
  
  bind (system, ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (input, &automaton1::up_uv_input),
	binder);

  destroy (system, input);

  dummy_unbind_listener unbind_listener;
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

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);

  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  ioa::parameter_handle<int> param = declare (system, output, &parameter);

  rescind (system, output, param);

  dummy_unbind_listener unbind_listener;
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

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);

  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  ioa::parameter_handle<int> param = declare (system, input, &parameter);

  rescind (system, input, param);

  dummy_unbind_listener unbind_listener;
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
  
  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);
  
  ioa::automaton_handle<automaton1> input = create (system, input_instance);
  
  dummy_unbind_listener unbind_listener;
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

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);

  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  ioa::parameter_handle<int> param = declare (system, output, &parameter);

  bind (system, ioa::make_action (output, &automaton1::p_uv_output, param),
	ioa::make_action (input, &automaton1::up_uv_input));

  dummy_unbind_listener unbind_listener;
  system.unbind (ioa::make_action (output, &automaton1::p_uv_output, param),
		 ioa::make_action (input, &automaton1::up_uv_input),
		 unbind_listener);
  BOOST_CHECK (unbind_listener.m_unbound);
}

BOOST_AUTO_TEST_CASE (system_rescind_automaton_dne)
{
  ioa::system system;
  int parameter;
  
  ioa::automaton_handle<automaton1> automaton = create (system, new automaton1 ());
  ioa::parameter_handle<int> param = declare (system, automaton, &parameter);

  destroy (system, automaton);

  dummy_rescind_listener listener;
  system.rescind (automaton, param, listener);
  BOOST_CHECK (listener.m_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_rescind_exists)
{
  ioa::system system;
  int parameter;
  
  ioa::automaton_handle<automaton1> automaton = create (system, new automaton1 ());
  ioa::parameter_handle<int> param = declare (system, automaton, &parameter);
  dummy_rescind_listener listener;
  system.rescind (automaton, param, listener);
  BOOST_CHECK (listener.m_rescinded);
  system.rescind (automaton, param, listener);
  BOOST_CHECK (listener.m_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_rescind_success)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::automaton_handle<automaton1> binder = create (system, binder_instance);
  
  ioa::automaton_handle<automaton1> output = create (system, output_instance);

  ioa::automaton_handle<automaton1> input = create (system, input_instance);

  ioa::parameter_handle<int> param = declare (system, binder, &parameter);

  bind (system, ioa::make_action (binder, &automaton1::p_uv_output, param),
	ioa::make_action (input, &automaton1::up_uv_input));

  bind (system, ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (binder, &automaton1::p_uv_input, param));

  dummy_rescind_listener listener;
  system.rescind (binder, param, listener);
  BOOST_CHECK (listener.m_rescinded);
  BOOST_CHECK (std::find (listener.output_records.begin (),
			  listener.output_records.end (),
			  output_unbind_record (binder,
						   &binder_instance->p_uv_output,
						   param,
						   input,
						   &input_instance->up_uv_input)) != listener.output_records.end ());
  BOOST_CHECK (std::find (listener.input_records.begin (),
			  listener.input_records.end (),
			  input_unbind_record (output,
						  &output_instance->up_uv_output,
						  binder,
						  &binder_instance->p_uv_input,
						  param)) != listener.input_records.end ());


}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_dne)
{
  ioa::system system;
  ioa::automaton_handle<automaton1> parent_handle = create (system, new automaton1 ());
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, new automaton1 ());

  destroy (system, parent_handle);
  
  dummy_destroy_listener listener2;
  system.destroy (parent_handle, child_handle, listener2);
  BOOST_CHECK (listener2.m_destroyer_dne);
}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_not_creator)
{
   ioa::system system;

  ioa::automaton_handle<automaton1> parent_handle = create (system, new automaton1 ());
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, new automaton1 ());
  ioa::automaton_handle<automaton1> third_party_handle = create (system, new automaton1 ());

   dummy_destroy_listener listener;
   system.destroy (third_party_handle, child_handle, listener);
   BOOST_CHECK (listener.m_destroyer_not_creator);
}

BOOST_AUTO_TEST_CASE (system_destroy_exists)
{
  ioa::system system;

  ioa::automaton_handle<automaton1> automaton = create (system, new automaton1 ());

  destroy (system, automaton);
  dummy_destroy_listener listener2;
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

  ioa::automaton_handle<automaton1> alpha = create (system, alpha_instance);

  ioa::automaton_handle<automaton1> beta = create (system, alpha, beta_instance);

  ioa::automaton_handle<automaton1> gamma = create (system, beta, gamma_instance);

  ioa::parameter_handle<int> param = declare (system, beta, &parameter);

  bind (system, ioa::make_action (alpha, &automaton1::up_uv_output),
	ioa::make_action (beta, &automaton1::up_uv_input),
	alpha);
  bind (system, ioa::make_action (beta, &automaton1::p_uv_output, param),
	ioa::make_action (gamma, &automaton1::up_uv_input));
  bind (system, ioa::make_action (gamma, &automaton1::up_uv_output),
	ioa::make_action (beta, &automaton1::p_uv_input, param));

  dummy_destroy_listener listener;
  system.destroy (beta, listener);

  std::pair <ioa::generic_automaton_handle, ioa::generic_automaton_handle> p1 (alpha, beta);
  BOOST_CHECK (std::find (listener.destroyed_records.begin (),
			  listener.destroyed_records.end (),
			  p1) != listener.destroyed_records.end ());
  std::pair <ioa::generic_automaton_handle, ioa::generic_automaton_handle> p2 (beta, gamma);
  BOOST_CHECK (std::find (listener.destroyed_records.begin (),
			  listener.destroyed_records.end (),
			  p2) != listener.destroyed_records.end ());
  BOOST_CHECK (std::find (listener.unbind_records.begin (),
			  listener.unbind_records.end (),
			  unbind_record (alpha,
					    &alpha_instance->up_uv_output,
					    beta,
					    &beta_instance->up_uv_input,
					    alpha)) != listener.unbind_records.end ());
  BOOST_CHECK (std::find (listener.output_records.begin (),
			  listener.output_records.end (),
			  output_unbind_record (beta,
						   &beta_instance->p_uv_output,
						   param,
						   gamma,
						   &gamma_instance->up_uv_input)) != listener.output_records.end ());
  BOOST_CHECK (std::find (listener.input_records.begin (),
  			  listener.input_records.end (),
  			  input_unbind_record (gamma,
  						  &gamma_instance->up_uv_output,
  						  beta,
  						  &beta_instance->p_uv_input,
  						  param)) != listener.input_records.end ());



}

BOOST_AUTO_TEST_CASE (system_execute_output_automaton_dne)
{
  ioa::system system;
  automaton1* output_instance = new automaton1 ();

  ioa::automaton_handle<automaton1> output = create (system, output_instance);

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

  ioa::automaton_handle<automaton1> output = create (system, output_instance);

  ioa::parameter_handle<int> param = declare (system, output, &parameter);

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

  ioa::automaton_handle<automaton1> output = create (system, output_instance);

  dummy_scheduler scheduler;
  dummy_execute_listener listener;
  system.execute (ioa::make_action (output, &automaton1::up_uv_output), scheduler, listener);
  BOOST_CHECK (output_instance->up_uv_output.state);
}

BOOST_AUTO_TEST_CASE (system_deliver_event_automaton_dne)
{
  ioa::system system;
  automaton1* instance = new automaton1 ();

  ioa::automaton_handle<automaton1> handle = create (system, instance);

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

  ioa::automaton_handle<automaton1> handle = create (system, instance);

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
