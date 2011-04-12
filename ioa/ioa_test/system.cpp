#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE System
#include <boost/test/unit_test.hpp>
#include "../src/ioa.hpp"
#include "automaton.hpp"

BOOST_AUTO_TEST_SUITE(SystemSuite)

BOOST_AUTO_TEST_CASE(GetRoot)
{
  ioa::system system;
  BOOST_CHECK (system.root_handle.valid());
}

struct create {
  bool m_created;

  struct create_callback {
    create& m_create;
    create_callback(create& create)
      : m_create(create) { }
    void child_created (const ioa::automaton_handle<ioa::root_automaton>& parent,
			const ioa::automaton_handle<int>& child) {
      BOOST_CHECK (child.valid());
      m_create.m_created = true;
    }
    void automaton_exists (const ioa::automaton_handle<ioa::root_automaton>& parent,
			   int* child) {
      BOOST_CHECK (false);
    }
  };
  create_callback cb1;

  create()
    : m_created(false),
      cb1(*this) { }
};

BOOST_AUTO_TEST_CASE(Create)
{
  ioa::system system;
  create c;
  system.create(c.cb1, system.root_handle, new int());
  BOOST_CHECK (c.m_created);
}

struct create1 {
  int* m_x;
  bool m_created;
  bool m_exists;

  struct create1_callback {
    create1& m_create;
    create1_callback(create1& create)
      : m_create(create) { }
    void child_created (const ioa::automaton_handle<ioa::root_automaton>& parent,
			const ioa::automaton_handle<int>& child) {
     BOOST_CHECK(child.valid());
      m_create.m_created = true;
    }
    void automaton_exists(const ioa::automaton_handle<ioa::root_automaton>& parent,
			  int* child) {
      BOOST_CHECK(false);
    }
  };
  create1_callback cb1;

  struct create2_callback {
    create1& m_create;
    create2_callback(create1& create)
      : m_create(create) { }
    void child_created (const ioa::automaton_handle<ioa::root_automaton>& parent,
			const ioa::automaton_handle<int>& child) {
      BOOST_CHECK(false);
    }
    void automaton_exists(const ioa::automaton_handle<ioa::root_automaton>& parent,
			  int* child) {
      BOOST_CHECK(m_create.m_x == child);
      m_create.m_exists = true;
    }
  };
  create2_callback cb2;

  create1(int* x)
    : m_x(x),
      m_created(false),
      m_exists(false),
      cb1(*this),
      cb2(*this) { }
};

BOOST_AUTO_TEST_CASE(CreateDuplicate)
{
  ioa::system* system = new ioa::system();
  int* x = new int();
  create1 c(x);
  system->create(c.cb1, system->root_handle, x);
  system->create(c.cb2, system->root_handle, x);
  BOOST_CHECK (c.m_created);
  BOOST_CHECK (c.m_exists);
  delete system;
}


class declare {
public:
  int param;
  ioa::automaton_handle<int> handle;

  struct create_callback {
    declare& m_declare;
    create_callback(declare& declare)
      : m_declare(declare) { }
    void child_created(const ioa::automaton_handle<ioa::root_automaton>& parent,
		       const ioa::automaton_handle<int>& child) {
      m_declare.handle = child;
    }
    void automaton_exists(const ioa::automaton_handle<ioa::root_automaton>& parent,
			  int*) {
      BOOST_CHECK(false);
    }
  };
  create_callback cb1;

  struct declare_callback {
    declare& m_declare;
    declare_callback(declare& declare)
      : m_declare(declare) { }
    void parameter_declared(const ioa::automaton_handle<int>& handle,
			    const void* parameter) {
      BOOST_CHECK (&m_declare.param == parameter);
    }
    void parameter_exists(const ioa::automaton_handle<int>& handle,
			  const void* parameter) {
      BOOST_CHECK (false);
    }
  };
  declare_callback cb2;

  declare()
    : cb1(*this),
      cb2(*this) { }
};

BOOST_AUTO_TEST_CASE(Declare)
{
  ioa::system system;
  declare d;
  system.create(d.cb1, system.root_handle, new int());
  system.declare(d.cb2, d.handle, &d.param);
}

class declare2 {
public:
  int param;
  ioa::automaton_handle<int> handle;

  struct create_callback {
    declare2& m_declare;
    create_callback(declare2& declare)
      : m_declare(declare) { }
    void child_created(const ioa::automaton_handle<ioa::root_automaton>& parent,
		       const ioa::automaton_handle<int>& child) {
      m_declare.handle = child;
    }
    void automaton_exists(const ioa::automaton_handle<ioa::root_automaton>& parent,
			  int* x) {
      BOOST_CHECK(false);
    }
  };
  create_callback cb1;

  struct declare_callback {
    declare2& m_declare;
    declare_callback(declare2& declare)
      : m_declare(declare) { }
    void parameter_declared(const ioa::automaton_handle<int>& handle,
			    const void* parameter) {
      BOOST_CHECK (&m_declare.param == parameter);
    }
    void parameter_exists(const ioa::automaton_handle<int>& handle,
			  const void* parameter) {
      BOOST_CHECK (false);
    }
  };
  declare_callback cb2;

  struct declare2_callback {
    declare2& m_declare;
    declare2_callback(declare2& declare)
      : m_declare(declare) { }
    void parameter_declared(const ioa::automaton_handle<int>& handle,
			    const void* parameter) {
      BOOST_CHECK (false);
    }
    void parameter_exists(const ioa::automaton_handle<int>& handle,
			  const void* parameter) {
      BOOST_CHECK (&m_declare.param == parameter);
    }
  };
  declare2_callback cb3;

  declare2()
    : cb1(*this),
      cb2(*this),
      cb3(*this) { }
};

BOOST_AUTO_TEST_CASE(Declare2)
{
  ioa::system system;
  declare2 d;
  system.create(d.cb1, system.root_handle, new int());
  system.declare(d.cb2, d.handle, &d.param);
  system.declare(d.cb3, d.handle, &d.param);
}

struct compose {
  ioa::automaton_handle<automaton> parent_handle;
  ioa::automaton_handle<automaton> child1_handle;
  ioa::automaton_handle<automaton> child2_handle;
  bool composed;
  bool already_composed;

  struct callback1 {
    compose& m_compose;
    callback1(compose& compose)
      : m_compose(compose) { }
    void child_created(const ioa::automaton_handle<ioa::root_automaton>& parent_handle,
		       const ioa::automaton_handle<automaton>& child_handle) {
      m_compose.parent_handle = child_handle;
    }
    void automaton_exists(const ioa::automaton_handle<ioa::root_automaton>& parent_handle,
			  automaton* ptr) {
      BOOST_CHECK(false);
    }
  };
  callback1 cb1;

  struct callback2 {
    compose& m_compose;
    callback2(compose& compose)
      : m_compose(compose) { }
    void child_created(const ioa::automaton_handle<automaton>& parent_handle,
		       const ioa::automaton_handle<automaton>& child_handle) {
      m_compose.child1_handle = child_handle;
    }
    void automaton_exists(const ioa::automaton_handle<automaton>& parent_handle,
			  automaton* ptr) {
      BOOST_CHECK(false);
    }
  };
  callback2 cb2;
  
  struct callback3 {
    compose& m_compose;
    callback3(compose& compose)
      : m_compose(compose) { }
    void child_created(const ioa::automaton_handle<automaton>& parent_handle,
		       const ioa::automaton_handle<automaton>& child_handle) {
      m_compose.child2_handle = child_handle;
    }
    void automaton_exists(const ioa::automaton_handle<automaton>& parent_handle,
			  automaton* ptr) {
      BOOST_CHECK(false);
    }
  };
  callback3 cb3;

  struct callback4 {
    compose& m_compose;
    callback4(compose& compose)
      : m_compose(compose) { }
    void output_invalid() {
      BOOST_CHECK(false);
    }
    void input_invalid() {
      BOOST_CHECK(false);
    }
    void composition_exists() {
      BOOST_CHECK(false);
    }
    void input_unavailable() {
      BOOST_CHECK(false);
    }
    void output_unavailable() {
      BOOST_CHECK(false);
    }
    void composed() {
      m_compose.composed = true;
    }
    void decompose() {
      BOOST_CHECK(false);
    }
  };
  callback4 cb4;

  struct callback5 {
    compose& m_compose;
    callback5(compose& compose)
      : m_compose(compose) { }
    void output_invalid() {
      BOOST_CHECK(false);
    }
    void input_invalid() {
      BOOST_CHECK(false);
    }
    void composition_exists() {
      m_compose.already_composed = true;
    }
    void input_unavailable() {
      BOOST_CHECK(false);
    }
    void output_unavailable() {
      BOOST_CHECK(false);
    }
    void composed() {
      BOOST_CHECK(false);
    }
    void decompose() {
      BOOST_CHECK(false);
    }
  };
  callback5 cb5;
  
  compose()
    : composed(false),
      already_composed(false),
      cb1(*this),
      cb2(*this),
      cb3(*this),
      cb4(*this),
      cb5(*this){ }
};

BOOST_AUTO_TEST_CASE(Compose)
{
  ioa::system* system = new ioa::system();
  compose c;
  automaton* parent = new automaton();
  automaton* child1 = new automaton();
  automaton* child2 = new automaton();
  system->create(c.cb1, system->root_handle, parent);
  system->create(c.cb2, c.parent_handle, child1);
  system->create(c.cb3, c.parent_handle, child2);
  system->compose<compose::callback4, automaton, automaton, automaton::output_action, automaton, automaton::input_action, int>(c.cb4, c.parent_handle, c.child1_handle, &automaton::output, c.child2_handle, &automaton::input);
  system->compose<compose::callback5, automaton, automaton, automaton::output_action, automaton, automaton::input_action, int>(c.cb5, c.parent_handle, c.child1_handle, &automaton::output, c.child2_handle, &automaton::input);
  BOOST_CHECK (c.composed);
  BOOST_CHECK (c.already_composed);
  system->execute_output (c.child1_handle, &automaton::output);
  BOOST_CHECK (child2->value == 9845);
  delete system;
}

struct execute_output {
  ioa::automaton_handle<automaton> m_handle;

  struct create_callback {
    execute_output& m_execute_output;
    create_callback(execute_output& execute_output)
      : m_execute_output(execute_output) { }
    void child_created (const ioa::automaton_handle<ioa::root_automaton>& parent,
			const ioa::automaton_handle<automaton>& child) {
      m_execute_output.m_handle = child;
    }
    void automaton_exists (const ioa::automaton_handle<ioa::root_automaton>& parent,
			   automaton* child) {
      BOOST_CHECK (false);
    }
  };
  create_callback cb1;

  execute_output()
    : cb1(*this) { }
};

BOOST_AUTO_TEST_CASE(ExecuteOutput)
{
  ioa::system* system = new ioa::system();
  execute_output c;
  automaton* a = new automaton();
  system->create(c.cb1, system->root_handle, a);
  system->execute_output(c.m_handle, &automaton::output);
  BOOST_CHECK (a->output.state);
  delete system;
}

struct execute_internal {
  ioa::automaton_handle<automaton> m_handle;

  struct create_callback {
    execute_internal& m_execute_internal;
    create_callback(execute_internal& execute_internal)
      : m_execute_internal(execute_internal) { }
    void child_created (const ioa::automaton_handle<ioa::root_automaton>& parent,
			const ioa::automaton_handle<automaton>& child) {
      m_execute_internal.m_handle = child;
    }
    void automaton_exists (const ioa::automaton_handle<ioa::root_automaton>& parent,
			   automaton* child) {
      BOOST_CHECK (false);
    }
  };
  create_callback cb1;

  execute_internal()
    : cb1(*this) { }
};

BOOST_AUTO_TEST_CASE(ExecuteInternal)
{
  ioa::system* system = new ioa::system();
  execute_internal c;
  automaton* a = new automaton();
  system->create(c.cb1, system->root_handle, a);
  system->execute_internal(c.m_handle, &automaton::internal);
  BOOST_CHECK (a->internal.state);
  delete system;
}

BOOST_AUTO_TEST_SUITE_END()