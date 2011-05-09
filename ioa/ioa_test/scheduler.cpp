#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE scheduler
#include <boost/test/unit_test.hpp>

#include <ioa.hpp>
#include "automaton2.hpp"

BOOST_AUTO_TEST_SUITE(scheduler_suite)

class create_exists
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

  void init_ () {
    ioa::scheduler.schedule (this, &create_exists::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, m_instance);
      m_state = CREATE1_SENT;
      break;
    case CREATE1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE1_RECV:
      ioa::scheduler.create (this, m_instance);
      m_state = CREATE2_SENT;
      break;
    case CREATE2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE2_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_state = CREATE1_RECV;
      ioa::scheduler.schedule (this, &create_exists::transition);
      break;
    case CREATE1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_EXISTS);
      m_state = CREATE2_RECV;
      ioa::scheduler.schedule (this, &create_exists::transition);
      break;
    case CREATE2_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<create_exists, &create_exists::init_> init;
  ioa::internal_wrapper<create_exists, &create_exists::transition_> transition;
  ioa::system_event_wrapper<create_exists, ioa::system::create_result, &create_exists::created_> created;

public:
  create_exists () :
    m_state (START),
    m_instance (new automaton2 ()),
    init (*this),
    transition (*this),
    created (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_create_exists)
{
  create_exists* instance = new create_exists ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, create_exists::STOP);
  ioa::scheduler.clear ();
}

class create_success
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

  void init_ () {
    ioa::scheduler.schedule (this, &create_success::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, m_instance);
      m_state = CREATE1_SENT;
      break;
    case CREATE1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE1_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_state = CREATE1_RECV;
      ioa::scheduler.schedule (this, &create_success::transition);
      break;
    case CREATE1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<create_success, &create_success::init_> init;
  ioa::internal_wrapper<create_success, &create_success::transition_> transition;
  ioa::system_event_wrapper<create_success, ioa::system::create_result, &create_success::created_> created;

public:
  create_success () :
    m_state (START),
    m_instance (new automaton2 ()),
    init (*this),
    transition (*this),
    created (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_create_success)
{
  create_success* instance = new create_success ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, create_success::STOP);
  ioa::scheduler.clear ();
}

class declare_exists
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

  void init_ () {
    ioa::scheduler.schedule (this, &declare_exists::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.declare (this, &m_parameter);
      m_state = DECLARE1_SENT;
      break;
    case DECLARE1_SENT:
      BOOST_CHECK (false);
      break;
    case DECLARE1_RECV:
      ioa::scheduler.declare (this, &m_parameter);
      m_state = DECLARE2_SENT;
      break;
    case DECLARE2_SENT:
      BOOST_CHECK (false);
      break;
    case DECLARE2_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void declared_ (const ioa::system::declare_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case DECLARE1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::DECLARE_SUCCESS);
      m_state = DECLARE1_RECV;
      ioa::scheduler.schedule (this, &declare_exists::transition);
      break;
    case DECLARE1_RECV:
      BOOST_CHECK (false);
      break;
    case DECLARE2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::DECLARE_EXISTS);
      m_state = DECLARE2_RECV;
      ioa::scheduler.schedule (this, &declare_exists::transition);
      break;
    case DECLARE2_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<declare_exists, &declare_exists::init_> init;
  ioa::internal_wrapper<declare_exists, &declare_exists::transition_> transition;
  ioa::system_event_wrapper<declare_exists, ioa::system::declare_result, &declare_exists::declared_> declared;

public:
  declare_exists () :
    m_state (START),
    init (*this),
    transition (*this),
    declared (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_declare_exists)
{
  declare_exists* instance = new declare_exists ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, declare_exists::STOP);
  ioa::scheduler.clear ();
}

class declare_success
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

  void init_ () {
    ioa::scheduler.schedule (this, &declare_success::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.declare (this, &m_parameter);
      m_state = DECLARE1_SENT;
      break;
    case DECLARE1_SENT:
      BOOST_CHECK (false);
      break;
    case DECLARE1_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void declared_ (const ioa::system::declare_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case DECLARE1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::DECLARE_SUCCESS);
      m_state = DECLARE1_RECV;
      ioa::scheduler.schedule (this, &declare_success::transition);
      break;
    case DECLARE1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<declare_success, &declare_success::init_> init;
  ioa::internal_wrapper<declare_success, &declare_success::transition_> transition;
  ioa::system_event_wrapper<declare_success, ioa::system::declare_result, &declare_success::declared_> declared;

public:
  declare_success () :
    m_state (START),
    init (*this),
    transition (*this),
    declared (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_declare_success)
{
  declare_success* instance = new declare_success ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, declare_success::STOP);
  ioa::scheduler.clear ();
}

class bind_output_automaton_dne
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

  void init_ () {
    ioa::scheduler.schedule (this, &bind_output_automaton_dne::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_child2 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_output_automaton_dne::transition);
      break;
    case CREATE_CHILD2_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound_ (const ioa::system::bind_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_OUTPUT_AUTOMATON_DNE);
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_output_automaton_dne::transition);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<bind_output_automaton_dne, &bind_output_automaton_dne::init_> init;
  ioa::internal_wrapper<bind_output_automaton_dne, &bind_output_automaton_dne::transition_> transition;
  ioa::system_event_wrapper<bind_output_automaton_dne, ioa::system::create_result, &bind_output_automaton_dne::created_> created;
  ioa::system_event_wrapper<bind_output_automaton_dne, ioa::system::bind_result, &bind_output_automaton_dne::bound_> bound;

public:
  bind_output_automaton_dne () :
    m_state (START),
    init (*this),
    transition (*this),
    created (*this),
    bound (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_output_automaton_dne)
{
  bind_output_automaton_dne* instance = new bind_output_automaton_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_output_automaton_dne::STOP);
  ioa::scheduler.clear ();
}

class bind_input_automaton_dne
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

  void init_ () {
    ioa::scheduler.schedule (this, &bind_input_automaton_dne::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_child1 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_input_automaton_dne::transition);
      break;
    case CREATE_CHILD1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound_ (const ioa::system::bind_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_INPUT_AUTOMATON_DNE);
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_input_automaton_dne::transition);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<bind_input_automaton_dne, &bind_input_automaton_dne::init_> init;
  ioa::internal_wrapper<bind_input_automaton_dne, &bind_input_automaton_dne::transition_> transition;
  ioa::system_event_wrapper<bind_input_automaton_dne, ioa::system::create_result, &bind_input_automaton_dne::created_> created;
  ioa::system_event_wrapper<bind_input_automaton_dne, ioa::system::bind_result, &bind_input_automaton_dne::bound_> bound;

public:
  bind_input_automaton_dne () :
    m_state (START),
    init (*this),
    transition (*this),
    created (*this),
    bound (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_input_automaton_dne)
{
  bind_input_automaton_dne* instance = new bind_input_automaton_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_input_automaton_dne::STOP);
  ioa::scheduler.clear ();
}

class bind_output_parameter_dne
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

  void init_ () {
    ioa::scheduler.schedule (this, &bind_output_parameter_dne::transition);
  }

  bool output_ (int* parameter) {
    return false;
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, &bind_output_parameter_dne::output, m_parameter, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_child2 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_output_parameter_dne::transition);
      break;
    case CREATE_CHILD2_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound_ (const ioa::system::bind_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_OUTPUT_PARAMETER_DNE);
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_output_parameter_dne::transition);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<bind_output_parameter_dne, &bind_output_parameter_dne::init_> init;
  ioa::void_parameter_output_wrapper<bind_output_parameter_dne, int, &bind_output_parameter_dne::output_> output;
  ioa::internal_wrapper<bind_output_parameter_dne, &bind_output_parameter_dne::transition_> transition;
  ioa::system_event_wrapper<bind_output_parameter_dne, ioa::system::create_result, &bind_output_parameter_dne::created_> created;
  ioa::system_event_wrapper<bind_output_parameter_dne, ioa::system::bind_result, &bind_output_parameter_dne::bound_> bound;

public:
  bind_output_parameter_dne () :
    m_state (START),
    init (*this),
    output (*this),
    transition (*this),
    created (*this),
    bound (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_output_parameter_dne)
{
  bind_output_parameter_dne* instance = new bind_output_parameter_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_output_parameter_dne::STOP);
  ioa::scheduler.clear ();
}

class bind_input_parameter_dne
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

  void init_ () {
    ioa::scheduler.schedule (this, &bind_input_parameter_dne::transition);
  }

  void input_ (int* parameter) {
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, &bind_input_parameter_dne::input, m_parameter);
      m_state = BIND1_SENT;
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_child1 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_input_parameter_dne::transition);
      break;
    case CREATE_CHILD1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound_ (const ioa::system::bind_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_INPUT_PARAMETER_DNE);
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_input_parameter_dne::transition);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<bind_input_parameter_dne, &bind_input_parameter_dne::init_> init;
  ioa::void_parameter_input_wrapper<bind_input_parameter_dne, int, &bind_input_parameter_dne::input_> input;
  ioa::internal_wrapper<bind_input_parameter_dne, &bind_input_parameter_dne::transition_> transition;
  ioa::system_event_wrapper<bind_input_parameter_dne, ioa::system::create_result, &bind_input_parameter_dne::created_> created;
  ioa::system_event_wrapper<bind_input_parameter_dne, ioa::system::bind_result, &bind_input_parameter_dne::bound_> bound;

public:
  bind_input_parameter_dne () :
    m_state (START),
    init (*this),
    input (*this),
    transition (*this),
    created (*this),
    bound (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_input_parameter_dne)
{
  bind_input_parameter_dne* instance = new bind_input_parameter_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_input_parameter_dne::STOP);
  ioa::scheduler.clear ();
}

class bind_exists
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

  void init_ () {
    ioa::scheduler.schedule (this, &bind_exists::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND2_SENT;
      break;
    case BIND2_SENT:
      BOOST_CHECK (false);
      break;
    case BIND2_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_child1 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_exists::transition);
      break;
    case CREATE_CHILD1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_child2 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_exists::transition);
      break;
    case CREATE_CHILD2_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND2_SENT:
      BOOST_CHECK (false);
      break;
    case BIND2_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound_ (const ioa::system::bind_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_SUCCESS);
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_exists::transition);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_EXISTS);
      m_state = BIND2_RECV;
      ioa::scheduler.schedule (this, &bind_exists::transition);
      break;
    case BIND2_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<bind_exists, &bind_exists::init_> init;
  ioa::internal_wrapper<bind_exists, &bind_exists::transition_> transition;
  ioa::system_event_wrapper<bind_exists, ioa::system::create_result, &bind_exists::created_> created;
  ioa::system_event_wrapper<bind_exists, ioa::system::bind_result, &bind_exists::bound_> bound;

public:
  bind_exists () :
    m_state (START),
    init (*this),
    transition (*this),
    created (*this),
    bound (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_exists)
{
  bind_exists* instance = new bind_exists ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_exists::STOP);
  ioa::scheduler.clear ();
}

class bind_input_action_unavailable
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

  void init_ () {
    ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_OUTPUT1_SENT;
      break;
    case CREATE_OUTPUT1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_OUTPUT2_SENT;
      break;
    case CREATE_OUTPUT2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT2_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_INPUT1_SENT;
      break;
    case CREATE_INPUT1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_INPUT1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_output2, &automaton2::output, m_input1, &automaton2::input);
      m_state = BIND2_SENT;
      break;
    case BIND2_SENT:
      BOOST_CHECK (false);
      break;
    case BIND2_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_output1 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_OUTPUT1_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    case CREATE_OUTPUT1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_output2 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_OUTPUT2_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    case CREATE_OUTPUT2_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_INPUT1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_input1 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_INPUT1_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    case CREATE_INPUT1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND2_SENT:
      BOOST_CHECK (false);
      break;
    case BIND2_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound_ (const ioa::system::bind_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT2_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_INPUT1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_INPUT1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_SUCCESS);
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_INPUT_ACTION_UNAVAILABLE);
      m_state = BIND2_RECV;
      ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
      break;
    case BIND2_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<bind_input_action_unavailable, &bind_input_action_unavailable::init_> init;
  ioa::internal_wrapper<bind_input_action_unavailable, &bind_input_action_unavailable::transition_> transition;
  ioa::system_event_wrapper<bind_input_action_unavailable, ioa::system::create_result, &bind_input_action_unavailable::created_> created;
  ioa::system_event_wrapper<bind_input_action_unavailable, ioa::system::bind_result, &bind_input_action_unavailable::bound_> bound;

public:
  bind_input_action_unavailable () :
    m_state (START),
    init (*this),
    transition (*this),
    created (*this),
    bound (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_input_action_unavailable)
{
  bind_input_action_unavailable* instance = new bind_input_action_unavailable ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_input_action_unavailable::STOP);
  ioa::scheduler.clear ();
}

class bind_output_action_unavailable
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

  void init_ () {
    ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_OUTPUT1_SENT;
      break;
    case CREATE_OUTPUT1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_INPUT1_SENT;
      break;
    case CREATE_INPUT1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_INPUT1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input2);
      m_state = BIND2_SENT;
      break;
    case BIND2_SENT:
      BOOST_CHECK (false);
      break;
    case BIND2_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_output1 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_OUTPUT1_RECV;
      ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
      break;
    case CREATE_OUTPUT1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_INPUT1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_input1 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_INPUT1_RECV;
      ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
      break;
    case CREATE_INPUT1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND2_SENT:
      BOOST_CHECK (false);
      break;
    case BIND2_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound_ (const ioa::system::bind_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_OUTPUT1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_INPUT1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_INPUT1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_SUCCESS);
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case BIND2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_OUTPUT_ACTION_UNAVAILABLE);
      m_state = BIND2_RECV;
      ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
      break;
    case BIND2_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<bind_output_action_unavailable, &bind_output_action_unavailable::init_> init;
  ioa::internal_wrapper<bind_output_action_unavailable, &bind_output_action_unavailable::transition_> transition;
  ioa::system_event_wrapper<bind_output_action_unavailable, ioa::system::create_result, &bind_output_action_unavailable::created_> created;
  ioa::system_event_wrapper<bind_output_action_unavailable, ioa::system::bind_result, &bind_output_action_unavailable::bound_> bound;

public:
  bind_output_action_unavailable () :
    m_state (START),
    init (*this),
    transition (*this),
    created (*this),
    bound (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_output_action_unavailable)
{
  bind_output_action_unavailable* instance = new bind_output_action_unavailable ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_output_action_unavailable::STOP);
  ioa::scheduler.clear ();
}

class bind_success
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

  void init_ () {
    ioa::scheduler.schedule (this, &bind_success::transition);
  }

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, new automaton2 ());
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
      m_state = BIND1_SENT;
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_child1 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (this, &bind_success::transition);
      break;
    case CREATE_CHILD1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_child2 = ioa::cast_automaton<automaton2> (r.automaton);
      m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (this, &bind_success::transition);
      break;
    case CREATE_CHILD2_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK (false);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

  void bound_ (const ioa::system::bind_result& r) {
    switch (m_state) {
    case START:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD1_RECV:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_SENT:
      BOOST_CHECK (false);
      break;
    case CREATE_CHILD2_RECV:
      BOOST_CHECK (false);
      break;
    case BIND1_SENT:
      BOOST_CHECK_EQUAL (r.type, ioa::system::BIND_SUCCESS);
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &bind_success::transition);
      break;
    case BIND1_RECV:
      BOOST_CHECK (false);
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<bind_success, &bind_success::init_> init;
  ioa::internal_wrapper<bind_success, &bind_success::transition_> transition;
  ioa::system_event_wrapper<bind_success, ioa::system::create_result, &bind_success::created_> created;
  ioa::system_event_wrapper<bind_success, ioa::system::bind_result, &bind_success::bound_> bound;

public:
  bind_success () :
    m_state (START),
    init (*this),
    transition (*this),
    created (*this),
    bound (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_bind_success)
{
  bind_success* instance = new bind_success ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, bind_success::STOP);
  ioa::scheduler.clear ();
}

// TODO      UNBIND_OUTPUT_AUTOMATON_DNE,
// TODO      UNBIND_INPUT_AUTOMATON_DNE,
// TODO      UNBIND_OUTPUT_PARAMETER_DNE,
// TODO      UNBIND_INPUT_PARAMETER_DNE,
// TODO      UNBIND_EXISTS,
// TODO      UNBIND_SUCCESS,

// TODO      RESCIND_EXISTS,
// TODO      RESCIND_SUCCESS,

// TODO      DESTROY_DESTROYER_NOT_CREATOR,
// TODO      DESTROY_EXISTS,
// TODO      DESTROY_SUCCESS

// TODO      EXECUTE_AUTOMATON_DNE,
// TODO      EXECUTE_PARAMETER_DNE,
// TODO      EXECUTE_SUCCESS,

  // TODO:  Send event to destroyed automaton.

BOOST_AUTO_TEST_SUITE_END()
