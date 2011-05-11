#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE scheduler
#include <boost/test/unit_test.hpp>

#include <ioa.hpp>
#include "automaton2.hpp"

BOOST_AUTO_TEST_SUITE(scheduler_suite)

class automaton_impl :
  public ioa::automaton_interface {

  virtual void init () { BOOST_CHECK (false); }
  virtual void instance_exists (const void*) { BOOST_CHECK (false); }
  virtual void automaton_created (const ioa::generic_automaton_handle&) { BOOST_CHECK (false); }
  virtual void parameter_exists (void*) { BOOST_CHECK (false); }
  virtual void parameter_declared (const ioa::generic_parameter_handle&) { BOOST_CHECK (false); }
  virtual void bind_output_automaton_dne () { BOOST_CHECK (false); }
  virtual void bind_input_automaton_dne () { BOOST_CHECK (false); }
  virtual void bind_output_parameter_dne () { BOOST_CHECK (false); }
  virtual void bind_input_parameter_dne () { BOOST_CHECK (false); }
  virtual void binding_exists () { BOOST_CHECK (false); }
  virtual void input_action_unavailable () { BOOST_CHECK (false); }
  virtual void output_action_unavailable () { BOOST_CHECK (false); }
  virtual void bound () { BOOST_CHECK (false); }
  virtual void unbind_output_automaton_dne () { BOOST_CHECK (false); }
  virtual void unbind_input_automaton_dne () { BOOST_CHECK (false); }
  virtual void unbind_output_parameter_dne () { BOOST_CHECK (false); }
  virtual void unbind_input_parameter_dne () { BOOST_CHECK (false); }
  virtual void binding_dne () { BOOST_CHECK (false); }
  virtual void unbound () { BOOST_CHECK (false); }
  virtual void parameter_dne (const ioa::generic_parameter_handle&) { BOOST_CHECK (false); }
  virtual void parameter_rescinded (void*) { BOOST_CHECK (false); }
  virtual void target_automaton_dne (const ioa::generic_automaton_handle&) { BOOST_CHECK (false); }
  virtual void destroyer_not_creator (const ioa::generic_automaton_handle&) { BOOST_CHECK (false); }
  virtual void automaton_destroyed (const ioa::generic_automaton_handle&) { BOOST_CHECK (false); }
};

class create_exists :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE1_SENT,
    CREATE1_RECV,
    CREATE2_SENT,
    CREATE2_RECV,
    STOP
  };
  state_type m_state;

private:
  automaton2* m_instance;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, m_instance);
      m_state = CREATE1_SENT;
      break;
    case CREATE1_RECV:
      ioa::scheduler.create (this, m_instance);
      m_state = CREATE2_SENT;
      break;
    case CREATE2_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<create_exists, &create_exists::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &create_exists::transition);
  }

  void instance_exists (const void* instance) {
    switch (m_state) {
    case CREATE2_SENT:
      BOOST_CHECK_EQUAL (m_instance, instance);
      m_state = CREATE2_RECV;
      ioa::scheduler.schedule (this, &create_exists::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void automaton_created (const ioa::generic_automaton_handle&) {
    switch (m_state) {
    case CREATE1_SENT:
      m_state = CREATE1_RECV;
      ioa::scheduler.schedule (this, &create_exists::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  create_exists () :
    m_state (START),
    m_instance (new automaton2 ()),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_create_exists)
{
  create_exists* instance = new create_exists ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, create_exists::STOP);
  ioa::scheduler.clear ();
}

class create_automaton_created :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE1_SENT,
    CREATE1_RECV,
    STOP
  };
  state_type m_state;

private:
  automaton2* m_instance;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, m_instance);
      m_state = CREATE1_SENT;
      break;
    case CREATE1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<create_automaton_created, &create_automaton_created::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &create_automaton_created::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle&) {
    switch (m_state) {
    case CREATE1_SENT:
      m_state = CREATE1_RECV;
      ioa::scheduler.schedule (this, &create_automaton_created::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  create_automaton_created () :
    m_state (START),
    m_instance (new automaton2 ()),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_create_automaton_created)
{
  create_automaton_created* instance = new create_automaton_created ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, create_automaton_created::STOP);
  ioa::scheduler.clear ();
}

class declare_exists :
  public automaton_impl
{
public:
  enum state_type {
    START,
    DECLARE1_SENT,
    DECLARE1_RECV,
    DECLARE2_SENT,
    DECLARE2_RECV,
    STOP
  };
  state_type m_state;

private:
  int m_parameter;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.declare (this, &m_parameter);
      m_state = DECLARE1_SENT;
      break;
    case DECLARE1_RECV:
      ioa::scheduler.declare (this, &m_parameter);
      m_state = DECLARE2_SENT;
      break;
    case DECLARE2_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<declare_exists, &declare_exists::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &declare_exists::transition);
  }

  void parameter_exists (void*) {
    switch (m_state) {
    case DECLARE2_SENT:
      m_state = DECLARE2_RECV;
      ioa::scheduler.schedule (this, &declare_exists::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  
  void parameter_declared (const ioa::generic_parameter_handle&) {
    switch (m_state) {
    case DECLARE1_SENT:
      m_state = DECLARE1_RECV;
      ioa::scheduler.schedule (this, &declare_exists::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  declare_exists () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_declare_exists)
{
  declare_exists* instance = new declare_exists ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, declare_exists::STOP);
  ioa::scheduler.clear ();
}

class declare_parameter_declared :
  public automaton_impl
{
public:
  enum state_type {
    START,
    DECLARE1_SENT,
    DECLARE1_RECV,
    STOP
  };
  state_type m_state;

private:
  int m_parameter;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.declare (this, &m_parameter);
      m_state = DECLARE1_SENT;
      break;
    case DECLARE1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<declare_parameter_declared, &declare_parameter_declared::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &declare_parameter_declared::transition);
  }

  void parameter_declared (const ioa::generic_parameter_handle&) {
    switch (m_state) {
    case DECLARE1_SENT:
      m_state = DECLARE1_RECV;
      ioa::scheduler.schedule (this, &declare_parameter_declared::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  declare_parameter_declared () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_declare_parameter_declared)
{
  declare_parameter_declared* instance = new declare_parameter_declared ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, declare_parameter_declared::STOP);
  ioa::scheduler.clear ();
}

class bind_output_automaton_dne_ :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD2_SENT,
    CREATE_CHILD2_RECV,
    BIND1_SENT,
    BIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_output_automaton_dne_, &bind_output_automaton_dne_::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_output_automaton_dne_::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD2_SENT:
      m_child2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_output_automaton_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bind_output_automaton_dne () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_output_automaton_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_output_automaton_dne_ () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_output_automaton_dne)
{
  bind_output_automaton_dne_* instance = new bind_output_automaton_dne_ ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_output_automaton_dne_::STOP);
  ioa::scheduler.clear ();
}

class bind_input_automaton_dne_ :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD1_SENT,
    CREATE_CHILD1_RECV,
    BIND1_SENT,
    BIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_input_automaton_dne_, &bind_input_automaton_dne_::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_input_automaton_dne_::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_input_automaton_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bind_input_automaton_dne () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_input_automaton_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_input_automaton_dne_ () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_input_automaton_dne)
{
  bind_input_automaton_dne_* instance = new bind_input_automaton_dne_ ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_input_automaton_dne_::STOP);
  ioa::scheduler.clear ();
}

class bind_output_parameter_dne_ :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD2_SENT,
    CREATE_CHILD2_RECV,
    BIND1_SENT,
    BIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::parameter_handle<int> m_parameter;
  ioa::automaton_handle<automaton2> m_child2;

  bool output_ (int* parameter) {
    return false;
  }
  ioa::void_parameter_output_wrapper<bind_output_parameter_dne_, int, &bind_output_parameter_dne_::output_> output;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, &bind_output_parameter_dne_::output, m_parameter, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_output_parameter_dne_, &bind_output_parameter_dne_::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_output_parameter_dne_::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD2_SENT:
      m_child2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_output_parameter_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bind_output_parameter_dne () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_output_parameter_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_output_parameter_dne_ () :
    m_state (START),
    output (*this),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_output_parameter_dne_)
{
  bind_output_parameter_dne_* instance = new bind_output_parameter_dne_ ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_output_parameter_dne_::STOP);
  ioa::scheduler.clear ();
}

class bind_input_parameter_dne_ :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD1_SENT,
    CREATE_CHILD1_RECV,
    BIND1_SENT,
    BIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::parameter_handle<int> m_parameter;

  void input_ (int* parameter) {
  }
  ioa::void_parameter_input_wrapper<bind_input_parameter_dne_, int, &bind_input_parameter_dne_::input_> input;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, &bind_input_parameter_dne_::input, m_parameter);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_input_parameter_dne_, &bind_input_parameter_dne_::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_input_parameter_dne_::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_input_parameter_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bind_input_parameter_dne () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_input_parameter_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_input_parameter_dne_ () :
    m_state (START),
    input (*this),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_input_parameter_dne_)
{
  bind_input_parameter_dne_* instance = new bind_input_parameter_dne_ ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_input_parameter_dne_::STOP);
  ioa::scheduler.clear ();
}

class bind_exists :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD1_SENT,
    CREATE_CHILD1_RECV,
    CREATE_CHILD2_SENT,
    CREATE_CHILD2_RECV,
    BIND1_SENT,
    BIND1_RECV,
    BIND2_SENT,
    BIND2_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND2_SENT;
      break;
    case BIND2_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_exists, &bind_exists::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_exists::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_exists::transition);
      break;
    case CREATE_CHILD2_SENT:
      m_child2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_exists::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void binding_exists () {
    switch (m_state) {
    case BIND2_SENT:
      m_state = BIND2_RECV;
      ioa::scheduler.schedule (this, &bind_exists::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_exists::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_exists () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_exists)
{
  bind_exists* instance = new bind_exists ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_exists::STOP);
  ioa::scheduler.clear ();
}

class bind_input_action_unavailable :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_OUTPUT1_SENT,
    CREATE_OUTPUT1_RECV,
    CREATE_OUTPUT2_SENT,
    CREATE_OUTPUT2_RECV,
    CREATE_INPUT1_SENT,
    CREATE_INPUT1_RECV,
    BIND1_SENT,
    BIND1_RECV,
    BIND2_SENT,
    BIND2_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_output1;
  ioa::automaton_handle<automaton2> m_output2;
  ioa::automaton_handle<automaton2> m_input1;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_OUTPUT1_SENT;
      break;
    case CREATE_OUTPUT1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_OUTPUT2_SENT;
      break;
    case CREATE_OUTPUT2_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_INPUT1_SENT;
      break;
    case CREATE_INPUT1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_output2, &automaton2::output, m_input1, &automaton2::input);
      m_state = BIND2_SENT;
      break;
    case BIND2_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_input_action_unavailable, &bind_input_action_unavailable::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_OUTPUT1_SENT:
      m_output1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_OUTPUT1_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    case CREATE_OUTPUT2_SENT:
      m_output2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_OUTPUT2_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    case CREATE_INPUT1_SENT:
      m_input1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_INPUT1_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void input_action_unavailable () {
    switch (m_state) {
    case BIND2_SENT:
      m_state = BIND2_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_input_action_unavailable () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_input_action_unavailable)
{
  bind_input_action_unavailable* instance = new bind_input_action_unavailable ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_input_action_unavailable::STOP);
  ioa::scheduler.clear ();
}

class bind_output_action_unavailable :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_OUTPUT1_SENT,
    CREATE_OUTPUT1_RECV,
    CREATE_INPUT1_SENT,
    CREATE_INPUT1_RECV,
    BIND1_SENT,
    BIND1_RECV,
    BIND2_SENT,
    BIND2_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_output1;
  ioa::automaton_handle<automaton2> m_input1;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_OUTPUT1_SENT;
      break;
    case CREATE_OUTPUT1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_INPUT1_SENT;
      break;
    case CREATE_INPUT1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input2);
      m_state = BIND2_SENT;
      break;
    case BIND2_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_output_action_unavailable, &bind_output_action_unavailable::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_OUTPUT1_SENT:
      m_output1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_OUTPUT1_RECV;
      ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
      break;
    case CREATE_INPUT1_SENT:
      m_input1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_INPUT1_RECV;
      ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void output_action_unavailable () {
    switch (m_state) {
    case BIND2_SENT:
      m_state = BIND2_RECV;
      ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_output_action_unavailable () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_output_action_unavailable)
{
  bind_output_action_unavailable* instance = new bind_output_action_unavailable ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_output_action_unavailable::STOP);
  ioa::scheduler.clear ();
}

class bind_bound :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD1_SENT,
    CREATE_CHILD1_RECV,
    CREATE_CHILD2_SENT,
    CREATE_CHILD2_RECV,
    BIND1_SENT,
    BIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_bound, &bind_bound::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_bound::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_bound::transition);
      break;
    case CREATE_CHILD2_SENT:
      m_child2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_bound::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_bound::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_bound () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_bound)
{
  bind_bound* instance = new bind_bound ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_bound::STOP);
  ioa::scheduler.clear ();
}

class unbind_output_automaton_dne_ :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD2_SENT,
    CREATE_CHILD2_RECV,
    UNBIND1_SENT,
    UNBIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = UNBIND1_SENT;
      break;
    case UNBIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<unbind_output_automaton_dne_, &unbind_output_automaton_dne_::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &unbind_output_automaton_dne_::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD2_SENT:
      m_child2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &unbind_output_automaton_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void unbind_output_automaton_dne () {
    switch (m_state) {
    case UNBIND1_SENT:
      m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (this, &unbind_output_automaton_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  unbind_output_automaton_dne_ () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_unbind_output_automaton_dne)
{
  unbind_output_automaton_dne_* instance = new unbind_output_automaton_dne_ ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, unbind_output_automaton_dne_::STOP);
  ioa::scheduler.clear ();
}

class unbind_input_automaton_dne_ :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD1_SENT,
    CREATE_CHILD1_RECV,
    UNBIND1_SENT,
    UNBIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = UNBIND1_SENT;
      break;
    case UNBIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<unbind_input_automaton_dne_, &unbind_input_automaton_dne_::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &unbind_input_automaton_dne_::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &unbind_input_automaton_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void unbind_input_automaton_dne () {
    switch (m_state) {
    case UNBIND1_SENT:
      m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (this, &unbind_input_automaton_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  unbind_input_automaton_dne_ () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_unbind_input_automaton_dne)
{
  unbind_input_automaton_dne_* instance = new unbind_input_automaton_dne_ ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, unbind_input_automaton_dne_::STOP);
  ioa::scheduler.clear ();
}

class unbind_output_parameter_dne_ :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD2_SENT,
    CREATE_CHILD2_RECV,
    UNBIND1_SENT,
    UNBIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::parameter_handle<int> m_parameter;
  ioa::automaton_handle<automaton2> m_child2;

  bool output_ (int* parameter) {
    return false;
  }
  ioa::void_parameter_output_wrapper<unbind_output_parameter_dne_, int, &unbind_output_parameter_dne_::output_> output;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.unbind (this, &unbind_output_parameter_dne_::output, m_parameter, m_child2, &automaton2::input);
      m_state = UNBIND1_SENT;
      break;
    case UNBIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<unbind_output_parameter_dne_, &unbind_output_parameter_dne_::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &unbind_output_parameter_dne_::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD2_SENT:
      m_child2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &unbind_output_parameter_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void unbind_output_parameter_dne () {
    switch (m_state) {
    case UNBIND1_SENT:
      m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (this, &unbind_output_parameter_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  unbind_output_parameter_dne_ () :
    m_state (START),
    output (*this),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_unbind_output_parameter_dne_)
{
  unbind_output_parameter_dne_* instance = new unbind_output_parameter_dne_ ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, unbind_output_parameter_dne_::STOP);
  ioa::scheduler.clear ();
}

class unbind_input_parameter_dne_ :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD1_SENT,
    CREATE_CHILD1_RECV,
    UNBIND1_SENT,
    UNBIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::parameter_handle<int> m_parameter;

  void input_ (int* parameter) {
  }
  ioa::void_parameter_input_wrapper<unbind_input_parameter_dne_, int, &unbind_input_parameter_dne_::input_> input;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, &unbind_input_parameter_dne_::input, m_parameter);
      m_state = UNBIND1_SENT;
      break;
    case UNBIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<unbind_input_parameter_dne_, &unbind_input_parameter_dne_::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &unbind_input_parameter_dne_::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &unbind_input_parameter_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void unbind_input_parameter_dne () {
    switch (m_state) {
    case UNBIND1_SENT:
      m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (this, &unbind_input_parameter_dne_::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  unbind_input_parameter_dne_ () :
    m_state (START),
    input (*this),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_unbind_input_parameter_dne_)
{
  unbind_input_parameter_dne_* instance = new unbind_input_parameter_dne_ ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, unbind_input_parameter_dne_::STOP);
  ioa::scheduler.clear ();
}

class bind_binding_dne :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD1_SENT,
    CREATE_CHILD1_RECV,
    CREATE_CHILD2_SENT,
    CREATE_CHILD2_RECV,
    UNBIND1_SENT,
    UNBIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = UNBIND1_SENT;
      break;
    case UNBIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<bind_binding_dne, &bind_binding_dne::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &bind_binding_dne::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_binding_dne::transition);
      break;
    case CREATE_CHILD2_SENT:
      m_child2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_binding_dne::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void binding_dne () {
    switch (m_state) {
    case UNBIND1_SENT:
      m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (this, &bind_binding_dne::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  bind_binding_dne () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_binding_dne)
{
  bind_binding_dne* instance = new bind_binding_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_binding_dne::STOP);
  ioa::scheduler.clear ();
}

class unbind_unbound :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE_CHILD1_SENT,
    CREATE_CHILD1_RECV,
    CREATE_CHILD2_SENT,
    CREATE_CHILD2_RECV,
    BIND1_SENT,
    BIND1_RECV,
    UNBIND1_SENT,
    UNBIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = UNBIND1_SENT;
      break;
    case UNBIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<unbind_unbound, &unbind_unbound::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &unbind_unbound::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_CHILD1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &unbind_unbound::transition);
      break;
    case CREATE_CHILD2_SENT:
      m_child2 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &unbind_unbound::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &unbind_unbound::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void unbound () {
    switch (m_state) {
    case UNBIND1_SENT:
      m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (this, &unbind_unbound::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  unbind_unbound () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_unbind_unbound)
{
  unbind_unbound* instance = new unbind_unbound ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, unbind_unbound::STOP);
  ioa::scheduler.clear ();
}

class rescind_parameter_dne :
  public automaton_impl
{
public:
  enum state_type {
    START,
    RESCIND1_SENT,
    RESCIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::generic_parameter_handle m_parameter;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.rescind (this, m_parameter);
      m_state = RESCIND1_SENT;
      break;
    case RESCIND1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<rescind_parameter_dne, &rescind_parameter_dne::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &rescind_parameter_dne::transition);
  }

  void parameter_dne (const ioa::generic_parameter_handle&) {
    switch (m_state) {
    case RESCIND1_SENT:
      m_state = RESCIND1_RECV;
      ioa::scheduler.schedule (this, &rescind_parameter_dne::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  rescind_parameter_dne () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_rescind_parameter_dne)
{
  rescind_parameter_dne* instance = new rescind_parameter_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, rescind_parameter_dne::STOP);
  ioa::scheduler.clear ();
}

class rescind_parameter_rescinded :
  public automaton_impl
{
public:
  enum state_type {
    START,
    DECLARE1_SENT,
    DECLARE1_RECV,
    RESCIND1_SENT,
    RESCIND1_RECV,
    STOP
  };
  state_type m_state;

private:
  int m_parameter1;
  ioa::parameter_handle<int> m_parameter2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.declare (this, &m_parameter1);
      m_state = DECLARE1_SENT;
      break;
    case DECLARE1_RECV:
      ioa::scheduler.rescind (this, m_parameter2);
      m_state = RESCIND1_SENT;
      break;
    case RESCIND1_RECV:
      m_state = STOP;
      break;

    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<rescind_parameter_rescinded, &rescind_parameter_rescinded::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &rescind_parameter_rescinded::transition);
  }

  void parameter_declared (const ioa::generic_parameter_handle& parameter) {
    switch (m_state) {
    case DECLARE1_SENT:
      m_parameter2 = ioa::cast_parameter<int> (parameter);
      m_state = DECLARE1_RECV;
      ioa::scheduler.schedule (this, &rescind_parameter_rescinded::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void parameter_rescinded (void*) {
    switch (m_state) {
    case RESCIND1_SENT:
      m_state = RESCIND1_RECV;
      ioa::scheduler.schedule (this, &rescind_parameter_rescinded::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  rescind_parameter_rescinded () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_rescind_parameter_rescinded)
{
  rescind_parameter_rescinded* instance = new rescind_parameter_rescinded ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, rescind_parameter_rescinded::STOP);
  ioa::scheduler.clear ();
}

class destroy_helper :
  public automaton_impl
{
public:
  enum state_type {
    START,
    DESTROY1_SENT,
    DESTROY1_RECV,
    STOP,
  };
  state_type m_state;

private:
  ioa::generic_automaton_handle m_automaton;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.destroy (this, m_automaton);
      m_state = DESTROY1_SENT;
      break;
    case DESTROY1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<destroy_helper, &destroy_helper::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &destroy_helper::transition);
  }

  void destroyer_not_creator (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
     case DESTROY1_SENT:
       BOOST_CHECK (m_automaton == automaton);
       m_state = DESTROY1_RECV;
       ioa::scheduler.schedule (this, &destroy_helper::transition);
       break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  destroy_helper (const ioa::generic_automaton_handle& automaton) :
    m_state (START),
    m_automaton (automaton),
    transition (*this)
  { }
};

class destroy_destroyer_not_creator :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE1_SENT,
    CREATE1_RECV,
    CREATE2_SENT,
    CREATE2_RECV,
    STOP
  };
  state_type m_state;
  destroy_helper* m_instance;

private:
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<destroy_helper> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE1_SENT;
      break;
    case CREATE1_RECV:
      m_instance = new destroy_helper (m_child1);
      ioa::scheduler.create (this, m_instance);
      m_state = CREATE2_SENT;
      break;
    case CREATE2_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<destroy_destroyer_not_creator, &destroy_destroyer_not_creator::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &destroy_destroyer_not_creator::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE1_SENT:
      m_child1 = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE1_RECV;
      ioa::scheduler.schedule (this, &destroy_destroyer_not_creator::transition);
      break;
    case CREATE2_SENT:
      m_child2 = ioa::cast_automaton<destroy_helper> (automaton);
      m_state = CREATE2_RECV;
      ioa::scheduler.schedule (this, &destroy_destroyer_not_creator::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  destroy_destroyer_not_creator () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_destroy_destroyer_not_creator)
{
  destroy_destroyer_not_creator* instance = new destroy_destroyer_not_creator ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, destroy_destroyer_not_creator::STOP);
  BOOST_CHECK_EQUAL (instance->m_instance->m_state, destroy_helper::STOP);
  ioa::scheduler.clear ();
}

class destroy_target_automaton_dne :
  public automaton_impl
{
public:
  enum state_type {
    START,
    DESTROY1_SENT,
    DESTROY1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.destroy (this, m_child);
      m_state = DESTROY1_SENT;
      break;
    case DESTROY1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<destroy_target_automaton_dne, &destroy_target_automaton_dne::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &destroy_target_automaton_dne::transition);
  }

  void target_automaton_dne (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case DESTROY1_SENT:
      BOOST_CHECK (m_child == automaton);
      m_state = DESTROY1_RECV;
      ioa::scheduler.schedule (this, &destroy_target_automaton_dne::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  destroy_target_automaton_dne () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_destroy_target_automaton_dne)
{
  destroy_target_automaton_dne* instance = new destroy_target_automaton_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, destroy_target_automaton_dne::STOP); // 
  ioa::scheduler.clear ();
}

class destroy_automaton_destroyed :
  public automaton_impl
{
public:
  enum state_type {
    START,
    CREATE1_SENT,
    CREATE1_RECV,
    DESTROY1_SENT,
    DESTROY1_RECV,
    STOP
  };
  state_type m_state;

private:
  ioa::automaton_handle<automaton2> m_child;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE1_SENT;
      break;
    case CREATE1_RECV:
      ioa::scheduler.destroy (this, m_child);
      m_state = DESTROY1_SENT;
      break;
    case DESTROY1_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<destroy_automaton_destroyed, &destroy_automaton_destroyed::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &destroy_automaton_destroyed::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE1_SENT:
      m_child = ioa::cast_automaton<automaton2> (automaton);
      m_state = CREATE1_RECV;
      ioa::scheduler.schedule (this, &destroy_automaton_destroyed::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

  void automaton_destroyed (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case DESTROY1_SENT:
      BOOST_CHECK (m_child == automaton);
      m_state = DESTROY1_RECV;
      ioa::scheduler.schedule (this, &destroy_automaton_destroyed::transition);
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  destroy_automaton_destroyed () :
    m_state (START),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_destroy_automaton_destroyed)
{
  destroy_automaton_destroyed* instance = new destroy_automaton_destroyed ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, destroy_automaton_destroyed::STOP); // 
  ioa::scheduler.clear ();
}

// TODO      EXECUTE_AUTOMATON_DNE,
// TODO      EXECUTE_PARAMETER_DNE,
// TODO      EXECUTE_SUCCESS,

// TODO:  Send event to destroyed automaton.

BOOST_AUTO_TEST_SUITE_END()
