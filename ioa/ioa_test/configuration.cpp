#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE configuration
#include <boost/test/unit_test.hpp>

#include <ioa.hpp>
#include "automaton2.hpp"
#include "instance_holder.hpp"

namespace automaton_dfa {

  typedef enum {
    START,
    CREATE_SENT,
    CREATE_RECV1,
    CREATE_RECV2,
    DESTROY_SENT,
    STOP,
    ERROR,
    STATE_COUNT,
  } state_type;

  typedef enum {
    // User symbols.
    CREATE,
    DESTROY,
    // System symbols.
    AUTOMATON_CREATED,
    INSTANCE_EXISTS,
    AUTOMATON_DESTROYED,
    TARGET_AUTOMATON_DNE,
    DESTROYER_NOT_CREATOR,
    SYMBOL_COUNT,
  } symbol_type;

  static const state_type transition[STATE_COUNT][SYMBOL_COUNT] =
    {
                        /* CREATE                      DESTROY                      AUTOMATON_CREATED            INSTANCE_EXISTS       AUTOMATON_DESTROYED   TARGET_AUTOMATON_DNE  DESTROYER_NOT_CREATOR */
      /* START */        { automaton_dfa::CREATE_SENT, automaton_dfa::ERROR,        automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
      /* CREATE_SENT */  { automaton_dfa::ERROR,       automaton_dfa::CREATE_RECV2, automaton_dfa::CREATE_RECV1, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
      /* CREATE_RECV1 */ { automaton_dfa::ERROR,       automaton_dfa::DESTROY_SENT, automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
      /* CREATE_RECV2 */ { automaton_dfa::ERROR,       automaton_dfa::ERROR,        automaton_dfa::DESTROY_SENT, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
      /* DESTROY_SENT */ { automaton_dfa::ERROR,       automaton_dfa::ERROR,        automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::STOP,  automaton_dfa::ERROR, automaton_dfa::ERROR  },
      /* STOP */         { automaton_dfa::ERROR,       automaton_dfa::ERROR,        automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR },
      /* ERROR */        { automaton_dfa::ERROR,       automaton_dfa::ERROR,        automaton_dfa::ERROR,        automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR, automaton_dfa::ERROR }
    };

};

template <class T, class G>
class automaton_helper
{
private:
  typedef typename G::result_type I;

  automaton_dfa::state_type m_state;
  const T* m_t;
  G m_generator;
  ioa::automaton_handle<I> m_handle;

public:
  automaton_helper (const T* t,
		    G generator) :
    m_state (automaton_dfa::START),
    m_t (t),
    m_generator (generator)
  { }

  void create () {
    switch (m_state) {
    case automaton_dfa::START:
      ioa::scheduler.create (m_t, m_generator, *this);
      break;
    default:
      break;
    }
    m_state = automaton_dfa::transition[m_state][automaton_dfa::CREATE];
  }

  void destroy () {
    switch (m_state) {
    case automaton_dfa::CREATE_RECV1:
      ioa::scheduler.destroy (m_t, m_handle, *this);
      break;
    default:
      break;
    }
    m_state = automaton_dfa::transition[m_state][automaton_dfa::DESTROY];
  }
  
  void automaton_created (const ioa::automaton_handle<I>& automaton) {
    switch (m_state) {
    case automaton_dfa::CREATE_SENT:
      m_handle = automaton;
      break;
    case automaton_dfa::CREATE_RECV2:
      m_handle = automaton;
      ioa::scheduler.destroy (m_t, m_handle, *this);
      break;
    default:
      break;
    }
    m_state = automaton_dfa::transition[m_state][automaton_dfa::AUTOMATON_CREATED];
  }
  
  void instance_exists (const I* /* */) {
    m_state = automaton_dfa::transition[m_state][automaton_dfa::INSTANCE_EXISTS];
  }
  
  void automaton_destroyed () {
    m_state = automaton_dfa::transition[m_state][automaton_dfa::AUTOMATON_DESTROYED];
  }

  void target_automaton_dne () {
    m_state = automaton_dfa::transition[m_state][automaton_dfa::TARGET_AUTOMATON_DNE];
  }

  void destroyer_not_creator () {
    m_state = automaton_dfa::transition[m_state][automaton_dfa::DESTROYER_NOT_CREATOR];
  }
  
  automaton_dfa::state_type get_state () const {
    return m_state;
  }
};

template <class P>
class parameter_helper
{
public:
  enum state_type {
    START,
    DECLARE_SENT,
    PARAMETER_EXISTS,
    PARAMETER_DECLARED,
  };

private:
  state_type m_state;
  P* m_parameter;
  ioa::parameter_handle<P> m_handle;

public:
  parameter_helper (P* parameter) :
    m_state (START),
    m_parameter (parameter)
  { }

  template <class T>
  void declare (const T* t) {
    BOOST_ASSERT (m_state == START);
    ioa::scheduler.declare (t, m_parameter, *this);
    m_state = DECLARE_SENT;
  }

  void rescind () {
    BOOST_ASSERT (false);
  }

  void parameter_declared (const ioa::parameter_handle<P>& parameter) {
    BOOST_ASSERT (m_state == DECLARE_SENT);
    m_handle = parameter;
    m_state = PARAMETER_DECLARED;
  }

  void parameter_exists () {
    BOOST_ASSERT (m_state == DECLARE_SENT);
    m_state = PARAMETER_EXISTS;
  }

  void parameter_rescinded () {
    BOOST_ASSERT (false);
  }

  state_type get_state () const {
    return m_state;
  }
};

BOOST_AUTO_TEST_SUITE(configuration_suite)

struct automaton_helper_automaton_created :
  public ioa::dispatching_automaton
{
  automaton_helper<automaton_helper_automaton_created, automaton2_generator> m_helper;

  automaton_helper_automaton_created () :
    m_helper (this, automaton2_generator ())
  {
    m_helper.create ();
  }

  ~automaton_helper_automaton_created () {
    BOOST_CHECK_EQUAL (m_helper.get_state (), automaton_dfa::CREATE_RECV1);
  }
};

BOOST_AUTO_TEST_CASE (config_automaton_helper_automaton_created)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_automaton_created> ());
  ioa::scheduler.clear ();
}

struct automaton_helper_instance_exists
  : public ioa::dispatching_automaton
{
  automaton2* m_instance;
  automaton_helper<automaton_helper_instance_exists, instance_holder<automaton2> > m_helper1;
  automaton_helper<automaton_helper_instance_exists, instance_holder<automaton2> > m_helper2;

  automaton_helper_instance_exists () :
    m_instance (new automaton2 ()),
    m_helper1 (this, m_instance),
    m_helper2 (this, m_instance)
  {
    m_helper1.create ();
    m_helper2.create ();
  }
 
  ~automaton_helper_instance_exists () {
    BOOST_CHECK ((m_helper1.get_state () == automaton_dfa::ERROR) ||
		 (m_helper2.get_state () == automaton_dfa::ERROR));
  }
};

BOOST_AUTO_TEST_CASE (config_automaton_helper_instance_exists)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_instance_exists> ());
  ioa::scheduler.clear ();
}

struct automaton_helper_automaton_destroyed :
  public ioa::dispatching_automaton
{
  automaton_helper<automaton_helper_automaton_destroyed, automaton2_generator> m_helper;

  void transition_ () {
    switch (m_helper.get_state ()) {
    case automaton_dfa::START:
    case automaton_dfa::CREATE_SENT:
      // Poll.
      ioa::scheduler.schedule (this, &automaton_helper_automaton_destroyed::transition);
      break;
    case automaton_dfa::CREATE_RECV1:
      // Destroy once created.
      m_helper.destroy ();
      break;
    default:
      BOOST_CHECK (false);
      break;
    }
  }
  ioa::internal_wrapper<automaton_helper_automaton_destroyed, &automaton_helper_automaton_destroyed::transition_> transition;

  automaton_helper_automaton_destroyed () :
    m_helper (this, automaton2_generator ()),
    transition (*this)
  {
    m_helper.create ();
    ioa::scheduler.schedule (this, &automaton_helper_automaton_destroyed::transition);
  }

  ~automaton_helper_automaton_destroyed () {
    BOOST_CHECK_EQUAL (m_helper.get_state (), automaton_dfa::STOP);
  }
};

BOOST_AUTO_TEST_CASE (config_automaton_helper_automaton_destroyed)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_automaton_destroyed> ());
  ioa::scheduler.clear ();
}

struct automaton_helper_automaton_destroyed_fast :
  public ioa::dispatching_automaton
{
  automaton_helper<automaton_helper_automaton_destroyed_fast, automaton2_generator> m_helper;

  automaton_helper_automaton_destroyed_fast () :
    m_helper (this, automaton2_generator ())
  {
    m_helper.create ();
    m_helper.destroy ();
  }

  ~automaton_helper_automaton_destroyed_fast () {
    BOOST_CHECK_EQUAL (m_helper.get_state (), automaton_dfa::STOP);
  }
};

BOOST_AUTO_TEST_CASE (config_automaton_helper_automaton_destroyed_fast)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_automaton_destroyed_fast> ());
  ioa::scheduler.clear ();
}









struct parameter_helper_parameter_exists :
  public ioa::dispatching_automaton
{
  int m_parameter;
  parameter_helper<int> m_helper1;
  parameter_helper<int> m_helper2;

  parameter_helper_parameter_exists () :
    m_helper1 (&m_parameter),
    m_helper2 (&m_parameter)
  {
    m_helper1.declare (this);
    m_helper2.declare (this);
  }

  ~parameter_helper_parameter_exists () {
    BOOST_CHECK ((m_helper1.get_state () == parameter_helper<int>::PARAMETER_EXISTS) ||
		 (m_helper2.get_state () == parameter_helper<int>::PARAMETER_EXISTS));
  }
};

BOOST_AUTO_TEST_CASE (config_parameter_helper_parameter_exists)
{
  ioa::scheduler.run (ioa::instance_generator<parameter_helper_parameter_exists> ());
  ioa::scheduler.clear ();
}

struct parameter_helper_parameter_declared :
  public ioa::dispatching_automaton
{
  int m_parameter;
  parameter_helper<int> m_helper;

  parameter_helper_parameter_declared () :
    m_helper (&m_parameter)
  {
    m_helper.declare (this);
  }

  ~parameter_helper_parameter_declared () {
    BOOST_CHECK_EQUAL (m_helper.get_state (), parameter_helper<int>::PARAMETER_DECLARED);
  }
};

BOOST_AUTO_TEST_CASE (config_parameter_helper_parameter_declared)
{
  ioa::scheduler.run (ioa::instance_generator<parameter_helper_parameter_declared> ());
  ioa::scheduler.clear ();
}

// class bind_output_automaton_dne_
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_output_automaton_dne_, &bind_output_automaton_dne_::transition_> transition;

// public:
//   void init () {
//     ioa::scheduler.schedule (this, &bind_output_automaton_dne_::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (this, &bind_output_automaton_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_output_automaton_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void bind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_exists () {
//     BOOST_CHECK (false);
//   }

//   void input_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void output_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void bound () {
//     BOOST_CHECK (false);
//   }

//   void unbound () {
//     BOOST_CHECK (false);
//   }

//   bind_output_automaton_dne_ () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_output_automaton_dne)
// {
//   bind_output_automaton_dne_* instance = new bind_output_automaton_dne_ ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_output_automaton_dne_::STOP);
//   ioa::scheduler.clear ();
// }

// class bind_input_automaton_dne_
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_input_automaton_dne_, &bind_input_automaton_dne_::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &bind_input_automaton_dne_::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (this, &bind_input_automaton_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_automaton_dne () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_input_automaton_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void bind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_exists () {
//     BOOST_CHECK (false);
//   }

//   void input_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void output_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void bound () {
//     BOOST_CHECK (false);
//   }

//   void unbound () {
//     BOOST_CHECK (false);
//   }

//   bind_input_automaton_dne_ () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_input_automaton_dne)
// {
//   bind_input_automaton_dne_* instance = new bind_input_automaton_dne_ ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_input_automaton_dne_::STOP);
//   ioa::scheduler.clear ();
// }

// class bind_output_parameter_dne_
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::parameter_handle<int> m_parameter;
//   ioa::automaton_handle<automaton2> m_child2;

//   bool output_ (int* parameter) {
//     return false;
//   }
//   ioa::void_parameter_output_wrapper<bind_output_parameter_dne_, int, &bind_output_parameter_dne_::output_> output;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this, &bind_output_parameter_dne_::output, m_parameter, m_child2, &automaton2::input);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_output_parameter_dne_, &bind_output_parameter_dne_::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &bind_output_parameter_dne_::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (this, &bind_output_parameter_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_output_parameter_dne () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_output_parameter_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void bind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_exists () {
//     BOOST_CHECK (false);
//   }

//   void input_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void output_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void bound () {
//     BOOST_CHECK (false);
//   }

//   void unbound () {
//     BOOST_CHECK (false);
//   }

//   bind_output_parameter_dne_ () :
//     m_state (START),
//     output (*this),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_output_parameter_dne_)
// {
//   bind_output_parameter_dne_* instance = new bind_output_parameter_dne_ ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_output_parameter_dne_::STOP);
//   ioa::scheduler.clear ();
// }

// class bind_input_parameter_dne_
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::parameter_handle<int> m_parameter;

//   void input_ (int* parameter) {
//   }
//   ioa::void_parameter_input_wrapper<bind_input_parameter_dne_, int, &bind_input_parameter_dne_::input_> input;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.bind (this, m_child1, &automaton2::output, &bind_input_parameter_dne_::input, m_parameter);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_input_parameter_dne_, &bind_input_parameter_dne_::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &bind_input_parameter_dne_::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (this, &bind_input_parameter_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_parameter_dne () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_input_parameter_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void binding_exists () {
//     BOOST_CHECK (false);
//   }

//   void input_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void output_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void bound () {
//     BOOST_CHECK (false);
//   }

//   void unbound () {
//     BOOST_CHECK (false);
//   }

//   bind_input_parameter_dne_ () :
//     m_state (START),
//     input (*this),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_input_parameter_dne_)
// {
//   bind_input_parameter_dne_* instance = new bind_input_parameter_dne_ ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_input_parameter_dne_::STOP);
//   ioa::scheduler.clear ();
// }

// class bind_exists
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     BIND2_SENT,
//     BIND2_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = BIND2_SENT;
//       break;
//     case BIND2_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_exists, &bind_exists::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &bind_exists::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (this, &bind_exists::transition);
//       break;
//     case CREATE_CHILD2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (this, &bind_exists::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_exists () {
//     switch (m_state) {
//     case BIND2_SENT:
//       m_state = BIND2_RECV;
//       ioa::scheduler.schedule (this, &bind_exists::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void input_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void output_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void bound () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_exists::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbound () {
//     BOOST_CHECK (false);
//   }

//   bind_exists () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_exists)
// {
//   bind_exists* instance = new bind_exists ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_exists::STOP);
//   ioa::scheduler.clear ();
// }

// class bind_input_action_unavailable
// {
// public:
//   enum state_type {
//     START,
//     CREATE_OUTPUT1_SENT,
//     CREATE_OUTPUT1_RECV,
//     CREATE_OUTPUT2_SENT,
//     CREATE_OUTPUT2_RECV,
//     CREATE_INPUT1_SENT,
//     CREATE_INPUT1_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     BIND2_SENT,
//     BIND2_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_output1;
//   ioa::automaton_handle<automaton2> m_output2;
//   ioa::automaton_handle<automaton2> m_input1;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_OUTPUT1_SENT;
//       break;
//     case CREATE_OUTPUT1_RECV:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_OUTPUT2_SENT;
//       break;
//     case CREATE_OUTPUT2_RECV:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_INPUT1_SENT;
//       break;
//     case CREATE_INPUT1_RECV:
//       ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       ioa::scheduler.bind (this, m_output2, &automaton2::output, m_input1, &automaton2::input);
//       m_state = BIND2_SENT;
//       break;
//     case BIND2_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_input_action_unavailable, &bind_input_action_unavailable::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_OUTPUT1_SENT:
//       m_output1 = automaton;
//       m_state = CREATE_OUTPUT1_RECV;
//       ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
//       break;
//     case CREATE_OUTPUT2_SENT:
//       m_output2 = automaton;
//       m_state = CREATE_OUTPUT2_RECV;
//       ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
//       break;
//     case CREATE_INPUT1_SENT:
//       m_input1 = automaton;
//       m_state = CREATE_INPUT1_RECV;
//       ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_exists () {
//     BOOST_CHECK (false);
//   }

//   void input_action_unavailable () {
//     switch (m_state) {
//     case BIND2_SENT:
//       m_state = BIND2_RECV;
//       ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void output_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void bound () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbound () {
//     BOOST_CHECK (false);
//   }

//   bind_input_action_unavailable () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_input_action_unavailable)
// {
//   bind_input_action_unavailable* instance = new bind_input_action_unavailable ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_input_action_unavailable::STOP);
//   ioa::scheduler.clear ();
// }

// class bind_output_action_unavailable
// {
// public:
//   enum state_type {
//     START,
//     CREATE_OUTPUT1_SENT,
//     CREATE_OUTPUT1_RECV,
//     CREATE_INPUT1_SENT,
//     CREATE_INPUT1_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     BIND2_SENT,
//     BIND2_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_output1;
//   ioa::automaton_handle<automaton2> m_input1;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_OUTPUT1_SENT;
//       break;
//     case CREATE_OUTPUT1_RECV:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_INPUT1_SENT;
//       break;
//     case CREATE_INPUT1_RECV:
//       ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       ioa::scheduler.bind (this, m_output1, &automaton2::output, m_input1, &automaton2::input2);
//       m_state = BIND2_SENT;
//       break;
//     case BIND2_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_output_action_unavailable, &bind_output_action_unavailable::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_OUTPUT1_SENT:
//       m_output1 = automaton;
//       m_state = CREATE_OUTPUT1_RECV;
//       ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
//       break;
//     case CREATE_INPUT1_SENT:
//       m_input1 = automaton;
//       m_state = CREATE_INPUT1_RECV;
//       ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_exists () {
//     BOOST_CHECK (false);
//   }

//   void input_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void output_action_unavailable () {
//     switch (m_state) {
//     case BIND2_SENT:
//       m_state = BIND2_RECV;
//       ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void bound () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbound () {
//     BOOST_CHECK (false);
//   }

//   bind_output_action_unavailable () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_output_action_unavailable)
// {
//   bind_output_action_unavailable* instance = new bind_output_action_unavailable ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_output_action_unavailable::STOP);
//   ioa::scheduler.clear ();
// }

// class bind_bound
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_bound, &bind_bound::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &bind_bound::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (this, &bind_bound::transition);
//       break;
//     case CREATE_CHILD2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (this, &bind_bound::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_exists () {
//     BOOST_CHECK (false);
//   }

//   void input_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void output_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void bound () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_bound::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbound () {
//     BOOST_CHECK (false);
//   }

//   bind_bound () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_bound)
// {
//   bind_bound* instance = new bind_bound ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_bound::STOP);
//   ioa::scheduler.clear ();
// }

// class unbind_output_automaton_dne_
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     UNBIND1_SENT,
//     UNBIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = UNBIND1_SENT;
//       break;
//     case UNBIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<unbind_output_automaton_dne_, &unbind_output_automaton_dne_::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &unbind_output_automaton_dne_::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (this, &unbind_output_automaton_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_automaton_dne () {
//     switch (m_state) {
//     case UNBIND1_SENT:
//       m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (this, &unbind_output_automaton_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_dne () {
//     BOOST_CHECK (false);
//   }

//   unbind_output_automaton_dne_ () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_unbind_output_automaton_dne)
// {
//   unbind_output_automaton_dne_* instance = new unbind_output_automaton_dne_ ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, unbind_output_automaton_dne_::STOP);
//   ioa::scheduler.clear ();
// }

// class unbind_input_automaton_dne_
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     UNBIND1_SENT,
//     UNBIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = UNBIND1_SENT;
//       break;
//     case UNBIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<unbind_input_automaton_dne_, &unbind_input_automaton_dne_::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &unbind_input_automaton_dne_::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (this, &unbind_input_automaton_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_automaton_dne () {
//     switch (m_state) {
//     case UNBIND1_SENT:
//       m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (this, &unbind_input_automaton_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_dne () {
//     BOOST_CHECK (false);
//   }

//   unbind_input_automaton_dne_ () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_unbind_input_automaton_dne)
// {
//   unbind_input_automaton_dne_* instance = new unbind_input_automaton_dne_ ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, unbind_input_automaton_dne_::STOP);
//   ioa::scheduler.clear ();
// }

// class unbind_output_parameter_dne_
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     UNBIND1_SENT,
//     UNBIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::parameter_handle<int> m_parameter;
//   ioa::automaton_handle<automaton2> m_child2;

//   bool output_ (int* parameter) {
//     return false;
//   }
//   ioa::void_parameter_output_wrapper<unbind_output_parameter_dne_, int, &unbind_output_parameter_dne_::output_> output;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.unbind (this, &unbind_output_parameter_dne_::output, m_parameter, m_child2, &automaton2::input);
//       m_state = UNBIND1_SENT;
//       break;
//     case UNBIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<unbind_output_parameter_dne_, &unbind_output_parameter_dne_::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &unbind_output_parameter_dne_::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (this, &unbind_output_parameter_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_parameter_dne () {
//     switch (m_state) {
//     case UNBIND1_SENT:
//       m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (this, &unbind_output_parameter_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_dne () {
//     BOOST_CHECK (false);
//   }

//   unbind_output_parameter_dne_ () :
//     m_state (START),
//     output (*this),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_unbind_output_parameter_dne_)
// {
//   unbind_output_parameter_dne_* instance = new unbind_output_parameter_dne_ ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, unbind_output_parameter_dne_::STOP);
//   ioa::scheduler.clear ();
// }

// class unbind_input_parameter_dne_
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     UNBIND1_SENT,
//     UNBIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::parameter_handle<int> m_parameter;

//   void input_ (int* parameter) {
//   }
//   ioa::void_parameter_input_wrapper<unbind_input_parameter_dne_, int, &unbind_input_parameter_dne_::input_> input;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.unbind (this, m_child1, &automaton2::output, &unbind_input_parameter_dne_::input, m_parameter);
//       m_state = UNBIND1_SENT;
//       break;
//     case UNBIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<unbind_input_parameter_dne_, &unbind_input_parameter_dne_::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &unbind_input_parameter_dne_::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (this, &unbind_input_parameter_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_parameter_dne () {
//     switch (m_state) {
//     case UNBIND1_SENT:
//       m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (this, &unbind_input_parameter_dne_::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void binding_dne () {
//     BOOST_CHECK (false);
//   }

//   unbind_input_parameter_dne_ () :
//     m_state (START),
//     input (*this),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_unbind_input_parameter_dne_)
// {
//   unbind_input_parameter_dne_* instance = new unbind_input_parameter_dne_ ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, unbind_input_parameter_dne_::STOP);
//   ioa::scheduler.clear ();
// }

// class bind_binding_dne
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     UNBIND1_SENT,
//     UNBIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = UNBIND1_SENT;
//       break;
//     case UNBIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<bind_binding_dne, &bind_binding_dne::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &bind_binding_dne::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (this, &bind_binding_dne::transition);
//       break;
//     case CREATE_CHILD2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (this, &bind_binding_dne::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_dne () {
//     switch (m_state) {
//     case UNBIND1_SENT:
//       m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (this, &bind_binding_dne::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   bind_binding_dne () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_binding_dne)
// {
//   bind_binding_dne* instance = new bind_binding_dne ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, bind_binding_dne::STOP);
//   ioa::scheduler.clear ();
// }

// class unbind_unbound
// {
// public:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     UNBIND1_SENT,
//     UNBIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       ioa::scheduler.unbind (this, m_child1, &automaton2::output, m_child2, &automaton2::input);
//       m_state = UNBIND1_SENT;
//       break;
//     case UNBIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<unbind_unbound, &unbind_unbound::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &unbind_unbound::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE_CHILD1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (this, &unbind_unbound::transition);
//       break;
//     case CREATE_CHILD2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (this, &unbind_unbound::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   void bind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void bind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_exists () {
//     BOOST_CHECK (false);
//   }

//   void input_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void output_action_unavailable () {
//     BOOST_CHECK (false);
//   }

//   void bound () {
//     switch (m_state) {
//     case BIND1_SENT:
//       m_state = BIND1_RECV;
//       ioa::scheduler.schedule (this, &unbind_unbound::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbound () {
//     switch (m_state) {
//     case UNBIND1_SENT:
//       m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (this, &unbind_unbound::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void unbind_output_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_automaton_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_output_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void unbind_input_parameter_dne () {
//     BOOST_CHECK (false);
//   }

//   void binding_dne () {
//     BOOST_CHECK (false);
//   }

//   unbind_unbound () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_unbind_unbound)
// {
//   unbind_unbound* instance = new unbind_unbound ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, unbind_unbound::STOP);
//   ioa::scheduler.clear ();
// }

// class rescind_parameter_dne
// {
// public:
//   enum state_type {
//     START,
//     RESCIND1_SENT,
//     RESCIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::parameter_handle<int> m_parameter;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.rescind (this, m_parameter);
//       m_state = RESCIND1_SENT;
//       break;
//     case RESCIND1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<rescind_parameter_dne, &rescind_parameter_dne::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &rescind_parameter_dne::transition);
//   }

//   template <class P>
//   void parameter_dne (const ioa::parameter_handle<P>&) {
//     switch (m_state) {
//     case RESCIND1_SENT:
//       m_state = RESCIND1_RECV;
//       ioa::scheduler.schedule (this, &rescind_parameter_dne::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   rescind_parameter_dne () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_rescind_parameter_dne)
// {
//   rescind_parameter_dne* instance = new rescind_parameter_dne ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, rescind_parameter_dne::STOP);
//   ioa::scheduler.clear ();
// }

// class rescind_parameter_rescinded
// {
// public:
//   enum state_type {
//     START,
//     DECLARE1_SENT,
//     DECLARE1_RECV,
//     RESCIND1_SENT,
//     RESCIND1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   int m_parameter1;
//   ioa::parameter_handle<int> m_parameter2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.declare (this, &m_parameter1);
//       m_state = DECLARE1_SENT;
//       break;
//     case DECLARE1_RECV:
//       ioa::scheduler.rescind (this, m_parameter2);
//       m_state = RESCIND1_SENT;
//       break;
//     case RESCIND1_RECV:
//       m_state = STOP;
//       break;

//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<rescind_parameter_rescinded, &rescind_parameter_rescinded::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &rescind_parameter_rescinded::transition);
//   }

//   template <class P>
//   void parameter_exists (const ioa::parameter_handle<P>&) {
//     BOOST_CHECK (false);
//   }

//   void parameter_declared (const ioa::parameter_handle<int>& parameter) {
//     switch (m_state) {
//     case DECLARE1_SENT:
//       m_parameter2 = parameter;
//       m_state = DECLARE1_RECV;
//       ioa::scheduler.schedule (this, &rescind_parameter_rescinded::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class P>
//   void parameter_dne (const ioa::parameter_handle<P>&) {
//     BOOST_CHECK (false);
//   }

//   template <class P>
//   void parameter_rescinded (const ioa::parameter_handle<P>& parameter) {
//     switch (m_state) {
//     case RESCIND1_SENT:
//       m_state = RESCIND1_RECV;
//       ioa::scheduler.schedule (this, &rescind_parameter_rescinded::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   rescind_parameter_rescinded () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_rescind_parameter_rescinded)
// {
//   rescind_parameter_rescinded* instance = new rescind_parameter_rescinded ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, rescind_parameter_rescinded::STOP);
//   ioa::scheduler.clear ();
// }

// class destroy_helper
// {
// public:
//   enum state_type {
//     START,
//     DESTROY1_SENT,
//     DESTROY1_RECV,
//     STOP,
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_automaton;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.destroy (this, m_automaton);
//       m_state = DESTROY1_SENT;
//       break;
//     case DESTROY1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<destroy_helper, &destroy_helper::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &destroy_helper::transition);
//   }

//   void destroyer_not_creator (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//      case DESTROY1_SENT:
//        BOOST_CHECK (m_automaton == automaton);
//        m_state = DESTROY1_RECV;
//        ioa::scheduler.schedule (this, &destroy_helper::transition);
//        break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void target_automaton_dne (const ioa::automaton_handle<I>& automaton) {
//     BOOST_CHECK (false);
//   }

//   destroy_helper (const ioa::automaton_handle<automaton2>& automaton) :
//     m_state (START),
//     m_automaton (automaton),
//     transition (*this)
//   { }
// };

// class destroy_destroyer_not_creator
// {
// public:
//   enum state_type {
//     START,
//     CREATE1_SENT,
//     CREATE1_RECV,
//     CREATE2_SENT,
//     CREATE2_RECV,
//     STOP
//   };
//   state_type m_state;
//   destroy_helper* m_instance;

// private:
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<destroy_helper> m_child2;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE1_SENT;
//       break;
//     case CREATE1_RECV:
//       m_instance = new destroy_helper (m_child1);
//       ioa::scheduler.create (this, m_instance);
//       m_state = CREATE2_SENT;
//       break;
//     case CREATE2_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<destroy_destroyer_not_creator, &destroy_destroyer_not_creator::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &destroy_destroyer_not_creator::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE1_SENT:
//       m_child1 = automaton;
//       m_state = CREATE1_RECV;
//       ioa::scheduler.schedule (this, &destroy_destroyer_not_creator::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void automaton_created (const ioa::automaton_handle<destroy_helper>& automaton) {
//     switch (m_state) {
//     case CREATE2_SENT:
//       m_child2 = automaton;
//       m_state = CREATE2_RECV;
//       ioa::scheduler.schedule (this, &destroy_destroyer_not_creator::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void automaton_destroyed (const ioa::automaton_handle<I>&) {
//     BOOST_CHECK (false);
//   }

//   destroy_destroyer_not_creator () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_destroy_destroyer_not_creator)
// {
//   destroy_destroyer_not_creator* instance = new destroy_destroyer_not_creator ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, destroy_destroyer_not_creator::STOP);
//   BOOST_CHECK_EQUAL (instance->m_instance->m_state, destroy_helper::STOP);
//   ioa::scheduler.clear ();
// }

// class destroy_target_automaton_dne
// {
// public:
//   enum state_type {
//     START,
//     DESTROY1_SENT,
//     DESTROY1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.destroy (this, m_child);
//       m_state = DESTROY1_SENT;
//       break;
//     case DESTROY1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<destroy_target_automaton_dne, &destroy_target_automaton_dne::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &destroy_target_automaton_dne::transition);
//   }

//   void target_automaton_dne (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case DESTROY1_SENT:
//       BOOST_CHECK (m_child == automaton);
//       m_state = DESTROY1_RECV;
//       ioa::scheduler.schedule (this, &destroy_target_automaton_dne::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   template <class I>
//   void destroyer_not_creator (const ioa::automaton_handle<I>& automaton) {
//     BOOST_CHECK (false);
//   }

//   destroy_target_automaton_dne () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_destroy_target_automaton_dne)
// {
//   destroy_target_automaton_dne* instance = new destroy_target_automaton_dne ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, destroy_target_automaton_dne::STOP); // 
//   ioa::scheduler.clear ();
// }

// class destroy_automaton_destroyed
// {
// public:
//   enum state_type {
//     START,
//     CREATE1_SENT,
//     CREATE1_RECV,
//     DESTROY1_SENT,
//     DESTROY1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:
//   ioa::automaton_handle<automaton2> m_child;

//   void transition_ () {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, new automaton2 ());
//       m_state = CREATE1_SENT;
//       break;
//     case CREATE1_RECV:
//       ioa::scheduler.destroy (this, m_child);
//       m_state = DESTROY1_SENT;
//       break;
//     case DESTROY1_RECV:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }
//   ioa::internal_wrapper<destroy_automaton_destroyed, &destroy_automaton_destroyed::transition_> transition;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &destroy_automaton_destroyed::transition);
//   }

//   template <class I>
//   void instance_exists (const I*) {
//     BOOST_CHECK (false);
//   }

//   void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case CREATE1_SENT:
//       m_child = automaton;
//       m_state = CREATE1_RECV;
//       ioa::scheduler.schedule (this, &destroy_automaton_destroyed::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void automaton_destroyed (const ioa::automaton_handle<automaton2>& automaton) {
//     switch (m_state) {
//     case DESTROY1_SENT:
//       BOOST_CHECK (m_child == automaton);
//       m_state = DESTROY1_RECV;
//       ioa::scheduler.schedule (this, &destroy_automaton_destroyed::transition);
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//   }

//   void target_automaton_dne (const ioa::automaton_handle<automaton2>& automaton) {
//     BOOST_CHECK (false);
//   }

//   template <class I>
//   void destroyer_not_creator (const ioa::automaton_handle<I>& automaton) {
//     BOOST_CHECK (false);
//   }

//   destroy_automaton_destroyed () :
//     m_state (START),
//     transition (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_destroy_automaton_destroyed)
// {
//   destroy_automaton_destroyed* instance = new destroy_automaton_destroyed ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, destroy_automaton_destroyed::STOP); // 
//   ioa::scheduler.clear ();
// }

// class schedule_output
// {
// public:
//   enum state_type {
//     START,
//     STOP
//   };
//   state_type m_state;

// private:

//   bool output_ () {
//     switch (m_state) {
//     case START:
//       m_state = STOP;
//       break;
//     default:
//       BOOST_CHECK (false);
//       break;
//     }
//     return false;
//   }
//   ioa::void_output_wrapper<schedule_output, &schedule_output::output_> output;

// public:

//   void init () {
//     ioa::scheduler.schedule (this, &schedule_output::output);
//   }

//   schedule_output () :
//     m_state (START),
//     output (*this)
//   { }
// };

// BOOST_AUTO_TEST_CASE (scheduler_schedule_output)
// {
//   schedule_output* instance = new schedule_output ();
//   ioa::scheduler.run (instance);
//   BOOST_CHECK_EQUAL (instance->m_state, schedule_output::STOP);
//   ioa::scheduler.clear ();
// }

BOOST_AUTO_TEST_SUITE_END()
