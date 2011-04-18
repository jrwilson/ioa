#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>
#include <ioa.hpp>
#include "automaton.hpp"

BOOST_AUTO_TEST_SUITE(system_suite)

BOOST_AUTO_TEST_CASE(get_root)
{
  ioa::system system;
  BOOST_CHECK (system.root_handle.valid());
}

struct create_created {
  bool m_created;

  struct create1_callback {
    create_created& m_create;
    create1_callback(create_created& create)
      : m_create(create) { }

    void operator() (const ioa::create_result<int>& r) {
      BOOST_CHECK (r.type == ioa::AUTOMATON_CREATED);
      BOOST_CHECK (r.handle.valid ());
      m_create.m_created = true;
    }

  };
  create1_callback cb1;

  create_created ()
    : m_created(false),
      cb1(*this) { }
};

BOOST_AUTO_TEST_CASE(automaton_created)
{
  ioa::system system;
  create_created c;
  system.create(system.root_handle, new int(), c.cb1);
  BOOST_CHECK (c.m_created);
}

struct create_exists {
  int* m_x;
  bool m_created;
  bool m_exists;

  struct create1_callback {
    create_exists& m_create;
    create1_callback(create_exists& create)
      : m_create(create) { }

    void operator() (const ioa::create_result<int>& r) {
      BOOST_CHECK (r.type == ioa::AUTOMATON_CREATED);
      m_create.m_created = true;
    }

  };
  create1_callback cb1;

  struct create2_callback {
    create_exists& m_create;
    create2_callback(create_exists& create)
      : m_create(create) { }

    void operator() (const ioa::create_result<int>& r) {
      BOOST_CHECK (r.type == ioa::AUTOMATON_EXISTS);
      BOOST_CHECK (!r.handle.valid ());
      m_create.m_exists = true;
    }
  };
  create2_callback cb2;

  create_exists(int* x)
    : m_x(x),
      m_created(false),
      m_exists(false),
      cb1(*this),
      cb2(*this) { }
};

BOOST_AUTO_TEST_CASE(automaton_exists)
{
  ioa::system* system = new ioa::system();
  int* x = new int();
  create_exists c(x);
  system->create(system->root_handle, x, c.cb1);
  system->create(system->root_handle, x, c.cb2);
  BOOST_CHECK (c.m_created);
  BOOST_CHECK (c.m_exists);
  delete system;
}

class declare_declared {
public:
  bool declared;
  int param;
  ioa::automaton_handle<int> handle;

  struct create_callback {
    declare_declared& m_declare;
    create_callback(declare_declared& declare)
      : m_declare(declare) { }

    void operator() (const ioa::create_result<int>& r) {
      m_declare.handle = r.handle;
    }
  };
  create_callback cb1;

  struct declare_callback {
    declare_declared& m_declare;
    declare_callback(declare_declared& declare)
      : m_declare(declare) { }

    void operator() (const ioa::declare_result<int, int>& r) {
      BOOST_CHECK (r.type == ioa::PARAMETER_DECLARED);
      BOOST_CHECK (r.handle.valid ());
      BOOST_CHECK_EQUAL (r.handle.get_parameter (), &m_declare.param);
      m_declare.declared = true;
    }
  };
  declare_callback cb2;

  declare_declared () :
    declared (false),
    cb1 (*this),
    cb2 (*this)
  { }
};

BOOST_AUTO_TEST_CASE (parameter_declared)
{
  ioa::system system;
  declare_declared d;
  system.create(system.root_handle, new int(), d.cb1);
  system.declare(d.handle, &d.param, d.cb2);
  BOOST_CHECK (d.declared);
}

class declare_exists {
public:
  bool exists;
  int param;
  ioa::automaton_handle<int> handle;

  struct create_callback {
    declare_exists& m_declare;
    create_callback(declare_exists& declare)
      : m_declare(declare) { }

    void operator() (const ioa::create_result<int>& r) {
      m_declare.handle = r.handle;
    }
  };
  create_callback cb1;

  struct declare_callback {
    declare_exists& m_declare;
    declare_callback(declare_exists& declare)
      : m_declare(declare) { }

    void operator() (const ioa::declare_result<int, int>& r) {
      BOOST_CHECK (r.type == ioa::PARAMETER_DECLARED);
    }
  };
  declare_callback cb2;

  struct declare2_callback {
    declare_exists& m_declare;
    declare2_callback(declare_exists& declare)
      : m_declare(declare) { }

    void operator() (const ioa::declare_result<int, int>& r) {
      BOOST_CHECK (r.type == ioa::PARAMETER_EXISTS);
      m_declare.exists = true;
    }

  };
  declare2_callback cb3;

  declare_exists () :
    exists (false),
    cb1 (*this),
    cb2 (*this),
    cb3 (*this)
  { }
};

BOOST_AUTO_TEST_CASE(parameter_exists)
{
  ioa::system system;
  declare_exists d;
  system.create (system.root_handle, new int(), d.cb1);
  system.declare (d.handle, &d.param, d.cb2);
  system.declare (d.handle, &d.param, d.cb3);
}

struct compose_output_invalid {
  ioa::automaton_handle<automaton> parent_handle;
  ioa::automaton_handle<automaton> child1_handle;
  ioa::automaton_handle<automaton> child2_handle;
  bool flag;

  struct callback1 {
    compose_output_invalid& m_compose;

    callback1(compose_output_invalid& compose) :
      m_compose(compose)
    { }
  
    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.parent_handle = r.handle;
    }
  };
  callback1 cb1;

  struct callback2 {
    compose_output_invalid& m_compose;

    callback2(compose_output_invalid& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      //m_compose.child1_handle = r.handle;
    }
  };
  callback2 cb2;
  
  struct callback3 {
    compose_output_invalid& m_compose;

    callback3(compose_output_invalid& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child2_handle = r.handle;
    }
  };
  callback3 cb3;

  struct callback4 {
    compose_output_invalid& m_compose;

    callback4(compose_output_invalid& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::OUTPUT_INVALID);
      m_compose.flag = true;
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback4 cb4;

  compose_output_invalid()
    : flag (false),
      cb1(*this),
      cb2(*this),
      cb3(*this),
      cb4(*this)
  { }
};

BOOST_AUTO_TEST_CASE(output_invalid)
{
  ioa::system* system = new ioa::system ();
  compose_output_invalid c;
  automaton* parent = new automaton ();
  automaton* child1 = new automaton ();
  automaton* child2 = new automaton ();
  system->create (system->root_handle, parent, c.cb1);
  system->create (c.parent_handle, child1, c.cb2);
  system->create (c.parent_handle, child2, c.cb3);

  ioa::action<automaton::up_t_output_action> output_action (c.child1_handle, child1->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_output_invalid::callback4> input_action (c.child2_handle, child2->up_t_input, c.parent_handle, c.cb4);

  system->compose(output_action, input_action, c.cb4);
  BOOST_CHECK (c.flag);
  delete system;
}

struct compose_input_invalid {
  ioa::automaton_handle<automaton> parent_handle;
  ioa::automaton_handle<automaton> child1_handle;
  ioa::automaton_handle<automaton> child2_handle;
  bool flag;

  struct callback1 {
    compose_input_invalid& m_compose;

    callback1(compose_input_invalid& compose) :
      m_compose(compose)
    { }
  
    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.parent_handle = r.handle;
    }
  };
  callback1 cb1;

  struct callback2 {
    compose_input_invalid& m_compose;

    callback2(compose_input_invalid& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child1_handle = r.handle;
    }
  };
  callback2 cb2;
  
  struct callback3 {
    compose_input_invalid& m_compose;

    callback3(compose_input_invalid& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      //m_compose.child2_handle = r.handle;
    }
  };
  callback3 cb3;

  struct callback4 {
    compose_input_invalid& m_compose;

    callback4(compose_input_invalid& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::INPUT_INVALID);
      m_compose.flag = true;
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback4 cb4;

  compose_input_invalid()
    : flag (false),
      cb1(*this),
      cb2(*this),
      cb3(*this),
      cb4(*this)
  { }
};

BOOST_AUTO_TEST_CASE(input_invalid)
{
  ioa::system* system = new ioa::system ();
  compose_input_invalid c;
  automaton* parent = new automaton ();
  automaton* child1 = new automaton ();
  automaton* child2 = new automaton ();
  system->create (system->root_handle, parent, c.cb1);
  system->create (c.parent_handle, child1, c.cb2);
  system->create (c.parent_handle, child2, c.cb3);

  ioa::action<automaton::up_t_output_action> output_action (c.child1_handle, child1->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_input_invalid::callback4> input_action (c.child2_handle, child2->up_t_input, c.parent_handle.get_automaton (), c.cb4);

  system->compose(output_action, input_action, c.cb4);
  BOOST_CHECK (c.flag);
  delete system;
}

struct compose_composition_exists {
  ioa::automaton_handle<automaton> parent_handle;
  ioa::automaton_handle<automaton> child1_handle;
  ioa::automaton_handle<automaton> child2_handle;
  bool flag;

  struct callback1 {
    compose_composition_exists& m_compose;

    callback1(compose_composition_exists& compose) :
      m_compose(compose)
    { }
  
    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.parent_handle = r.handle;
    }
  };
  callback1 cb1;

  struct callback2 {
    compose_composition_exists& m_compose;

    callback2(compose_composition_exists& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child1_handle = r.handle;
    }
  };
  callback2 cb2;
  
  struct callback3 {
    compose_composition_exists& m_compose;

    callback3(compose_composition_exists& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child2_handle = r.handle;
    }
  };
  callback3 cb3;

  struct callback4 {
    compose_composition_exists& m_compose;

    callback4(compose_composition_exists& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::COMPOSED);
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback4 cb4;

  struct callback5 {
    compose_composition_exists& m_compose;

    callback5(compose_composition_exists& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::COMPOSITION_EXISTS);
      m_compose.flag = true;
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback5 cb5;

  compose_composition_exists()
    : flag(false),
      cb1(*this),
      cb2(*this),
      cb3(*this),
      cb4(*this),
      cb5(*this)
  { }
};

BOOST_AUTO_TEST_CASE(composition_exists)
{
  ioa::system* system = new ioa::system ();
  compose_composition_exists c;
  automaton* parent = new automaton ();
  automaton* child1 = new automaton ();
  automaton* child2 = new automaton ();
  system->create (system->root_handle, parent, c.cb1);
  system->create (c.parent_handle, child1, c.cb2);
  system->create (c.parent_handle, child2, c.cb3);

  ioa::action<automaton::up_t_output_action> output_action1 (c.child1_handle, child1->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_composition_exists::callback4> input_action1 (c.child2_handle, child2->up_t_input, c.parent_handle.get_automaton (), c.cb4);

  system->compose(output_action1, input_action1, c.cb4);

  ioa::action<automaton::up_t_output_action> output_action2 (c.child1_handle, child1->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_composition_exists::callback5> input_action2 (c.child2_handle, child2->up_t_input, c.parent_handle.get_automaton (), c.cb5);

  system->compose(output_action2, input_action2, c.cb5);

  BOOST_CHECK (c.flag);
  delete system;
}

struct compose_input_unavailable {
  ioa::automaton_handle<automaton> parent_handle;
  ioa::automaton_handle<automaton> child1_handle;
  ioa::automaton_handle<automaton> child2_handle;
  ioa::automaton_handle<automaton> child3_handle;
  bool flag;

  struct callback1 {
    compose_input_unavailable& m_compose;

    callback1(compose_input_unavailable& compose) :
      m_compose(compose)
    { }
  
    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.parent_handle = r.handle;
    }
  };
  callback1 cb1;

  struct callback2 {
    compose_input_unavailable& m_compose;

    callback2(compose_input_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child1_handle = r.handle;
    }
  };
  callback2 cb2;
  
  struct callback3 {
    compose_input_unavailable& m_compose;

    callback3(compose_input_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child2_handle = r.handle;
    }
  };
  callback3 cb3;

  struct callback4 {
    compose_input_unavailable& m_compose;

    callback4(compose_input_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child3_handle = r.handle;
    }
  };
  callback4 cb4;


  struct callback5 {
    compose_input_unavailable& m_compose;

    callback5(compose_input_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::COMPOSED);
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback5 cb5;

  struct callback6 {
    compose_input_unavailable& m_compose;

    callback6(compose_input_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::INPUT_UNAVAILABLE);
      m_compose.flag = true;
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback6 cb6;

  compose_input_unavailable()
    : flag(false),
      cb1(*this),
      cb2(*this),
      cb3(*this),
      cb4(*this),
      cb5(*this),
      cb6(*this)
  { }
};

BOOST_AUTO_TEST_CASE(input_unavailable)
{
  ioa::system* system = new ioa::system ();
  compose_input_unavailable c;
  automaton* parent = new automaton ();
  automaton* child1 = new automaton ();
  automaton* child2 = new automaton ();
  automaton* child3 = new automaton ();

  system->create (system->root_handle, parent, c.cb1);
  system->create (c.parent_handle, child1, c.cb2);
  system->create (c.parent_handle, child2, c.cb3);
  system->create (c.parent_handle, child3, c.cb4);

  ioa::action<automaton::up_t_output_action> output_action1 (c.child1_handle, child1->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_input_unavailable::callback5> input_action1 (c.child2_handle, child2->up_t_input, c.parent_handle.get_automaton (), c.cb5);

  system->compose(output_action1, input_action1, c.cb5);

  ioa::action<automaton::up_t_output_action> output_action2 (c.child3_handle, child3->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_input_unavailable::callback6> input_action2 (c.child2_handle, child2->up_t_input, c.parent_handle.get_automaton (), c.cb6);

  system->compose(output_action2, input_action2, c.cb6);

  BOOST_CHECK (c.flag);

  delete system;
}

struct compose_output_unavailable {
  ioa::automaton_handle<automaton> parent_handle;
  ioa::automaton_handle<automaton> child1_handle;
  ioa::automaton_handle<automaton> child2_handle;
  ioa::parameter_handle<automaton, int> child3_handle;
  int param;
  bool flag;

  struct callback1 {
    compose_output_unavailable& m_compose;

    callback1(compose_output_unavailable& compose) :
      m_compose(compose)
    { }
  
    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.parent_handle = r.handle;
    }
  };
  callback1 cb1;

  struct callback2 {
    compose_output_unavailable& m_compose;

    callback2(compose_output_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child1_handle = r.handle;
    }
  };
  callback2 cb2;
  
  struct callback3 {
    compose_output_unavailable& m_compose;

    callback3(compose_output_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child2_handle = r.handle;
    }
  };
  callback3 cb3;

  struct declare_callback {
    compose_output_unavailable& m_compose;
    declare_callback(compose_output_unavailable& compose) :
      m_compose (compose) { }

    void operator() (const ioa::declare_result<automaton, int>& r) {
      m_compose.child3_handle = r.handle;
    }
  };
  declare_callback cb4;

  struct callback5 {
    compose_output_unavailable& m_compose;

    callback5(compose_output_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::COMPOSED);
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback5 cb5;

  struct callback6 {
    compose_output_unavailable& m_compose;

    callback6(compose_output_unavailable& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::OUTPUT_UNAVAILABLE);
      m_compose.flag = true;
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback6 cb6;

  compose_output_unavailable()
    : flag(false),
      cb1(*this),
      cb2(*this),
      cb3(*this),
      cb4(*this),
      cb5(*this),
      cb6(*this)
  { }
};

BOOST_AUTO_TEST_CASE(output_unavailable)
{
  ioa::system* system = new ioa::system ();
  compose_output_unavailable c;
  automaton* parent = new automaton ();
  automaton* child1 = new automaton ();
  automaton* child2 = new automaton ();

  system->create (system->root_handle, parent, c.cb1);
  system->create (c.parent_handle, child1, c.cb2);
  system->create (c.parent_handle, child2, c.cb3);
  system->declare (c.child2_handle, &c.param, c.cb4);

  ioa::action<automaton::up_t_output_action> output_action1 (c.child1_handle, child1->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_output_unavailable::callback5> input_action1 (c.child2_handle, child2->up_t_input, c.parent_handle, c.cb5);

   system->compose(output_action1, input_action1, c.cb5);

   ioa::action<automaton::p_t_input_action, compose_output_unavailable::callback6> input_action2 (c.child3_handle, child2->p_t_input, c.parent_handle, c.cb6);
  
   system->compose(output_action1, input_action2, c.cb6);
   
   BOOST_CHECK (c.flag);
   
   delete system;
}

struct compose_composed {
  ioa::automaton_handle<automaton> parent_handle;
  ioa::automaton_handle<automaton> child1_handle;
  ioa::automaton_handle<automaton> child2_handle;
  bool flag;

  struct callback1 {
    compose_composed& m_compose;

    callback1(compose_composed& compose) :
      m_compose(compose)
    { }
  
    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.parent_handle = r.handle;
    }
  };
  callback1 cb1;

  struct callback2 {
    compose_composed& m_compose;

    callback2(compose_composed& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child1_handle = r.handle;
    }
  };
  callback2 cb2;
  
  struct callback3 {
    compose_composed& m_compose;

    callback3(compose_composed& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child2_handle = r.handle;
    }
  };
  callback3 cb3;

  struct callback4 {
    compose_composed& m_compose;

    callback4(compose_composed& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::COMPOSED);
      m_compose.flag = true;
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback4 cb4;

  compose_composed()
    : flag(false),
      cb1(*this),
      cb2(*this),
      cb3(*this),
      cb4(*this)
  { }
};

BOOST_AUTO_TEST_CASE(composed)
{
  ioa::system* system = new ioa::system ();
  compose_composed c;
  automaton* parent = new automaton ();
  automaton* child1 = new automaton ();
  automaton* child2 = new automaton ();
  system->create (system->root_handle, parent, c.cb1);
  system->create (c.parent_handle, child1, c.cb2);
  system->create (c.parent_handle, child2, c.cb3);

  ioa::action<automaton::up_t_output_action> output_action (c.child1_handle, child1->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_composed::callback4> input_action (c.child2_handle, child2->up_t_input, c.parent_handle.get_automaton (), c.cb4);

  system->compose(output_action, input_action, c.cb4);
  BOOST_CHECK (c.flag);
  system->execute (c.child1_handle, output_action);
  BOOST_CHECK (child2->up_t_input.value == 9845);
  delete system;
}

struct compose_decomposed {
  ioa::automaton_handle<automaton> parent_handle;
  ioa::automaton_handle<automaton> child1_handle;
  ioa::automaton_handle<automaton> child2_handle;
  bool flag;

  struct callback1 {
    compose_decomposed& m_compose;

    callback1(compose_decomposed& compose) :
      m_compose(compose)
    { }
  
    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.parent_handle = r.handle;
    }
  };
  callback1 cb1;

  struct callback2 {
    compose_decomposed& m_compose;

    callback2(compose_decomposed& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child1_handle = r.handle;
    }
  };
  callback2 cb2;
  
  struct callback3 {
    compose_decomposed& m_compose;

    callback3(compose_decomposed& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::create_result<automaton>& r) {
      m_compose.child2_handle = r.handle;
    }
  };
  callback3 cb3;

  struct callback4 {
    compose_decomposed& m_compose;

    callback4(compose_decomposed& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::compose_result& r) {
      BOOST_CHECK_EQUAL (r.type, ioa::COMPOSED);
    }

    void operator() () {
      m_compose.flag = true;
    }
  };
  callback4 cb4;

  struct callback5 {
    compose_decomposed& m_compose;

    callback5(compose_decomposed& compose) :
      m_compose(compose)
    { }

    void operator() (const ioa::decompose_result& r) {
      BOOST_CHECK (false);
    }

    void operator() () {
      BOOST_CHECK (false);
    }
  };
  callback5 cb5;

  compose_decomposed()
    : flag(false),
      cb1(*this),
      cb2(*this),
      cb3(*this),
      cb4(*this),
      cb5(*this)
  { }
};

BOOST_AUTO_TEST_CASE(decomposed)
{
  ioa::system* system = new ioa::system ();
  compose_decomposed c;
  automaton* parent = new automaton ();
  automaton* child1 = new automaton ();
  automaton* child2 = new automaton ();
  system->create (system->root_handle, parent, c.cb1);
  system->create (c.parent_handle, child1, c.cb2);
  system->create (c.parent_handle, child2, c.cb3);

  ioa::action<automaton::up_t_output_action> output_action (c.child1_handle, child1->up_t_output);
  ioa::action<automaton::up_t_input_action, compose_decomposed::callback4> input_action (c.child2_handle, child2->up_t_input, c.parent_handle.get_automaton (), c.cb4);

  system->compose (output_action, input_action, c.cb4);
  system->decompose (output_action, input_action, c.cb5);

  BOOST_CHECK (c.flag);

  delete system;
}

// struct execute_output {
//   ioa::automaton_handle<automaton> m_handle;

//   struct create_callback {
//     execute_output& m_execute_output;
//     create_callback(execute_output& execute_output)
//       : m_execute_output(execute_output) { }
//     void child_created (const ioa::automaton_handle<ioa::root_automaton>& parent,
// 			const ioa::automaton_handle<automaton>& child) {
//       m_execute_output.m_handle = child;
//     }
//     void automaton_exists (const ioa::automaton_handle<ioa::root_automaton>& parent,
// 			   automaton* child) {
//       BOOST_CHECK (false);
//     }
//   };
//   create_callback cb1;

//   execute_output()
//     : cb1(*this) { }
// };

// BOOST_AUTO_TEST_CASE(ExecuteOutput)
// {
//   ioa::system* system = new ioa::system();
//   execute_output c;
//   automaton* a = new automaton();
//   system->create(c.cb1, system->root_handle, a);
//   system->execute_output(c.m_handle, &automaton::output);
//   BOOST_CHECK (a->output.state);
//   delete system;
// }

// struct execute_internal {
//   ioa::automaton_handle<automaton> m_handle;

//   struct create_callback {
//     execute_internal& m_execute_internal;
//     create_callback(execute_internal& execute_internal)
//       : m_execute_internal(execute_internal) { }
//     void child_created (const ioa::automaton_handle<ioa::root_automaton>& parent,
// 			const ioa::automaton_handle<automaton>& child) {
//       m_execute_internal.m_handle = child;
//     }
//     void automaton_exists (const ioa::automaton_handle<ioa::root_automaton>& parent,
// 			   automaton* child) {
//       BOOST_CHECK (false);
//     }
//   };
//   create_callback cb1;

//   execute_internal()
//     : cb1(*this) { }
// };

// BOOST_AUTO_TEST_CASE(ExecuteInternal)
// {
//   ioa::system* system = new ioa::system();
//   execute_internal c;
//   automaton* a = new automaton();
//   system->create(c.cb1, system->root_handle, a);
//   system->execute_internal(c.m_handle, &automaton::internal);
//   BOOST_CHECK (a->internal.state);
//   delete system;
// }

BOOST_AUTO_TEST_SUITE_END()
