#include "minunit.h"

#include "automaton2.hpp"
#include <ioa/ioa.hpp>
#include <iostream>

static bool goal_reached;

class uv_up_uv_up_automaton :
  public ioa::automaton_interface,
  public ioa::observer
{
private:
  ioa::bind_helper_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->is_bound ()) {
      goal_reached = true;
    }
  }

public:  
  uv_up_uv_up_automaton () {
    ioa::automaton_helper<automaton2>* output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = ioa::make_bind_helper (this, output, &automaton2::uv_up_output, input, &automaton2::uv_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_up_uv_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<uv_up_uv_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_up_uv_p_automaton :
  public ioa::automaton_interface,
  public ioa::observer
{
private:
  ioa::bind_helper_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->is_bound ()) {
      goal_reached = true;
    }
  }

public:  
  uv_up_uv_p_automaton () {
    ioa::automaton_helper<automaton2>* output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = ioa::make_bind_helper (this, output, &automaton2::uv_up_output, input, &automaton2::uv_p_input, 37);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_up_uv_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<uv_up_uv_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_p_uv_up_automaton :
  public ioa::automaton_interface,
  public ioa::observer
{
private:
  ioa::bind_helper_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->is_bound ()) {
      goal_reached = true;
    }
  }

public:  
  uv_p_uv_up_automaton () {
    ioa::automaton_helper<automaton2>* output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = ioa::make_bind_helper (this, output, &automaton2::uv_p_output, 37, input, &automaton2::uv_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_p_uv_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<uv_p_uv_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class uv_p_uv_p_automaton :
  public ioa::automaton_interface,
  public ioa::observer
{
private:
  ioa::bind_helper_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->is_bound ()) {
      goal_reached = true;
    }
  }

public:  
  uv_p_uv_p_automaton () {
    ioa::automaton_helper<automaton2>* output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = ioa::make_bind_helper (this, output, &automaton2::uv_p_output, 37, input, &automaton2::uv_p_input, 85);
    add_observable (m_helper);
  };
};

static const char*
bind_uv_p_uv_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<uv_p_uv_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_up_v_up_automaton :
  public ioa::automaton_interface,
  public ioa::observer
{
private:
  ioa::bind_helper_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->is_bound ()) {
      goal_reached = true;
    }
  }

public:  
  v_up_v_up_automaton () {
    ioa::automaton_helper<automaton2>* output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = ioa::make_bind_helper (this, output, &automaton2::v_up_output, input, &automaton2::v_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_v_up_v_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<v_up_v_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_up_v_p_automaton :
  public ioa::automaton_interface,
  public ioa::observer
{
private:
  ioa::bind_helper_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->is_bound ()) {
      goal_reached = true;
    }
  }

public:  
  v_up_v_p_automaton () {
    ioa::automaton_helper<automaton2>* output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = ioa::make_bind_helper (this, output, &automaton2::v_up_output, input, &automaton2::v_p_input, 37);
    add_observable (m_helper);
  };
};

static const char*
bind_v_up_v_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<v_up_v_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_p_v_up_automaton :
  public ioa::automaton_interface,
  public ioa::observer
{
private:
  ioa::bind_helper_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->is_bound ()) {
      goal_reached = true;
    }
  }

public:  
  v_p_v_up_automaton () {
    ioa::automaton_helper<automaton2>* output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = ioa::make_bind_helper (this, output, &automaton2::v_p_output, 37, input, &automaton2::v_up_input);
    add_observable (m_helper);
  };
};

static const char*
bind_v_p_v_up ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<v_p_v_up_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

class v_p_v_p_automaton :
  public ioa::automaton_interface,
  public ioa::observer
{
private:
  ioa::bind_helper_interface* m_helper;

  void observe (ioa::observable* o) {
    assert (o == m_helper);
    if (m_helper->is_bound ()) {
      goal_reached = true;
    }
  }

public:  
  v_p_v_p_automaton () {
    ioa::automaton_helper<automaton2>* output = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    ioa::automaton_helper<automaton2>* input = new ioa::automaton_helper<automaton2> (this, ioa::make_generator<automaton2> ());
    m_helper = ioa::make_bind_helper (this, output, &automaton2::v_p_output, 37, input, &automaton2::v_p_input, 85);
    add_observable (m_helper);
  };
};

static const char*
bind_v_p_v_p ()
{
  std::cout << __func__ << std::endl;
  goal_reached = false;
  ioa::scheduler::run (ioa::make_generator<v_p_v_p_automaton> ());
  mu_assert (goal_reached);
  return 0;
}

const char*
all_tests ()
{
  mu_run_test (bind_uv_up_uv_up);
  mu_run_test (bind_uv_up_uv_p);
  mu_run_test (bind_uv_p_uv_up);
  mu_run_test (bind_uv_p_uv_p);
  mu_run_test (bind_v_up_v_up);
  mu_run_test (bind_v_up_v_p);
  mu_run_test (bind_v_p_v_up);
  mu_run_test (bind_v_p_v_p);

  return 0;
}
