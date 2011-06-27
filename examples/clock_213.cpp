#include <iostream>

#include <ioa/ioa.hpp>
#include <ioa/simple_scheduler.hpp>

class trigger :
  public ioa::automaton
{
private:

  bool request_precondition () const {
    return true;
  }
  
  void request_effect () {
  }
  
  void schedule () const {
    if (request_precondition ()) {
      ioa::schedule (&trigger::request);
    }
  }

public:
  trigger ()
  {
    schedule ();
  }

  UV_UP_OUTPUT (trigger, request);
};

/*
  Clock Automaton
  Distributed Algorithms, p. 213.
*/

class clock_automaton :
  public ioa::automaton
{
private:
  int m_counter;
  int m_flag;

public:

  clock_automaton () :
    m_counter (0),
    m_flag (false)
  {
    schedule ();
  }

private:
  void schedule () const {
    if (tick_precondition ()) {
      ioa::schedule (&clock_automaton::tick);
    }
    if (clock_precondition ()) {
      ioa::schedule (&clock_automaton::clock);
    }
  }

  void request_effect () {
    m_flag = true;
  }

public:
  UV_UP_INPUT (clock_automaton, request);

private:
  bool tick_precondition () const {
    return true;
  }
  
  void tick_effect () {
    m_counter = m_counter + 1;
  }

public:
  UP_INTERNAL (clock_automaton, tick);

private:
  bool clock_precondition () const {
    return m_flag;
  }

  int clock_effect () {
    m_flag = false;
    return m_counter;
  }

public:
  V_UP_OUTPUT (clock_automaton, clock, int);
};


class display :
  public ioa::automaton
{
private:
  void schedule () const { }

  void clock_effect (int const & t) {
    std::cout << "t = " << t << std::endl;
  }

public:
  V_UP_INPUT (display, clock, int);
  
};


class composer :
  public ioa::automaton
{
public:
  composer ()
  {
    ioa::automaton_manager<trigger>* t = new ioa::automaton_manager<trigger> (this, ioa::make_generator<trigger> ());
    ioa::automaton_manager<clock_automaton>* c = new ioa::automaton_manager<clock_automaton> (this, ioa::make_generator<clock_automaton> ());
    ioa::automaton_manager<display>* d = new ioa::automaton_manager<display> (this, ioa::make_generator<display> ());
    ioa::make_bind_helper (this, t, &trigger::request, c, &clock_automaton::request);
    ioa::make_bind_helper (this, c, &clock_automaton::clock, d, &display::clock);
  }

};

int
main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<composer> ());
  return 0; 
}
