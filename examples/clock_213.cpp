#include <iostream>

#include <ioa.hpp>

class trigger :
  public ioa::automaton_interface
{
private:

  bool trigger_precondition () const {
    return true;
  }

  DECLARE_UV_UP_OUTPUT (trigger, request);

  void schedule () {
    if (trigger_precondition ()) {
      ioa::scheduler::schedule (&trigger::request);
    }
  }

public:
  trigger () :
    ACTION (trigger, request)
  {
    schedule ();
  }
};

DEFINE_UV_UP_OUTPUT (trigger, request) {
  bool retval = false;
  if (trigger_precondition ()) {
    retval = true;
  }
  schedule ();
  return retval;
}

class ioa_clock :
  public ioa::automaton_interface
{
private:
  int m_counter;
  int m_flag;

  DECLARE_UV_UP_INPUT (ioa_clock, request);

  bool tick_precondition () const {
    return true;
  }

  DECLARE_UP_INTERNAL (ioa_clock, tick);

  bool clock_precondition () const {
    return m_flag;
  }

  DECLARE_V_UP_OUTPUT (ioa_clock, clock, int);

  void schedule () {
    if (tick_precondition ()) {
      ioa::scheduler::schedule (&ioa_clock::tick);
    }
    if (clock_precondition ()) {
      ioa::scheduler::schedule (&ioa_clock::clock);
    }
  }

public:

  ioa_clock () :
    m_counter (0),
    m_flag (false),
    ACTION (ioa_clock, clock)
  {
    schedule ();
  }

};

DEFINE_UV_UP_INPUT (ioa_clock, request) {
  m_flag = true;
  schedule ();
}

DEFINE_UP_INTERNAL (ioa_clock, tick) {
  if (tick_precondition ()) {
    m_counter = m_counter + 1;
  }
  schedule ();
}

DEFINE_V_UP_OUTPUT (ioa_clock, clock, int) {
  std::pair<bool, int> retval;
  if (clock_precondition ()) {
    m_flag = false;
    retval = std::make_pair (true, m_counter);
  }
  schedule ();
  return retval;
}

class display :
  public ioa::automaton_interface
{
private:

  DECLARE_V_UP_INPUT (display, clock, int, t);

};

DEFINE_V_UP_INPUT (display, clock, int, t) {
  std::cout << "t = " << t << std::endl;
}

class composer :
  public ioa::automaton_interface
{
public:
  composer ()
  {
    ioa::automaton_helper<trigger>* t = new ioa::automaton_helper<trigger> (this, ioa::make_generator<trigger> ());
    ioa::automaton_helper<ioa_clock>* c = new ioa::automaton_helper<ioa_clock> (this, ioa::make_generator<ioa_clock> ());
    ioa::automaton_helper<display>* d = new ioa::automaton_helper<display> (this, ioa::make_generator<display> ());
    ioa::make_bind_helper (this, t, &trigger::request, c, &ioa_clock::request);
    ioa::make_bind_helper (this, c, &ioa_clock::clock, d, &display::clock);
  }

};

int
main () {
  ioa::scheduler::run (ioa::make_generator<composer> ());
  return 0; 
}
