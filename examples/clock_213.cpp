#include <iostream>

#include <ioa.hpp>

class trigger :
  public ioa::automaton_interface
{
private:

  bool request_precondition () const {
    return true;
  }
  
  void request_action () {
    schedule ();
  }
  
  void schedule () {
    if (request_precondition ()) {
      ioa::scheduler::schedule (&trigger::request);
    }
  }

public:
  trigger () :
    ACTION (trigger, request)
  {
    schedule ();
  }

  UV_UP_OUTPUT (trigger, request);
};

class ioa_clock :
  public ioa::automaton_interface
{
private:
  int m_counter;
  int m_flag;

  void request_action () {
    m_flag = true;
    schedule ();
  }

  bool tick_precondition () const {
    return true;
  }
  
  void tick_action () {
    m_counter = m_counter + 1;
    schedule ();
  }

  UP_INTERNAL (ioa_clock, tick);

  bool clock_precondition () const {
    return m_flag;
  }

  int clock_action () {
    m_flag = false;
    schedule ();
    return m_counter;
  }

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

  UV_UP_INPUT (ioa_clock, request);
  V_UP_OUTPUT (ioa_clock, clock, int);
};


class display :
  public ioa::automaton_interface
{
private:

  void clock_action (int const & t) {
    std::cout << "t = " << t << std::endl;
  }

public:
  
  V_UP_INPUT (display, clock, int);
  
};


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
