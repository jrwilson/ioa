#include "minunit.h"

#include <ioa/simple_scheduler.hpp>
#include <ioa/generator.hpp>
#include "automaton2.hpp"

#include "instance_holder.hpp"

// class create_exists :
//   public ioa::dispatching_automaton
// {
// private:
//   enum state_type {
//     START,
//     CREATE1_SENT,
//     CREATE1_RECV,
//     CREATE2_SENT,
//     CREATE2_RECV,
//     STOP
//   };
//   state_type m_state;

//   struct create1_d
//   {
//     create_exists& m_ce;

//     create1_d (create_exists& ce) :
//       m_ce (ce)
//     { }

//     template <class I>
//     void automaton_created (const ioa::automaton_handle<I>&) {
//       m_ce.m_state = CREATE1_RECV;
//       ioa::scheduler.schedule (&m_ce, &create_exists::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create2_d
//   {
//     create_exists& m_ce;

//     create2_d (create_exists& ce) :
//       m_ce (ce)
//     { }

//     template <class I>
//     void automaton_created (const ioa::automaton_handle<I>&) {
//       BOOST_CHECK (false);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       m_ce.m_state = CREATE2_RECV;
//       ioa::scheduler.schedule (&m_ce, &create_exists::transition);
//     }

//     void automaton_destroyed () {
//       BOOST_CHECK (false);
//     }
//   };

//   create1_d m_create1_d;
//   create2_d m_create2_d;
//   automaton2* m_instance;

//   UP_INTERNAL (create_exists, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, std::auto_ptr<ioa::generator_interface<automaton2> > (new instance_holder<automaton2> (m_instance)), m_create1_d);
//       m_state = CREATE1_SENT;
//       break;
//     case CREATE1_RECV:
//       ioa::scheduler.create (this, std::auto_ptr<ioa::generator_interface<automaton2> > (new instance_holder<automaton2> (m_instance)), m_create2_d);
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

// public:
//   create_exists () :
//     m_state (START),
//     m_create1_d (*this),
//     m_create2_d (*this),
//     m_instance (new automaton2 ()),
//     ACTION (create_exists, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &create_exists::transition);
//   }
  
//   ~create_exists () {
//     BOOST_CHECK_EQUAL (m_state, STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_create_exists)
// {
//   ioa::scheduler.run (ioa::make_instance_generator<create_exists> ());
//   ioa::scheduler.clear ();
// }

class create_automaton_created :
  public ioa::automaton_interface
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
  UP_INTERNAL (create_automaton_created, transition) {
    switch (m_state) {
    case START:
      ioa::scheduler::create (this, ioa::make_generator<automaton2> (), 0);
      m_state = CREATE1_SENT;
      break;
    case CREATE1_RECV:
      m_state = STOP;
      break;
    default:
      //BOOST_CHECK (false);
      break;
    }
  }

public:

  create_automaton_created () :
    m_state (START),
    ACTION (create_automaton_created, transition)
  { }

  void init () {
    assert (false);
    // ioa::scheduler::schedule (this, &create_automaton_created::transition);
  }
};

static const char*
automaton_created ()
{
  ioa::scheduler::clear ();

  create_automaton_created* instance = new create_automaton_created ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<create_automaton_created> (instance));
  ioa::scheduler::run (holder);
  mu_assert (instance->m_state == create_automaton_created::STOP);

  return 0;
}

// class bind_output_automaton_dne_ :
//   public ioa::dispatching_automaton
// {
// private:
//   enum state_type {
//     START,
//     CREATE_CHILD2_SENT,
//     CREATE_CHILD2_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     STOP
//   };
//   state_type m_state;

//   struct create1_d
//   {
//     bind_output_automaton_dne_& m_ce;

//     create1_d (bind_output_automaton_dne_& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child2 = automaton;
//       m_ce.m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_output_automaton_dne_::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct bind1_d
//   {
//     bind_output_automaton_dne_& m_ce;

//     bind1_d (bind_output_automaton_dne_& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       m_ce.m_state = BIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_output_automaton_dne_::transition);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }
    
//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t) {
//       BOOST_CHECK (false);
//     }

//     void unbound () {
//       BOOST_CHECK (false);
//     }

//   };

//   create1_d m_create1_d;
//   bind1_d m_bind1_d;
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   UP_INTERNAL (bind_output_automaton_dne_, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_child1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_child2, &automaton2::uv_up_input),
// 			   m_bind1_d);
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

// public:
//   bind_output_automaton_dne_ () :
//     m_state (START),
//     m_create1_d (*this),
//     m_bind1_d (*this),
//     ACTION (bind_output_automaton_dne_, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &bind_output_automaton_dne_::transition);
//   }

//   ~bind_output_automaton_dne_ () {
//     BOOST_CHECK_EQUAL (m_state, bind_output_automaton_dne_::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_output_automaton_dne)
// {
//   ioa::scheduler.run (ioa::make_generator<bind_output_automaton_dne_> ());
//   ioa::scheduler.clear ();
// }

// class bind_input_automaton_dne_ :
//   public ioa::dispatching_automaton
// {
// private:
//   enum state_type {
//     START,
//     CREATE_CHILD1_SENT,
//     CREATE_CHILD1_RECV,
//     BIND1_SENT,
//     BIND1_RECV,
//     STOP
//   };
//   state_type m_state;

//   struct create1_d
//   {
//     bind_input_automaton_dne_& m_ce;

//     create1_d (bind_input_automaton_dne_& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child1 = automaton;
//       m_ce.m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_input_automaton_dne_::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct bind1_d
//   {
//     bind_input_automaton_dne_& m_ce;

//     bind1_d (bind_input_automaton_dne_& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       m_ce.m_state = BIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_input_automaton_dne_::transition);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }
    
//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t) {
//       BOOST_CHECK (false);
//     }

//     void unbound () {
//       BOOST_CHECK (false);
//     }

//   };

//   create1_d m_create1_d;
//   bind1_d m_bind1_d;
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   UP_INTERNAL (bind_input_automaton_dne_, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_child1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_child2, &automaton2::uv_up_input),
// 			   m_bind1_d);
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

// public:
//   bind_input_automaton_dne_ () :
//     m_state (START),
//     m_create1_d (*this),
//     m_bind1_d (*this),
//     ACTION (bind_input_automaton_dne_, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &bind_input_automaton_dne_::transition);
//   }

//   ~bind_input_automaton_dne_ () {
//     BOOST_CHECK_EQUAL (m_state, bind_input_automaton_dne_::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_input_automaton_dne)
// {
//   ioa::scheduler.run (ioa::make_generator<bind_input_automaton_dne_> ());
//   ioa::scheduler.clear ();
// }

// class bind_exists :
//   public ioa::dispatching_automaton
// {
// private:
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

//   struct create1_d
//   {
//     bind_exists& m_ce;
    
//     create1_d (bind_exists& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child1 = automaton;
//       m_ce.m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_exists::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create2_d
//   {
//     bind_exists& m_ce;
    
//     create2_d (bind_exists& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child2 = automaton;
//       m_ce.m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_exists::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct bind1_d
//   {
//     bind_exists& m_ce;

//     bind1_d (bind_exists& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t) {
//       m_ce.m_state = BIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_exists::transition);
//     }

//     void unbound () {
//       // Okay.
//     }
    
//   };

//   struct bind2_d
//   {
//     bind_exists& m_ce;

//     bind2_d (bind_exists& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       m_ce.m_state = BIND2_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_exists::transition);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t) {
//       BOOST_CHECK (false);
//     }

//     void unbound () {
//       BOOST_CHECK (false);
//     }

//   };

//   create1_d m_create1_d;
//   create2_d m_create2_d;
//   bind1_d m_bind1_d;
//   bind2_d m_bind2_d;

//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   UP_INTERNAL (bind_exists, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create2_d);
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_child1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_child2, &automaton2::uv_up_input),
// 			   m_bind1_d);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_child1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_child2, &automaton2::uv_up_input),
// 			   m_bind2_d);
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

// public:
//   bind_exists () :
//     m_state (START),
//     m_create1_d (*this),
//     m_create2_d (*this),
//     m_bind1_d (*this),
//     m_bind2_d (*this),
//     ACTION (bind_exists, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &bind_exists::transition);
//   }

//   ~bind_exists () {
//     BOOST_CHECK_EQUAL (m_state, bind_exists::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_exists)
// {
//   ioa::scheduler.run (ioa::make_generator<bind_exists> ());
//   ioa::scheduler.clear ();
// }

// class bind_input_action_unavailable :
//   public ioa::dispatching_automaton
// {
// private:
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

//   struct create1_d
//   {
//     bind_input_action_unavailable& m_ce;
    
//     create1_d (bind_input_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_output1 = automaton;
//       m_ce.m_state = CREATE_OUTPUT1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create2_d
//   {
//     bind_input_action_unavailable& m_ce;
    
//     create2_d (bind_input_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_output2 = automaton;
//       m_ce.m_state = CREATE_OUTPUT2_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create3_d
//   {
//     bind_input_action_unavailable& m_ce;
    
//     create3_d (bind_input_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_input1 = automaton;
//       m_ce.m_state = CREATE_INPUT1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct bind1_d
//   {
//     bind_input_action_unavailable& m_ce;

//     bind1_d (bind_input_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t) {
//       m_ce.m_state = BIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
//     }

//     void unbound () {
//       // Okay.
//     }

//   };

//   struct bind2_d
//   {
//     bind_input_action_unavailable& m_ce;

//     bind2_d (bind_input_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       m_ce.m_state = BIND2_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_input_action_unavailable::transition);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t) {
//       BOOST_CHECK (false);
//     }

//     void unbound () {
//       BOOST_CHECK (false);
//     }

//   };

//   create1_d m_create1_d;
//   create2_d m_create2_d;
//   create3_d m_create3_d;
//   bind1_d m_bind1_d;
//   bind2_d m_bind2_d;
//   ioa::automaton_handle<automaton2> m_output1;
//   ioa::automaton_handle<automaton2> m_output2;
//   ioa::automaton_handle<automaton2> m_input1;

//   UP_INTERNAL (bind_input_action_unavailable, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE_OUTPUT1_SENT;
//       break;
//     case CREATE_OUTPUT1_RECV:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create2_d);
//       m_state = CREATE_OUTPUT2_SENT;
//       break;
//     case CREATE_OUTPUT2_RECV:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create3_d);
//       m_state = CREATE_INPUT1_SENT;
//       break;
//     case CREATE_INPUT1_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_output1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_input1, &automaton2::uv_up_input),
// 			   m_bind1_d);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_output2, &automaton2::uv_up_output),
// 			   ioa::make_action (m_input1, &automaton2::uv_up_input),
// 			   m_bind2_d);
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

// public:
//   bind_input_action_unavailable () :
//     m_state (START),
//     m_create1_d (*this),
//     m_create2_d (*this),
//     m_create3_d (*this),
//     m_bind1_d (*this),
//     m_bind2_d (*this),
//     ACTION (bind_input_action_unavailable, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &bind_input_action_unavailable::transition);
//   }

//   ~bind_input_action_unavailable () {
//     BOOST_CHECK_EQUAL (m_state, bind_input_action_unavailable::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_input_action_unavailable)
// {
//   ioa::scheduler.run (ioa::make_generator<bind_input_action_unavailable> ());
//   ioa::scheduler.clear ();
// }

// class bind_output_action_unavailable :
//   public ioa::dispatching_automaton
// {
// private:
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

//   struct create1_d
//   {
//     bind_output_action_unavailable& m_ce;
    
//     create1_d (bind_output_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_output1 = automaton;
//       m_ce.m_state = CREATE_OUTPUT1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_output_action_unavailable::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create2_d
//   {
//     bind_output_action_unavailable& m_ce;
    
//     create2_d (bind_output_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_input1 = automaton;
//       m_ce.m_state = CREATE_INPUT1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_output_action_unavailable::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct bind1_d
//   {
//     bind_output_action_unavailable& m_ce;

//     bind1_d (bind_output_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t) {
//       m_ce.m_state = BIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_output_action_unavailable::transition);
//     }

//     void unbound () {
//       // Okay.
//     }

//   };

//   struct bind2_d
//   {
//     bind_output_action_unavailable& m_ce;

//     bind2_d (bind_output_action_unavailable& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       m_ce.m_state = BIND2_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_output_action_unavailable::transition);
//     }

//     void bound (const ioa::bid_t) {
//       BOOST_CHECK (false);
//     }

//     void unbound () {
//       BOOST_CHECK (false);
//     }

//   };

//   create1_d m_create1_d;
//   create2_d m_create2_d;
//   bind1_d m_bind1_d;
//   bind2_d m_bind2_d;
//   ioa::automaton_handle<automaton2> m_output1;
//   ioa::automaton_handle<automaton2> m_input1;

//   UP_INTERNAL (bind_output_action_unavailable, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE_OUTPUT1_SENT;
//       break;
//     case CREATE_OUTPUT1_RECV:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create2_d);
//       m_state = CREATE_INPUT1_SENT;
//       break;
//     case CREATE_INPUT1_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_output1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_input1, &automaton2::uv_up_input),
// 			   m_bind1_d);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_output1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_input1, &automaton2::uv_p_input, 1.9f),
// 			   m_bind2_d);
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

// public:
//   bind_output_action_unavailable () :
//     m_state (START),
//     m_create1_d (*this),
//     m_create2_d (*this),
//     m_bind1_d (*this),
//     m_bind2_d (*this),
//     ACTION (bind_output_action_unavailable, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &bind_output_action_unavailable::transition);
//   }

//   ~bind_output_action_unavailable () {
//     BOOST_CHECK_EQUAL (m_state, bind_output_action_unavailable::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_output_action_unavailable)
// {
//   ioa::scheduler.run (ioa::make_generator<bind_output_action_unavailable> ());
//   ioa::scheduler.clear ();
// }

// class bind_bound :
//   public ioa::dispatching_automaton
// {
// private:
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

//   struct create1_d
//   {
//     bind_bound& m_ce;
    
//     create1_d (bind_bound& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child1 = automaton;
//       m_ce.m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_bound::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create2_d
//   {
//     bind_bound& m_ce;
    
//     create2_d (bind_bound& ce) :
//       m_ce (ce)
//     { }
    
//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child2 = automaton;
//       m_ce.m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_bound::transition);
//     }
    
//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }
    
//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct bind1_d
//   {
//     bind_bound& m_ce;

//     bind1_d (bind_bound& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t) {
//       m_ce.m_state = BIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &bind_bound::transition);
//     }

//     void unbound () {
//       // Okay.
//     }

//   };

//   create1_d m_create1_d;
//   create2_d m_create2_d;
//   bind1_d m_bind1_d;
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;

//   UP_INTERNAL (bind_bound, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create2_d);
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_child1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_child2, &automaton2::uv_up_input),
// 			   m_bind1_d);
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

// public:
//   bind_bound () :
//     m_state (START),
//     m_create1_d (*this),
//     m_create2_d (*this),
//     m_bind1_d (*this),
//     ACTION (bind_bound, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &bind_bound::transition);
//   }

//   ~bind_bound () {
//     BOOST_CHECK_EQUAL (m_state, bind_bound::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_bind_bound)
// {
//   ioa::scheduler.run (ioa::make_generator<bind_bound> ());
//   ioa::scheduler.clear ();
// }

// class unbind_binding_dne :
//   public ioa::dispatching_automaton
// {
// private:
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

//   struct create1_d
//   {
//     unbind_binding_dne& m_ce;

//     create1_d (unbind_binding_dne& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child1 = automaton;
//       m_ce.m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (&m_ce, &unbind_binding_dne::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create2_d
//   {
//     unbind_binding_dne& m_ce;

//     create2_d (unbind_binding_dne& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child2 = automaton;
//       m_ce.m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (&m_ce, &unbind_binding_dne::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct unbind1_d
//   {
//     unbind_binding_dne& m_ce;

//     unbind1_d (unbind_binding_dne& ce) :
//       m_ce (ce)
//     { }
    
//     void unbind_output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void unbind_input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void unbind_output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void unbind_input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_dne () {
//       m_ce.m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &unbind_binding_dne::transition);
//     }
    
//   };

//   create1_d m_create1_d;
//   create2_d m_create2_d;
//   unbind1_d m_unbind1_d;
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;
//   ioa::bid_t m_bid;

//   UP_INTERNAL (unbind_binding_dne, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create2_d);
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.unbind (this, m_bid, m_unbind1_d);
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

// public:
//   unbind_binding_dne () :
//     m_state (START),
//     m_create1_d (*this),
//     m_create2_d (*this),
//     m_unbind1_d (*this),
//     ACTION (unbind_binding_dne, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &unbind_binding_dne::transition);
//   }

//   ~unbind_binding_dne () {
//     BOOST_CHECK_EQUAL (m_state, unbind_binding_dne::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_unbind_binding_dne)
// {
//   ioa::scheduler.run (ioa::make_generator<unbind_binding_dne> ());
//   ioa::scheduler.clear ();
// }

// class unbind_unbound :
//   public ioa::dispatching_automaton
// {
// private:
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

//   struct create1_d
//   {
//     unbind_unbound& m_ce;

//     create1_d (unbind_unbound& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child1 = automaton;
//       m_ce.m_state = CREATE_CHILD1_RECV;
//       ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create2_d
//   {
//     unbind_unbound& m_ce;

//     create2_d (unbind_unbound& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child2 = automaton;
//       m_ce.m_state = CREATE_CHILD2_RECV;
//       ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct bind1_d
//   {
//     unbind_unbound& m_ce;

//     bind1_d (unbind_unbound& ce) :
//       m_ce (ce)
//     { }
    
//     void output_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void output_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void input_parameter_dne () {
//       BOOST_CHECK (false);
//     }

//     void binding_exists () {
//       BOOST_CHECK (false);
//     }

//     void input_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void output_action_unavailable () {
//       BOOST_CHECK (false);
//     }

//     void bound (const ioa::bid_t bid) {
//       m_ce.m_bid = bid;
//       m_ce.m_state = BIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
//     }

//     void unbound () {
//       m_ce.m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
//     }

//   };

//   struct unbind1_d
//   {
//     unbind_unbound& m_ce;

//     unbind1_d (unbind_unbound& ce) :
//       m_ce (ce)
//     { }
    
//     void binding_dne () {
//       m_ce.m_state = UNBIND1_RECV;
//       ioa::scheduler.schedule (&m_ce, &unbind_unbound::transition);
//     }
    
//   };

//   create1_d m_create1_d;
//   create2_d m_create2_d;
//   bind1_d m_bind1_d;
//   unbind1_d m_unbind1_d;
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<automaton2> m_child2;
//   ioa::bid_t m_bid;

//   UP_INTERNAL (unbind_unbound, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE_CHILD1_SENT;
//       break;
//     case CREATE_CHILD1_RECV:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create2_d);
//       m_state = CREATE_CHILD2_SENT;
//       break;
//     case CREATE_CHILD2_RECV:
//       ioa::scheduler.bind (this,
// 			   ioa::make_action (m_child1, &automaton2::uv_up_output),
// 			   ioa::make_action (m_child2, &automaton2::uv_up_input),
// 			   m_bind1_d);
//       m_state = BIND1_SENT;
//       break;
//     case BIND1_RECV:
//       ioa::scheduler.unbind (this, m_bid, m_unbind1_d);
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

// public:
//   unbind_unbound () :
//     m_state (START),
//     m_create1_d (*this),
//     m_create2_d (*this),
//     m_bind1_d (*this),
//     m_unbind1_d (*this),
//     ACTION (unbind_unbound, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &unbind_unbound::transition);
//   }

//   ~unbind_unbound () {
//     BOOST_CHECK_EQUAL (m_state, unbind_unbound::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_unbind_unbound)
// {
//   ioa::scheduler.run (ioa::make_generator <unbind_unbound> ());
//   ioa::scheduler.clear ();
// }

// class destroy_helper :
//   public ioa::dispatching_automaton
// {
// private:
//   enum state_type {
//     START,
//     DESTROY1_SENT,
//     DESTROY1_RECV,
//     STOP,
//   };
//   state_type m_state;

// private:

//   struct destroy1_d
//   {
    
//     destroy_helper& m_ce;

//     destroy1_d (destroy_helper& ce) :
//       m_ce (ce)
//     { }

//     void target_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void destroyer_not_creator () {
//       m_ce.m_state = DESTROY1_RECV;
//       ioa::scheduler.schedule (&m_ce, &destroy_helper::transition);
//     }

//   };

//   destroy1_d m_destroy1_d;
//   ioa::automaton_handle<automaton2> m_automaton;

//   UP_INTERNAL (destroy_helper, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.destroy (this, m_automaton, m_destroy1_d);
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

// public:
//   destroy_helper (const ioa::automaton_handle<automaton2>& automaton) :
//     m_state (START),
//     m_destroy1_d (*this),
//     m_automaton (automaton),
//     ACTION (destroy_helper, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &destroy_helper::transition);
//   }

//   ~destroy_helper () {
//     BOOST_CHECK_EQUAL (m_state, destroy_helper::STOP);
//   }
// };

// class destroy_destroyer_not_creator :
//   public ioa::dispatching_automaton
// {
// private:
//   enum state_type {
//     START,
//     CREATE1_SENT,
//     CREATE1_RECV,
//     CREATE2_SENT,
//     CREATE2_RECV,
//     STOP
//   };
//   state_type m_state;

// private:

//   struct create1_d
//   {
//     destroy_destroyer_not_creator& m_ce;

//     create1_d (destroy_destroyer_not_creator& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child1 = automaton;
//       m_ce.m_state = CREATE1_RECV;
//       ioa::scheduler.schedule (&m_ce, &destroy_destroyer_not_creator::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   struct create2_d
//   {
//     destroy_destroyer_not_creator& m_ce;

//     create2_d (destroy_destroyer_not_creator& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<destroy_helper>& automaton) {
//       m_ce.m_child2 = automaton;
//       m_ce.m_state = CREATE2_RECV;
//       ioa::scheduler.schedule (&m_ce, &destroy_destroyer_not_creator::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       // Okay.
//     }
//   };

//   create1_d m_create1_d;
//   create2_d m_create2_d;
//   ioa::automaton_handle<automaton2> m_child1;
//   ioa::automaton_handle<destroy_helper> m_child2;
  
//   UP_INTERNAL (destroy_destroyer_not_creator, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE1_SENT;
//       break;
//     case CREATE1_RECV:
//       ioa::scheduler.create (this, ioa::make_generator<destroy_helper, ioa::automaton_handle<automaton2> > (m_child1), m_create2_d);
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

// public:
//   destroy_destroyer_not_creator () :
//     m_state (START),
//     m_create1_d (*this),
//     m_create2_d (*this),
//     ACTION (destroy_destroyer_not_creator, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &destroy_destroyer_not_creator::transition);
//   }

//   ~destroy_destroyer_not_creator () {
//     BOOST_CHECK_EQUAL (m_state, destroy_destroyer_not_creator::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_destroy_destroyer_not_creator)
// {
//   ioa::scheduler.run (ioa::make_generator <destroy_destroyer_not_creator> ());
//   ioa::scheduler.clear ();
// }

// class destroy_target_automaton_dne :
//   public ioa::dispatching_automaton
// {
// private:
//   enum state_type {
//     START,
//     DESTROY1_SENT,
//     DESTROY1_RECV,
//     STOP
//   };
//   state_type m_state;

// private:

//   struct destroy1_d
//   {
    
//     destroy_target_automaton_dne& m_ce;

//     destroy1_d (destroy_target_automaton_dne& ce) :
//       m_ce (ce)
//     { }

//     void target_automaton_dne () {
//       m_ce.m_state = DESTROY1_RECV;
//       ioa::scheduler.schedule (&m_ce, &destroy_target_automaton_dne::transition);
//     }

//     void destroyer_not_creator () {
//       BOOST_CHECK (false);
//     }

//   };
  
//   destroy1_d m_destroy1_d;
//   ioa::automaton_handle<automaton2> m_child;
  
//   UP_INTERNAL (destroy_target_automaton_dne, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.destroy (this, m_child, m_destroy1_d);
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

// public:
//   destroy_target_automaton_dne () :
//     m_state (START),
//     m_destroy1_d (*this),
//     ACTION (destroy_target_automaton_dne, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &destroy_target_automaton_dne::transition);
//   }

//   ~destroy_target_automaton_dne () {
//     BOOST_CHECK_EQUAL (m_state, destroy_target_automaton_dne::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_destroy_target_automaton_dne)
// {
//   ioa::scheduler.run (ioa::make_generator<destroy_target_automaton_dne> ());
//   ioa::scheduler.clear ();
// }

// class destroy_automaton_destroyed :
//   public ioa::dispatching_automaton
// {
// private:
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

//   struct create1_d
//   {
//     destroy_automaton_destroyed& m_ce;

//     create1_d (destroy_automaton_destroyed& ce) :
//       m_ce (ce)
//     { }

//     void automaton_created (const ioa::automaton_handle<automaton2>& automaton) {
//       m_ce.m_child = automaton;
//       m_ce.m_state = CREATE1_RECV;
//       ioa::scheduler.schedule (&m_ce, &destroy_automaton_destroyed::transition);
//     }

//     template <class I>
//     void instance_exists (const I*) {
//       BOOST_CHECK (false);
//     }

//     void automaton_destroyed () {
//       m_ce.m_state = DESTROY1_RECV;
//       ioa::scheduler.schedule (&m_ce, &destroy_automaton_destroyed::transition);
//     }
//   };

//   struct destroy1_d
//   {

//     destroy_automaton_destroyed& m_ce;

//     destroy1_d (destroy_automaton_destroyed& ce) :
//       m_ce (ce)
//     { }

//     void target_automaton_dne () {
//       BOOST_CHECK (false);
//     }

//     void destroyer_not_creator () {
//       BOOST_CHECK (false);
//     }

//   };

//   create1_d m_create1_d;
//   destroy1_d m_destroy1_d;
//   ioa::automaton_handle<automaton2> m_child;

//   UP_INTERNAL (destroy_automaton_destroyed, transition) {
//     switch (m_state) {
//     case START:
//       ioa::scheduler.create (this, ioa::make_generator<automaton2> (), m_create1_d);
//       m_state = CREATE1_SENT;
//       break;
//     case CREATE1_RECV:
//       ioa::scheduler.destroy (this, m_child, m_destroy1_d);
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

// public:
//   destroy_automaton_destroyed () :
//     m_state (START),
//     m_create1_d (*this),
//     m_destroy1_d (*this),
//     ACTION (destroy_automaton_destroyed, transition)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &destroy_automaton_destroyed::transition);
//   }

//   ~destroy_automaton_destroyed () {
//     BOOST_CHECK_EQUAL (m_state, destroy_automaton_destroyed::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_destroy_automaton_destroyed)
// {
//   ioa::scheduler.run (ioa::make_generator<destroy_automaton_destroyed> ());
//   ioa::scheduler.clear ();
// }

// class schedule_output :
//   public ioa::dispatching_automaton
// {
// private:
//   enum state_type {
//     START,
//     STOP
//   };
//   state_type m_state;

// private:

//   UV_UP_OUTPUT (schedule_output, output) {
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

// public:
//   schedule_output () :
//     m_state (START),
//     ACTION (schedule_output, output)
//   { }

//   void init () {
//     ioa::scheduler.schedule (this, &schedule_output::output);
//   }

//   ~schedule_output () {
//     BOOST_CHECK_EQUAL (m_state, schedule_output::STOP);
//   }
// };

// BOOST_AUTO_TEST_CASE (scheduler_schedule_output)
// {
//   ioa::scheduler.run (ioa::make_generator<schedule_output> ());
//   ioa::scheduler.clear ();
// }

// // No to test internal because we've already used it.

// BOOST_AUTO_TEST_SUITE_END()

const char*
all_tests ()
{
  // mu_run_test (creator_dne);
  // mu_run_test (instance_exists);
  mu_run_test (automaton_created);
  // mu_run_test (binder_dne);
  // mu_run_test (output_automaton_dne);
  // mu_run_test (input_automaton_dne);
  // mu_run_test (binding_exists);
  // mu_run_test (input_action_unavailable);
  // mu_run_test (output_action_unavailable);
  // mu_run_test (bound);
  // mu_run_test (unbinder_dne);
  // mu_run_test (binding_dne);
  // mu_run_test (unbound);
  // mu_run_test (destroyer_dne);
  // mu_run_test (destroyer_not_creator);
  // mu_run_test (target_automaton_dne);
  // mu_run_test (automaton_destroyed);
  // mu_run_test (execute_output_automaton_dne);
  // mu_run_test (execute_output);
  // mu_run_test (execute_event_automaton_dne);
  // mu_run_test (recipient_dne);
  // mu_run_test (execute_event);

  return 0;
}
