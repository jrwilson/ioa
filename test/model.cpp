#include "minunit.h"

#include "../lib/model.hpp"
#include <ioa/generator.hpp>
#include <ioa/action_executor.hpp>
#include "automaton1.hpp"

#include "instance_holder.hpp"
#include <iostream>

struct unbound_t {
  const ioa::aid_t aid;
  const ioa::unbound_t type;
  void* const key;

  unbound_t (const ioa::aid_t a,
	     const ioa::unbound_t t,
	     void* const k) :
    aid (a),
    type (t),
    key (k)
  { }

  bool operator< (const unbound_t other) const {
    if (aid != other.aid) {
      return aid < other.aid;
    }
    if (type != other.type) {
      return type < other.type;
    }
    return key < other.key;
  }
};

struct unbound_record
{
  ioa::aid_t m_aid;
  const void* m_member_ptr;
  size_t m_pid;

  unbound_record (const ioa::action_executor_interface& ac) :
    m_aid (ac.get_aid ()),
    m_member_ptr (ac.get_member_ptr ()),
    m_pid (ac.get_pid ())
  { }

  bool operator< (const unbound_record& other) const {
    if (m_aid != other.m_aid) {
      return m_aid < other.m_aid;
    }
    if (m_member_ptr != other.m_member_ptr) {
      return m_member_ptr < other.m_member_ptr;
    }
    return m_pid < other.m_pid;
  }
};

struct destroyed_t {
  const ioa::aid_t aid;
  const ioa::destroyed_t type;
  void* const key;

  destroyed_t (const ioa::aid_t a,
	       const ioa::destroyed_t t,
	       void* const k) :
    aid (a),
    type (t),
    key (k)
  { }

  bool operator< (const destroyed_t other) const {
    if (aid != other.aid) {
      return aid < other.aid;
    }
    if (type != other.type) {
      return type < other.type;
    }
    return key < other.key;
  }
};

struct test_system_scheduler :
  public ioa::system_scheduler_interface
{
  ioa::aid_t m_created_automaton;
  ioa::created_t m_created_type;
  void* m_created_key;
  ioa::aid_t m_created_aid;

  ioa::aid_t m_bound_automaton;
  ioa::bound_t m_bound_type;
  void* m_bound_key;

  ioa::aid_t m_output_bound_aid;
  const void* m_output_bound_member_ptr;
  size_t m_output_bound_pid;

  ioa::aid_t m_input_bound_aid;
  const void* m_input_bound_member_ptr;
  size_t m_input_bound_pid;

  std::set<unbound_t> m_unbound;

  std::set<unbound_record> m_output_unbound;

  std::set<unbound_record> m_input_unbound;

  std::set<destroyed_t> m_automaton_destroyed;

  test_system_scheduler () {
    reset ();
  }

  void reset () {
    m_created_automaton = -1;
    m_created_key = 0;
    m_created_aid = -1;

    m_bound_automaton = -1;
    m_bound_key = 0;

    m_output_bound_aid = -1;
    m_output_bound_member_ptr = 0;
    m_output_bound_pid = 0;

    m_input_bound_aid = -1;
    m_input_bound_member_ptr = 0;
    m_input_bound_pid = 0;
    
    m_unbound.clear ();

    m_output_unbound.clear ();
    
    m_input_unbound.clear ();
    
    m_automaton_destroyed.clear ();
  }

  void set_current_aid (ioa::aid_t) { }
  void clear_current_aid () { }

  void create (const ioa::aid_t automaton,
	       std::auto_ptr<ioa::generator_interface> generator,
	       void* const key) {
    assert (false);
  }

  void bind (const ioa::aid_t automaton,
	     ioa::shared_ptr<ioa::bind_executor_interface> exec,
	     void* const key) {
    assert (false);
  }

  void unbind (const ioa::aid_t automaton,
	       void* const key) {
    assert (false);
  }

  void destroy (const ioa::aid_t automaton,
		void* const key) {
    assert (false);
  }

  void created (const ioa::aid_t automaton,
		const ioa::created_t type,
		void* const key,
		const ioa::aid_t aid) {
    m_created_automaton = automaton;
    m_created_type = type;
    m_created_key = key;
    m_created_aid = aid;
  }

  void bound (const ioa::aid_t automaton,
	      const ioa::bound_t type,
	      void* const key) {
    m_bound_automaton = automaton;
    m_bound_type = type;
    m_bound_key = key;
  }

  void output_bound (const ioa::output_executor_interface& exec) {
    m_output_bound_aid = exec.get_aid ();
    m_output_bound_member_ptr = exec.get_member_ptr ();
    m_output_bound_pid = exec.get_pid ();
  }

  void input_bound (const ioa::input_executor_interface& exec) {
    m_input_bound_aid = exec.get_aid ();
    m_input_bound_member_ptr = exec.get_member_ptr ();
    m_input_bound_pid = exec.get_pid ();
  }

  void unbound (const ioa::aid_t automaton,
		const ioa::unbound_t type,
		void* const key) {
    m_unbound.insert (unbound_t (automaton, type, key));
  }

  void output_unbound (const ioa::output_executor_interface& exec) {
    m_output_unbound.insert (unbound_record (exec));
  }

  void input_unbound (const ioa::input_executor_interface& exec) {
    m_input_unbound.insert (unbound_record (exec));
  }
  
  void destroyed (const ioa::aid_t automaton,
		  const ioa::destroyed_t type,
		  void* const key) {
    m_automaton_destroyed.insert (destroyed_t (automaton, type, key));
  }
};

ioa::aid_t create (ioa::model& model,
		   test_system_scheduler& tss,
		   std::auto_ptr<ioa::generator_interface> generator) {
  tss.reset ();
  ioa::aid_t handle = model.create (generator);
  assert (handle != -1);

  return handle;
}

ioa::aid_t create (ioa::model& model,
		   test_system_scheduler& tss,
		   const ioa::aid_t creator,
		   std::auto_ptr<ioa::generator_interface> generator,
		   void* const key) {
  tss.reset ();
  ioa::aid_t handle = model.create (creator, generator, key);
  assert (handle != -1);
  assert (tss.m_created_automaton == creator);
  assert (tss.m_created_type == ioa::AUTOMATON_CREATED_RESULT);
  assert (tss.m_created_key == key);
  assert (tss.m_created_aid == handle);

  return handle;
}

void bind (ioa::model& model,
	   test_system_scheduler& tss,
	   const ioa::aid_t binder,
	   ioa::shared_ptr<ioa::bind_executor_interface> exec,
	   void* const key) {
  tss.reset ();
  assert (model.bind (binder, exec, key) == 0);
  assert (tss.m_bound_automaton == binder);
  assert (tss.m_bound_key == key);
  assert (tss.m_output_bound_aid == exec->get_output ().get_aid ());
  assert (tss.m_output_bound_member_ptr == exec->get_output ().get_member_ptr ());
  assert (tss.m_output_bound_pid == exec->get_output ().get_pid ());
  assert (tss.m_input_bound_aid == exec->get_input ().get_aid ());
  assert (tss.m_input_bound_member_ptr == exec->get_input ().get_member_ptr ());
  assert (tss.m_input_bound_pid == exec->get_input ().get_pid ());
}

void destroy (ioa::model& model,
	      test_system_scheduler& tss,
	      const ioa::aid_t automaton)
{
  tss.reset ();
  assert (model.destroy (automaton) == 0);
  // TODO:  Check something here.
}

static const char*
creator_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  ioa::aid_t creator_handle = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  destroy (model, tss, creator_handle);

  tss.reset ();
  ioa::aid_t handle = model.create (creator_handle, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()), 0);
  mu_assert (handle == -1);

  return 0;
}

static const char*
create_key_exists ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  ioa::aid_t creator_handle = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  int key;
  create (model, tss, creator_handle, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()), &key);

  tss.reset ();
  ioa::aid_t handle = model.create (creator_handle, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()), &key);
  mu_assert (handle == -1);
  mu_assert (tss.m_created_automaton == creator_handle);
  mu_assert (tss.m_created_type == ioa::CREATE_KEY_EXISTS_RESULT);
  mu_assert (tss.m_created_key == &key);
  
  return 0;
}

static const char*
instance_exists ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  automaton1* instance2 = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder2 (new instance_holder<automaton1> (instance2));
  std::auto_ptr<ioa::generator_interface> holder3 (new instance_holder<automaton1> (instance2));

  ioa::aid_t creator = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  int key1;
  create (model, tss, creator, holder2, &key1);

  tss.reset ();
  int key2;
  ioa::aid_t h2 = model.create (creator, holder3, &key2);
  mu_assert (h2 == -1);
  mu_assert (tss.m_created_automaton == creator);
  mu_assert (tss.m_created_type == ioa::INSTANCE_EXISTS_RESULT);
  mu_assert (tss.m_created_key == &key2);

  return 0;
}

static const char*
automaton_created ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  ioa::aid_t creator = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  int key;
  ioa::aid_t handle = model.create (creator, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()), &key);
  mu_assert (handle != -1);
  mu_assert (tss.m_created_automaton == creator);
  mu_assert (tss.m_created_type == ioa::AUTOMATON_CREATED_RESULT);
  mu_assert (tss.m_created_key == &key);
  mu_assert (tss.m_created_aid == handle);

  return 0;
}

static const char*
binder_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  ioa::aid_t binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  destroy (model, tss, binder);
  
  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  mu_assert (model.bind (binder,
			 ioa::make_bind_executor (output, &automaton1::uv_up_output,
						  input, &automaton1::uv_up_input),
			 0) == -1);
  
  return 0;
}

static const char*
bind_key_exists ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  ioa::aid_t binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> output1 = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> input1 = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));


  ioa::automaton_handle<automaton1> output2 = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> input2 = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  int key;
  bind (model, tss, binder,
  	ioa::make_bind_executor (output1, &automaton1::uv_up_output,
  				 input1, &automaton1::uv_up_input),
  	&key);
  
  tss.reset ();
  mu_assert (model.bind (binder,
  			 ioa::make_bind_executor (output2, &automaton1::uv_up_output,
  						  input2, &automaton1::uv_up_input),
  			 &key) == -1);
  mu_assert (tss.m_bound_automaton == binder);
  mu_assert (tss.m_bound_type == ioa::BIND_KEY_EXISTS_RESULT);
  mu_assert (tss.m_bound_key == &key);
  
  return 0;
}

static const char*
output_automaton_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  ioa::aid_t binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  destroy (model, tss, output);
  
  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  tss.reset ();
  int key;
  mu_assert (model.bind (binder,
			 ioa::make_bind_executor (output, &automaton1::uv_up_output,
						  input, &automaton1::uv_up_input),
			 &key) == -1);
  mu_assert (tss.m_bound_automaton == binder);
  mu_assert (tss.m_bound_type == ioa::OUTPUT_AUTOMATON_DNE_RESULT);
  mu_assert (tss.m_bound_key == &key);
  
  return 0;
}

static const char*
input_automaton_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  ioa::aid_t binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));


  destroy (model, tss, input);
  
  tss.reset ();
  int key;
  mu_assert (model.bind (binder,
			 ioa::make_bind_executor (output, &automaton1::uv_up_output,
						  input, &automaton1::uv_up_input),
			 &key) == -1);
  mu_assert (tss.m_bound_automaton == binder);
  mu_assert (tss.m_bound_type == ioa::INPUT_AUTOMATON_DNE_RESULT);
  mu_assert (tss.m_bound_key == &key);
  
  return 0;
}

static const char*
binding_exists ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  ioa::automaton_handle<automaton1> binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));


  int key1;
  bind (model, tss, binder,
	ioa::make_bind_executor (output, &automaton1::uv_up_output,
				 input, &automaton1::uv_up_input),
	&key1);
  
  tss.reset ();
  int key2;
  mu_assert (model.bind (binder,
			 ioa::make_bind_executor (output, &automaton1::uv_up_output,
						  input, &automaton1::uv_up_input),
			 &key2) == -1);
  mu_assert (tss.m_bound_automaton == binder);
  mu_assert (tss.m_bound_type == ioa::BINDING_EXISTS_RESULT);
  mu_assert (tss.m_bound_key == &key2);

  return 0;
}

static const char*
input_action_unavailable ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  ioa::automaton_handle<automaton1> binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> output1 = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  ioa::automaton_handle<automaton1> output2 = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  int key1;
  bind (model, tss, binder,
	ioa::make_bind_executor (output1, &automaton1::uv_up_output,
				 input, &automaton1::uv_up_input),
	&key1);

  tss.reset ();
  int key2;
  mu_assert (model.bind (binder,
			 ioa::make_bind_executor (output2, &automaton1::uv_up_output,
						  input, &automaton1::uv_up_input),
			 &key2) == -1);
  mu_assert (tss.m_bound_automaton == binder);
  mu_assert (tss.m_bound_type == ioa::INPUT_ACTION_UNAVAILABLE_RESULT);
  mu_assert (tss.m_bound_key == &key2);

  return 0;
}

static const char*
output_action_unavailable ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  int parameter = 0;

  ioa::automaton_handle<automaton1> binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
    
  int key1;
  bind (model, tss, binder,
	ioa::make_bind_executor (output, &automaton1::uv_up_output,
				 input, &automaton1::uv_up_input),
	&key1);

  tss.reset ();
  int key2;
  mu_assert (model.bind (input,
			 ioa::make_bind_executor (output, &automaton1::uv_up_output,
						  input, &automaton1::uv_p_input, parameter),
			 &key2) == -1);
  mu_assert (tss.m_bound_automaton == input);
  mu_assert (tss.m_bound_type == ioa::OUTPUT_ACTION_UNAVAILABLE_RESULT);
  mu_assert (tss.m_bound_key == &key2);

  return 0;
}

/*
  TODO:  Incorporate all of the tests in binding.cpp.
*/
static const char*
bound ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  int parameter = 0;

  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> input = create (model, tss, holder);
  
  tss.reset ();
  int key;
  mu_assert (model.bind (output,
			 ioa::make_bind_executor (output, &automaton1::uv_p_output, parameter,
						  input, &automaton1::uv_up_input),
			 &key) == 0);
  mu_assert (tss.m_bound_automaton == output);
  mu_assert (tss.m_bound_key == &key);

  ioa::action_executor<automaton1, automaton1::uv_p_output_action> out (output, &automaton1::uv_p_output, parameter);

  mu_assert (model.execute (out) == 0);
  mu_assert (instance->uv_up_input.state);

  return 0;
}

static const char*
unbinder_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  ioa::automaton_handle<automaton1> binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  bind (model, tss, binder,
	ioa::make_bind_executor (output, &automaton1::uv_up_output,
				 input, &automaton1::uv_up_input),
	0);
  
  destroy (model, tss, binder);

  mu_assert (model.unbind (binder,
			   0) == -1);

  return 0;
}

static const char*
bind_key_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  ioa::automaton_handle<automaton1> binder = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  int key;
  bind (model, tss, binder,
	ioa::make_bind_executor (output, &automaton1::uv_up_output,
				 input, &automaton1::uv_up_input),
	&key);
  
  tss.reset ();
  mu_assert (model.unbind (binder,
			   &key) == 0);
  mu_assert (tss.m_unbound.count (unbound_t (binder, ioa::UNBOUND_RESULT, &key)) == 1);

  tss.reset ();
  mu_assert (model.unbind (binder,
			   &key) == -1);

  mu_assert (tss.m_unbound.count (unbound_t (binder, ioa::BIND_KEY_DNE_RESULT, &key)) == 1);

  return 0;
}

static const char*
unbound ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  int parameter = 0;

  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  ioa::automaton_handle<automaton1> input = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  ioa::shared_ptr<ioa::bind_executor_interface> bind_exec = ioa::make_bind_executor (output, &automaton1::uv_p_output, parameter, input, &automaton1::uv_up_input);

  int key;
  bind (model, tss, output,
	bind_exec,
	&key);
  
  tss.reset ();
  mu_assert (model.unbind (output,
			   &key) == 0);
  mu_assert (tss.m_unbound.count (unbound_t (output, ioa::UNBOUND_RESULT, &key)) == 1);
  mu_assert (tss.m_output_unbound.count (unbound_record (bind_exec->get_output ())) == 1);
  mu_assert (tss.m_input_unbound.count (unbound_record (bind_exec->get_input ())) == 1);

  return 0;
}

static const char*
destroyer_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  ioa::automaton_handle<automaton1> parent_handle = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  int key;
  ioa::automaton_handle<automaton1> child_handle = create (model, tss, parent_handle, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()), &key);

  destroy (model, tss, parent_handle);
  
  mu_assert (model.destroy (parent_handle, &key) == -1);

  return 0;
}

static const char*
create_key_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);

  ioa::aid_t parent_handle = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  int key;
  ioa::automaton_handle<automaton1> child_handle = create (model, tss, parent_handle, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()), &key);
  
  tss.reset ();
  mu_assert (model.destroy (parent_handle, &key) == 0);
  mu_assert (tss.m_automaton_destroyed.count (destroyed_t (parent_handle, ioa::AUTOMATON_DESTROYED_RESULT, &key)) == 1);

  tss.reset ();
  mu_assert (model.destroy (parent_handle, &key) == -1);
  mu_assert (tss.m_automaton_destroyed.count (destroyed_t (parent_handle, ioa::CREATE_KEY_DNE_RESULT, &key)) == 1);

  return 0;
}

static const char*
automaton_destroyed ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  int parameter = 0;

  ioa::automaton_handle<automaton1> alpha = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));

  int beta_key;
  ioa::automaton_handle<automaton1> beta = create (model, tss, alpha, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()), &beta_key);

  int gamma_key;
  ioa::automaton_handle<automaton1> gamma = create (model, tss, beta, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()), &gamma_key);

  ioa::shared_ptr<ioa::bind_executor_interface> bind_exec1 = 
    ioa::make_bind_executor (alpha, &automaton1::uv_p_output, parameter,
			     beta, &automaton1::uv_up_input);

  ioa::shared_ptr<ioa::bind_executor_interface> bind_exec2 =
    ioa::make_bind_executor (beta, &automaton1::uv_p_output, parameter,
			     gamma, &automaton1::uv_up_input);

  ioa::shared_ptr<ioa::bind_executor_interface> bind_exec3 =
    ioa::make_bind_executor (gamma, &automaton1::uv_up_output,
			     beta, &automaton1::uv_p_input, parameter);

  int bind1;
  bind (model, tss, alpha,
	bind_exec1,
	&bind1);
  int bind2;
  bind (model, tss, beta,
	bind_exec2,
	&bind2);
  int bind3;
  bind (model, tss, beta,
	bind_exec3,
	&bind3);

  tss.reset ();
  mu_assert (model.destroy (beta) == 0);
  mu_assert (tss.m_automaton_destroyed.count (destroyed_t (alpha, ioa::AUTOMATON_DESTROYED_RESULT, &beta_key)) == 1);
  mu_assert (tss.m_automaton_destroyed.count (destroyed_t (beta, ioa::AUTOMATON_DESTROYED_RESULT, &gamma_key)) == 1);
  mu_assert (tss.m_unbound.count (unbound_t (alpha, ioa::UNBOUND_RESULT, &bind1)) == 1);
  mu_assert (tss.m_unbound.count (unbound_t (beta, ioa::UNBOUND_RESULT, &bind2)) == 1);
  mu_assert (tss.m_unbound.count (unbound_t (beta, ioa::UNBOUND_RESULT, &bind3)) == 1);
  
  mu_assert (tss.m_output_unbound.count (unbound_record (bind_exec1->get_output ())) == 1);
  mu_assert (tss.m_output_unbound.count (unbound_record (bind_exec2->get_output ())) == 1);
  mu_assert (tss.m_output_unbound.count (unbound_record (bind_exec3->get_output ())) == 1);

  mu_assert (tss.m_input_unbound.count (unbound_record (bind_exec1->get_input ())) == 1);
  mu_assert (tss.m_input_unbound.count (unbound_record (bind_exec2->get_input ())) == 1);
  mu_assert (tss.m_input_unbound.count (unbound_record (bind_exec3->get_input ())) == 1);

  return 0;
}

static const char*
execute_automaton_dne ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  ioa::automaton_handle<automaton1> output = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
  
  destroy (model, tss, output);
  
  ioa::action_executor<automaton1, automaton1::uv_up_output_action> exec (output, &automaton1::uv_up_output);
  mu_assert (model.execute (exec) == -1);
  
  return 0;
}

static const char*
execute_output ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  automaton1* output_instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (output_instance));

  ioa::automaton_handle<automaton1> output = create (model, tss, holder);

  ioa::action_executor<automaton1, automaton1::uv_up_output_action> exec1 (output, &automaton1::uv_up_output);
  mu_assert (model.execute (exec1) == 0);
  mu_assert (output_instance->uv_up_output.state);

  ioa::action_executor<automaton1, automaton1::uv_p_output_action> exec2 (output, &automaton1::uv_p_output, 567);
  mu_assert (model.execute (exec2) == 0);
  mu_assert (output_instance->uv_p_output.state);
  mu_assert (output_instance->uv_p_output.last_parameter == 567);

  ioa::action_executor<automaton1, automaton1::v_up_output_action> exec3 (output, &automaton1::v_up_output);
  mu_assert (model.execute (exec3) == 0);
  mu_assert (output_instance->v_up_output.state);

  ioa::action_executor<automaton1, automaton1::v_p_output_action> exec4 (output, &automaton1::v_p_output, 123);
  mu_assert (model.execute (exec4) == 0);
  mu_assert (output_instance->v_p_output.state);
  mu_assert (output_instance->v_p_output.last_parameter == 123);

  return 0;
}

static const char*
execute_internal ()
{
  std::cout << __func__ << std::endl;
  test_system_scheduler tss;
  ioa::model model (tss);
  
  automaton1* output_instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (output_instance));

  ioa::automaton_handle<automaton1> output = create (model, tss, holder);

  ioa::action_executor<automaton1, automaton1::up_internal_action> exec1 (output, &automaton1::up_internal);
  mu_assert (model.execute (exec1) == 0);
  mu_assert (output_instance->up_internal.state);

  ioa::action_executor<automaton1, automaton1::p_internal_action> exec2 (output, &automaton1::p_internal, 567);
  mu_assert (model.execute (exec2) == 0);
  mu_assert (output_instance->p_internal.state);
  mu_assert (output_instance->p_internal.last_parameter == 567);

  return 0;
}

// static const char*
// execute_system_input ()
// {
//   
//   ioa::model model (tss);
  
//   automaton1* event_instance = new automaton1 ();
//   std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (event_instance));

//   ioa::automaton_handle<automaton1> from = create (model, tss, std::auto_ptr<ioa::generator_interface> (ioa::make_generator<automaton1> ()));
//   ioa::automaton_handle<automaton1> to = create (model, tss, holder);

//   // tss.reset ();
//   // int key1;
//   // mu_assert (model.execute (from, ioa::make_action (to, &automaton1::uv_event), &key1) == 0);
//   // mu_assert (tss.m_event_delivered_automaton == from);
//   // mu_assert (tss.m_event_delivered_key == &key1);
//   // mu_assert (event_instance->uv_event.state);

//   // tss.reset ();
//   // int key2;
//   // mu_assert (model.execute (from, ioa::make_action (to, &automaton1::v_event, 37), &key2) == 0);
//   // mu_assert (tss.m_event_delivered_automaton == from);
//   // mu_assert (tss.m_event_delivered_key == &key2);
//   // mu_assert (event_instance->v_event.state);
//   // mu_assert (event_instance->v_event.last_value == 37);

//   return 0;
// }

const char*
all_tests ()
{
  mu_run_test (creator_dne);
  mu_run_test (create_key_exists);
  mu_run_test (instance_exists);
  mu_run_test (automaton_created);
  mu_run_test (binder_dne);
  mu_run_test (bind_key_exists);
  mu_run_test (output_automaton_dne);
  mu_run_test (input_automaton_dne);
  mu_run_test (binding_exists);
  mu_run_test (input_action_unavailable);
  mu_run_test (output_action_unavailable);
  mu_run_test (bound);
  mu_run_test (unbinder_dne);
  mu_run_test (bind_key_dne);
  mu_run_test (unbound);
  mu_run_test (destroyer_dne);
  mu_run_test (create_key_dne);
  mu_run_test (automaton_destroyed);
  mu_run_test (execute_automaton_dne);
  mu_run_test (execute_output);
  mu_run_test (execute_internal);
  // mu_run_test (execute_system_input);

  return 0;
}
