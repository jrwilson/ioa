#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE scheduler
#include <boost/test/unit_test.hpp>

#include <ioa.hpp>
#include "automaton2.hpp"

BOOST_AUTO_TEST_SUITE(scheduler_suite)

class create_exists :
  public ioa::dispatching_automaton
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
  class gen
  {
  public:
    typedef automaton2 result_type;

    automaton2* m_instance;

    gen (automaton2* instance) :
      m_instance (instance)
    { }

    automaton2* operator() () {
      return m_instance;
    }
  };

  struct create1_d
  {
    create_exists& m_ce;

    create1_d (create_exists& ce) :
      m_ce (ce)
    { }

    template <class I>
    void automaton_created (const ioa::automaton_handle<I>&) {
      m_ce.m_state = CREATE1_RECV;
      ioa::scheduler.schedule (&m_ce, &create_exists::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create2_d
  {
    create_exists& m_ce;

    create2_d (create_exists& ce) :
      m_ce (ce)
    { }

    template <class I>
    void automaton_created (const ioa::automaton_handle<I>&) {
      BOOST_CHECK (false);
    }

    template <class I>
    void instance_exists (const I*) {
      m_ce.m_state = CREATE2_RECV;
      ioa::scheduler.schedule (&m_ce, &create_exists::transition);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  create1_d m_create1_d;
  create2_d m_create2_d;
  gen m_g;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, m_g, m_create1_d);
      m_state = CREATE1_SENT;
      break;
    case CREATE1_RECV:
      ioa::scheduler.create (this, m_g, m_create2_d);
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

public:
  void init () {
    ioa::scheduler.schedule (this, &create_exists::transition);
  }

  create_exists () :
    m_state (START),
    m_create1_d (*this),
    m_create2_d (*this),
    m_g (new automaton2 ()),
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
  public ioa::dispatching_automaton
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
  struct create1_d
  {
    create_automaton_created& m_ce;

    create1_d (create_automaton_created& ce) :
      m_ce (ce)
    { }

    template <class I>
    void automaton_created (const ioa::automaton_handle<I>&) {
      m_ce.m_state = CREATE1_RECV;
      ioa::scheduler.schedule (&m_ce, &create_automaton_created::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
    
  };

  create1_d m_create1_d;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
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

public:
  void init () {
    ioa::scheduler.schedule (this, &create_automaton_created::transition);
  }

  create_automaton_created () :
    m_state (START),
    m_create1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct declare1_d
  {
    declare_exists& m_ce;

    declare1_d (declare_exists& ce) :
      m_ce (ce)
    { }

    template <class P>
    void parameter_declared (const ioa::parameter_handle<P>&) {
      m_ce.m_state = DECLARE1_RECV;
      ioa::scheduler.schedule (&m_ce, &declare_exists::transition);
    }

    void parameter_exists () {
      BOOST_CHECK (false);
    }

    void parameter_rescinded () {
      BOOST_CHECK (false);
    }
    
  };

  struct declare2_d
  {
    declare_exists& m_ce;

    declare2_d (declare_exists& ce) :
      m_ce (ce)
    { }

    template <class P>
    void parameter_declared (const ioa::parameter_handle<P>&) {
      BOOST_CHECK (false);
    }

    void parameter_exists () {
      m_ce.m_state = DECLARE2_RECV;
      ioa::scheduler.schedule (&m_ce, &declare_exists::transition);
    }

    void parameter_rescinded () {
      BOOST_CHECK (false);
    }

  };

  declare1_d m_declare1_d;
  declare2_d m_declare2_d;
  int m_parameter;
  
  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.declare (this, &m_parameter, m_declare1_d);
      m_state = DECLARE1_SENT;
      break;
    case DECLARE1_RECV:
      ioa::scheduler.declare (this, &m_parameter, m_declare2_d);
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

public:
  void init () {
    ioa::scheduler.schedule (this, &declare_exists::transition);
  }

  declare_exists () :
    m_state (START),
    m_declare1_d (*this),
    m_declare2_d (*this),
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
  public ioa::dispatching_automaton
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

  struct declare1_d
  {
    declare_parameter_declared& m_ce;

    declare1_d (declare_parameter_declared& ce) :
      m_ce (ce)
    { }

    template <class P>
    void parameter_declared (const ioa::parameter_handle<P>&) {
      m_ce.m_state = DECLARE1_RECV;
      ioa::scheduler.schedule (&m_ce, &declare_parameter_declared::transition);
    }

    void parameter_exists () {
      BOOST_CHECK (false);
    }

    void parameter_rescinded () {
      BOOST_CHECK (false);
    }
    
  };

  declare1_d m_declare1_d;

  int m_parameter;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.declare (this, &m_parameter, m_declare1_d);
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

public:
  void init () {
    ioa::scheduler.schedule (this, &declare_parameter_declared::transition);
  }

  declare_parameter_declared () :
    m_state (START),
    m_declare1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    bind_output_automaton_dne_& m_ce;

    create1_d (bind_output_automaton_dne_& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_output_automaton_dne_::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    bind_output_automaton_dne_& m_ce;

    bind1_d (bind_output_automaton_dne_& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_output_automaton_dne_::transition);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }
    
    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      BOOST_CHECK (false);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  bind1_d m_bind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_bind1_d);
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

public:
  void init () {
    ioa::scheduler.schedule (this, &bind_output_automaton_dne_::transition);
  }

  bind_output_automaton_dne_ () :
    m_state (START),
    m_create1_d (*this),
    m_bind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    bind_input_automaton_dne_& m_ce;

    create1_d (bind_input_automaton_dne_& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_automaton_dne_::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    bind_input_automaton_dne_& m_ce;

    bind1_d (bind_input_automaton_dne_& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_automaton_dne_::transition);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }
    
    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      BOOST_CHECK (false);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  bind1_d m_bind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_bind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &bind_input_automaton_dne_::transition);
  }

  bind_input_automaton_dne_ () :
    m_state (START),
    m_create1_d (*this),
    m_bind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    bind_output_parameter_dne_& m_ce;

    create1_d (bind_output_parameter_dne_& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_output_parameter_dne_::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    bind_output_parameter_dne_& m_ce;

    bind1_d (bind_output_parameter_dne_& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_output_parameter_dne_::transition);
    }
    
    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      BOOST_CHECK (false);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  bind1_d m_bind1_d;
  ioa::parameter_handle<int> m_parameter;
  ioa::automaton_handle<automaton2> m_child2;

  bool output_ (int* parameter) {
    return false;
  }
  ioa::void_parameter_output_wrapper<bind_output_parameter_dne_, int, &bind_output_parameter_dne_::output_> output;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, &bind_output_parameter_dne_::output, m_parameter, m_child2, &automaton2::input, m_bind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &bind_output_parameter_dne_::transition);
  }

  bind_output_parameter_dne_ () :
    m_state (START),
    m_create1_d (*this),
    m_bind1_d (*this),
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
  public ioa::dispatching_automaton
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
  struct create1_d
  {
    bind_input_parameter_dne_& m_ce;

    create1_d (bind_input_parameter_dne_& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_parameter_dne_::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    bind_input_parameter_dne_& m_ce;

    bind1_d (bind_input_parameter_dne_& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_parameter_dne_::transition);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      BOOST_CHECK (false);
    }

    void unbound () {
      BOOST_CHECK (false);
    }
    
  };

  create1_d m_create1_d;
  bind1_d m_bind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::parameter_handle<int> m_parameter;

  void input_ (int* parameter) {
  }
  ioa::void_parameter_input_wrapper<bind_input_parameter_dne_, int, &bind_input_parameter_dne_::input_> input;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, &bind_input_parameter_dne_::input, m_parameter, m_bind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &bind_input_parameter_dne_::transition);
  }

  bind_input_parameter_dne_ () :
    m_state (START),
    m_create1_d (*this),
    m_bind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    bind_exists& m_ce;
    
    create1_d (bind_exists& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_exists::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create2_d
  {
    bind_exists& m_ce;
    
    create2_d (bind_exists& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_exists::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    bind_exists& m_ce;

    bind1_d (bind_exists& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_exists::transition);
    }

    void unbound () {
      BOOST_CHECK (false);
    }
    
  };

  struct bind2_d
  {
    bind_exists& m_ce;

    bind2_d (bind_exists& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      m_ce.m_state = BIND2_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_exists::transition);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      BOOST_CHECK (false);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  create2_d m_create2_d;
  bind1_d m_bind1_d;
  bind2_d m_bind2_d;

  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, automaton2_generator (), m_create2_d);
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_bind1_d);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_bind2_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &bind_exists::transition);
  }

  bind_exists () :
    m_state (START),
    m_create1_d (*this),
    m_create2_d (*this),
    m_bind1_d (*this),
    m_bind2_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    bind_input_action_unavailable& m_ce;
    
    create1_d (bind_input_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_output1 = automaton;
      m_ce.m_state = CREATE_OUTPUT1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create2_d
  {
    bind_input_action_unavailable& m_ce;
    
    create2_d (bind_input_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_output2 = automaton;
      m_ce.m_state = CREATE_OUTPUT2_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create3_d
  {
    bind_input_action_unavailable& m_ce;
    
    create3_d (bind_input_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_input1 = automaton;
      m_ce.m_state = CREATE_INPUT1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    bind_input_action_unavailable& m_ce;

    bind1_d (bind_input_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  struct bind2_d
  {
    bind_input_action_unavailable& m_ce;

    bind2_d (bind_input_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      m_ce.m_state = BIND2_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      BOOST_CHECK (false);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  create2_d m_create2_d;
  create3_d m_create3_d;
  bind1_d m_bind1_d;
  bind2_d m_bind2_d;
  ioa::automaton_handle<automaton2> m_output1;
  ioa::automaton_handle<automaton2> m_output2;
  ioa::automaton_handle<automaton2> m_input1;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_OUTPUT1_SENT;
      break;
    case CREATE_OUTPUT1_RECV:
      ioa::scheduler.create (this, automaton2_generator (), m_create2_d);
      m_state = CREATE_OUTPUT2_SENT;
      break;
    case CREATE_OUTPUT2_RECV:
      ioa::scheduler.create (this, automaton2_generator (), m_create3_d);
      m_state = CREATE_INPUT1_SENT;
      break;
    case CREATE_INPUT1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input, m_bind1_d);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_output2, &automaton2::output, m_input1, &automaton2::input, m_bind2_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
  }

  bind_input_action_unavailable () :
    m_state (START),
    m_create1_d (*this),
    m_create2_d (*this),
    m_create3_d (*this),
    m_bind1_d (*this),
    m_bind2_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    bind_output_action_unavailable& m_ce;
    
    create1_d (bind_output_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_output1 = automaton;
      m_ce.m_state = CREATE_OUTPUT1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_output_action_unavailable::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create2_d
  {
    bind_output_action_unavailable& m_ce;
    
    create2_d (bind_output_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_input1 = automaton;
      m_ce.m_state = CREATE_INPUT1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_output_action_unavailable::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    bind_output_action_unavailable& m_ce;

    bind1_d (bind_output_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_output_action_unavailable::transition);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  struct bind2_d
  {
    bind_output_action_unavailable& m_ce;

    bind2_d (bind_output_action_unavailable& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      m_ce.m_state = BIND2_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_output_action_unavailable::transition);
    }

    void bound () {
      BOOST_CHECK (false);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  create2_d m_create2_d;
  bind1_d m_bind1_d;
  bind2_d m_bind2_d;
  ioa::automaton_handle<automaton2> m_output1;
  ioa::automaton_handle<automaton2> m_input1;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_OUTPUT1_SENT;
      break;
    case CREATE_OUTPUT1_RECV:
      ioa::scheduler.create (this, automaton2_generator (), m_create2_d);
      m_state = CREATE_INPUT1_SENT;
      break;
    case CREATE_INPUT1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input, m_bind1_d);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input2, m_bind2_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
  }

  bind_output_action_unavailable () :
    m_state (START),
    m_create1_d (*this),
    m_create2_d (*this),
    m_bind1_d (*this),
    m_bind2_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    bind_bound& m_ce;
    
    create1_d (bind_bound& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_bound::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create2_d
  {
    bind_bound& m_ce;
    
    create2_d (bind_bound& ce) :
      m_ce (ce)
    { }
    
    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_bound::transition);
    }
    
    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }
    
    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    bind_bound& m_ce;

    bind1_d (bind_bound& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &bind_bound::transition);
    }

    void unbound () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  create2_d m_create2_d;
  bind1_d m_bind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, automaton2_generator (), m_create2_d);
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_bind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &bind_bound::transition);
  }

  bind_bound () :
    m_state (START),
    m_create1_d (*this),
    m_create2_d (*this),
    m_bind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    unbind_output_automaton_dne_& m_ce;

    create1_d (unbind_output_automaton_dne_& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_output_automaton_dne_::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct unbind1_d
  {
    unbind_output_automaton_dne_& m_ce;

    unbind1_d (unbind_output_automaton_dne_& ce) :
      m_ce (ce)
    { }
    
    void unbind_output_automaton_dne () {
      m_ce.m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_output_automaton_dne_::transition);
    }

    void unbind_input_automaton_dne () {
      BOOST_CHECK (false);
    }
    
    void unbind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_dne () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  unbind1_d m_unbind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_unbind1_d);
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

public:
  void init () {
    ioa::scheduler.schedule (this, &unbind_output_automaton_dne_::transition);
  }

  unbind_output_automaton_dne_ () :
    m_state (START),
    m_create1_d (*this),
    m_unbind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    unbind_input_automaton_dne_& m_ce;

    create1_d (unbind_input_automaton_dne_& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_input_automaton_dne_::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct unbind1_d
  {
    unbind_input_automaton_dne_& m_ce;

    unbind1_d (unbind_input_automaton_dne_& ce) :
      m_ce (ce)
    { }
    
    void unbind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_automaton_dne () {
      m_ce.m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_input_automaton_dne_::transition);
    }

    void unbind_output_parameter_dne () {
      BOOST_CHECK (false);
    }
    
    void unbind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_dne () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  unbind1_d m_unbind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_unbind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &unbind_input_automaton_dne_::transition);
  }

  unbind_input_automaton_dne_ () :
    m_state (START),
    m_create1_d (*this),
    m_unbind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    unbind_output_parameter_dne_& m_ce;

    create1_d (unbind_output_parameter_dne_& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_output_parameter_dne_::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct unbind1_d
  {
    unbind_output_parameter_dne_& m_ce;

    unbind1_d (unbind_output_parameter_dne_& ce) :
      m_ce (ce)
    { }
    
    void unbind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_output_parameter_dne () {
      m_ce.m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_output_parameter_dne_::transition);
    }
    
    void unbind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_dne () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  unbind1_d m_unbind1_d;
  ioa::parameter_handle<int> m_parameter;
  ioa::automaton_handle<automaton2> m_child2;

  bool output_ (int* parameter) {
    return false;
  }
  ioa::void_parameter_output_wrapper<unbind_output_parameter_dne_, int, &unbind_output_parameter_dne_::output_> output;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.unbind (this, &unbind_output_parameter_dne_::output, m_parameter, m_child2, &automaton2::input, m_unbind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &unbind_output_parameter_dne_::transition);
  }

  unbind_output_parameter_dne_ () :
    m_state (START),
    m_create1_d (*this),
    m_unbind1_d (*this),
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
  public ioa::dispatching_automaton
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
  struct create1_d
  {
    unbind_input_parameter_dne_& m_ce;

    create1_d (unbind_input_parameter_dne_& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_input_parameter_dne_::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct unbind1_d
  {
    unbind_input_parameter_dne_& m_ce;

    unbind1_d (unbind_input_parameter_dne_& ce) :
      m_ce (ce)
    { }
    
    void unbind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_parameter_dne () {
      m_ce.m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_input_parameter_dne_::transition);
    }

    void binding_dne () {
      BOOST_CHECK (false);
    }
    
  };

  create1_d m_create1_d;
  unbind1_d m_unbind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::parameter_handle<int> m_parameter;

  void input_ (int* parameter) {
  }
  ioa::void_parameter_input_wrapper<unbind_input_parameter_dne_, int, &unbind_input_parameter_dne_::input_> input;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, &unbind_input_parameter_dne_::input, m_parameter, m_unbind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &unbind_input_parameter_dne_::transition);
  }

  unbind_input_parameter_dne_ () :
    m_state (START),
    m_create1_d (*this),
    m_unbind1_d (*this),
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

class unbind_binding_dne :
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    unbind_binding_dne& m_ce;

    create1_d (unbind_binding_dne& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_binding_dne::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create2_d
  {
    unbind_binding_dne& m_ce;

    create2_d (unbind_binding_dne& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_binding_dne::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct unbind1_d
  {
    unbind_binding_dne& m_ce;

    unbind1_d (unbind_binding_dne& ce) :
      m_ce (ce)
    { }
    
    void unbind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_dne () {
      m_ce.m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_binding_dne::transition);
    }
    
  };

  create1_d m_create1_d;
  create2_d m_create2_d;
  unbind1_d m_unbind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, automaton2_generator (), m_create2_d);
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_unbind1_d);
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
  ioa::internal_wrapper<unbind_binding_dne, &unbind_binding_dne::transition_> transition;

public:

  void init () {
    ioa::scheduler.schedule (this, &unbind_binding_dne::transition);
  }

  unbind_binding_dne () :
    m_state (START),
    m_create1_d (*this),
    m_create2_d (*this),
    m_unbind1_d (*this),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_unbind_binding_dne)
{
  unbind_binding_dne* instance = new unbind_binding_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, unbind_binding_dne::STOP);
  ioa::scheduler.clear ();
}

class unbind_unbound :
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    unbind_unbound& m_ce;

    create1_d (unbind_unbound& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE_CHILD1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create2_d
  {
    unbind_unbound& m_ce;

    create2_d (unbind_unbound& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE_CHILD2_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct bind1_d
  {
    unbind_unbound& m_ce;

    bind1_d (unbind_unbound& ce) :
      m_ce (ce)
    { }
    
    void bind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void bind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void bind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_exists () {
      BOOST_CHECK (false);
    }

    void input_action_unavailable () {
      BOOST_CHECK (false);
    }

    void output_action_unavailable () {
      BOOST_CHECK (false);
    }

    void bound () {
      m_ce.m_state = BIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
    }

    void unbound () {
      m_ce.m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
    }

  };

  struct unbind1_d
  {
    unbind_unbound& m_ce;

    unbind1_d (unbind_unbound& ce) :
      m_ce (ce)
    { }
    
    void unbind_output_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_automaton_dne () {
      BOOST_CHECK (false);
    }

    void unbind_output_parameter_dne () {
      BOOST_CHECK (false);
    }

    void unbind_input_parameter_dne () {
      BOOST_CHECK (false);
    }

    void binding_dne () {
      m_ce.m_state = UNBIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
    }
    
  };

  create1_d m_create1_d;
  create2_d m_create2_d;
  bind1_d m_bind1_d;
  unbind1_d m_unbind1_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<automaton2> m_child2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE_CHILD1_SENT;
      break;
    case CREATE_CHILD1_RECV:
      ioa::scheduler.create (this, automaton2_generator (), m_create2_d);
      m_state = CREATE_CHILD2_SENT;
      break;
    case CREATE_CHILD2_RECV:
      ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_bind1_d);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input, m_unbind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &unbind_unbound::transition);
  }

  unbind_unbound () :
    m_state (START),
    m_create1_d (*this),
    m_create2_d (*this),
    m_bind1_d (*this),
    m_unbind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct rescind1_d
  {
    rescind_parameter_dne& m_ce;

    rescind1_d (rescind_parameter_dne& ce) :
      m_ce (ce)
    { }

    void parameter_dne () {
      m_ce.m_state = RESCIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &rescind_parameter_dne::transition);
    }

  };

  rescind1_d m_rescind1_d;
  ioa::parameter_handle<int> m_parameter;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.rescind (this, m_parameter, m_rescind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &rescind_parameter_dne::transition);
  }

  rescind_parameter_dne () :
    m_state (START),
    m_rescind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct declare1_d
  {
    rescind_parameter_rescinded& m_ce;

    declare1_d (rescind_parameter_rescinded& ce) :
      m_ce (ce)
    { }

    template <class P>
    void parameter_declared (const ioa::parameter_handle<P>&) {
      m_ce.m_state = DECLARE1_RECV;
      ioa::scheduler.schedule (&m_ce, &rescind_parameter_rescinded::transition);
    }

    void parameter_exists () {
      BOOST_CHECK (false);
    }

    void parameter_rescinded () {
      BOOST_CHECK (false);
    }
    
  };

  struct rescind1_d
  {
    rescind_parameter_rescinded& m_ce;

    rescind1_d (rescind_parameter_rescinded& ce) :
      m_ce (ce)
    { }

    void parameter_dne () {
      m_ce.m_state = RESCIND1_RECV;
      ioa::scheduler.schedule (&m_ce, &rescind_parameter_rescinded::transition);
    }

  };

  declare1_d m_declare1_d;
  rescind1_d m_rescind1_d;
  int m_parameter1;
  ioa::parameter_handle<int> m_parameter2;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.declare (this, &m_parameter1, m_declare1_d);
      m_state = DECLARE1_SENT;
      break;
    case DECLARE1_RECV:
      ioa::scheduler.rescind (this, m_parameter2, m_rescind1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &rescind_parameter_rescinded::transition);
  }

  rescind_parameter_rescinded () :
    m_state (START),
    m_declare1_d (*this),
    m_rescind1_d (*this),
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
  public ioa::dispatching_automaton
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

  struct destroy1_d
  {
    
    destroy_helper& m_ce;

    destroy1_d (destroy_helper& ce) :
      m_ce (ce)
    { }

    void target_automaton_dne () {
      BOOST_CHECK (false);
    }

    void destroyer_not_creator () {
      m_ce.m_state = DESTROY1_RECV;
      ioa::scheduler.schedule (&m_ce, &destroy_helper::transition);
    }

  };

  destroy1_d m_destroy1_d;
  ioa::automaton_handle<automaton2> m_automaton;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.destroy (this, m_automaton, m_destroy1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &destroy_helper::transition);
  }

  destroy_helper (const ioa::automaton_handle<automaton2>& automaton) :
    m_state (START),
    m_destroy1_d (*this),
    m_automaton (automaton),
    transition (*this)
  { }
};

class destroy_destroyer_not_creator :
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    destroy_destroyer_not_creator& m_ce;

    create1_d (destroy_destroyer_not_creator& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child1 = automaton;
      m_ce.m_state = CREATE1_RECV;
      ioa::scheduler.schedule (&m_ce, &destroy_destroyer_not_creator::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  struct create2_d
  {
    destroy_destroyer_not_creator& m_ce;

    create2_d (destroy_destroyer_not_creator& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<destroy_helper>& automaton) {
      m_ce.m_child2 = automaton;
      m_ce.m_state = CREATE2_RECV;
      ioa::scheduler.schedule (&m_ce, &destroy_destroyer_not_creator::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      BOOST_CHECK (false);
    }
  };

  create1_d m_create1_d;
  create2_d m_create2_d;
  ioa::automaton_handle<automaton2> m_child1;
  ioa::automaton_handle<destroy_helper> m_child2;
  
  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE1_SENT;
      break;
    case CREATE1_RECV:
      m_instance = new destroy_helper (m_child1);
      ioa::scheduler.create (this, instance_holder<destroy_helper> (m_instance), m_create2_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &destroy_destroyer_not_creator::transition);
  }

  destroy_destroyer_not_creator () :
    m_state (START),
    m_create1_d (*this),
    m_create2_d (*this),
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
  public ioa::dispatching_automaton
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

  struct destroy1_d
  {
    
    destroy_target_automaton_dne& m_ce;

    destroy1_d (destroy_target_automaton_dne& ce) :
      m_ce (ce)
    { }

    void target_automaton_dne () {
      m_ce.m_state = DESTROY1_RECV;
      ioa::scheduler.schedule (&m_ce, &destroy_target_automaton_dne::transition);
    }

    void destroyer_not_creator () {
      BOOST_CHECK (false);
    }

  };
  
  destroy1_d m_destroy1_d;
  ioa::automaton_handle<automaton2> m_child;
  
  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.destroy (this, m_child, m_destroy1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &destroy_target_automaton_dne::transition);
  }

  destroy_target_automaton_dne () :
    m_state (START),
    m_destroy1_d (*this),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_destroy_target_automaton_dne)
{
  destroy_target_automaton_dne* instance = new destroy_target_automaton_dne ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, destroy_target_automaton_dne::STOP);
  ioa::scheduler.clear ();
}

class destroy_automaton_destroyed :
  public ioa::dispatching_automaton
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

  struct create1_d
  {
    destroy_automaton_destroyed& m_ce;

    create1_d (destroy_automaton_destroyed& ce) :
      m_ce (ce)
    { }

    void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
      m_ce.m_child = automaton;
      m_ce.m_state = CREATE1_RECV;
      ioa::scheduler.schedule (&m_ce, &destroy_automaton_destroyed::transition);
    }

    template <class I>
    void instance_exists (const I*) {
      BOOST_CHECK (false);
    }

    void automaton_destroyed () {
      m_ce.m_state = DESTROY1_RECV;
      ioa::scheduler.schedule (&m_ce, &destroy_automaton_destroyed::transition);
    }
  };

  struct destroy1_d
  {

    destroy_automaton_destroyed& m_ce;

    destroy1_d (destroy_automaton_destroyed& ce) :
      m_ce (ce)
    { }

    void target_automaton_dne () {
      BOOST_CHECK (false);
    }

    void destroyer_not_creator () {
      BOOST_CHECK (false);
    }

  };

  create1_d m_create1_d;
  destroy1_d m_destroy1_d;
  ioa::automaton_handle<automaton2> m_child;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, automaton2_generator (), m_create1_d);
      m_state = CREATE1_SENT;
      break;
    case CREATE1_RECV:
      ioa::scheduler.destroy (this, m_child, m_destroy1_d);
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

public:

  void init () {
    ioa::scheduler.schedule (this, &destroy_automaton_destroyed::transition);
  }

  destroy_automaton_destroyed () :
    m_state (START),
    m_create1_d (*this),
    m_destroy1_d (*this),
    transition (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_destroy_automaton_destroyed)
{
  destroy_automaton_destroyed* instance = new destroy_automaton_destroyed ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, destroy_automaton_destroyed::STOP);
  ioa::scheduler.clear ();
}

class schedule_output :
  public ioa::dispatching_automaton
{
public:
  enum state_type {
    START,
    STOP
  };
  state_type m_state;

private:

  bool output_ () {
    switch (m_state) {
    case START:
      m_state = STOP;
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
    return false;
  }
  ioa::void_output_wrapper<schedule_output, &schedule_output::output_> output;

public:

  void init () {
    ioa::scheduler.schedule (this, &schedule_output::output);
  }

  schedule_output () :
    m_state (START),
    output (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_schedule_output)
{
  schedule_output* instance = new schedule_output ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, schedule_output::STOP);
  ioa::scheduler.clear ();
}


BOOST_AUTO_TEST_SUITE_END()
