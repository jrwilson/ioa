#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class trigger :
  public ioa::dispatching_automaton
{
private:

  UV_UP_OUTPUT (trigger, request) {
    //std::cout << "trigger request" << std::endl;
    ioa::scheduler.schedule (this, &trigger::request);
    return true;
  }

public:
  trigger () :
    ACTION (trigger, request)
  { }

  void init () {
    ioa::scheduler.schedule (this, &trigger::request);
  }
};

class ioa_clock :
  public ioa::dispatching_automaton
{
private:
  int m_counter;
  int m_flag;

  UV_UP_INPUT (ioa_clock, request) {
    //std::cout << "ioa_clock request" << std::endl;
    m_flag = true;
    ioa::scheduler.schedule (this, &ioa_clock::clock);
  }

  UP_INTERNAL (ioa_clock, tick) {
    //std::cout << "ioa_clock tick" << std::endl;
    m_counter = m_counter + 1;
    ioa::scheduler.schedule (this, &ioa_clock::tick);
  }

  V_UP_OUTPUT (ioa_clock, clock, int) {
    //std::cout << "ioa_clock clock" << std::endl;
    if (m_flag) {
      m_flag = false;
      return std::make_pair (true, m_counter);
    }
    else {
      return std::make_pair (false, 0);
    }
  }
public:

  ioa_clock () :
    m_counter (0),
    m_flag (false),
    ACTION (ioa_clock, request),
    ACTION (ioa_clock, tick),
    ACTION (ioa_clock, clock)
  { }

  void init () {
    ioa::scheduler.schedule (this, &ioa_clock::tick);
  }
};

class display :
  public ioa::dispatching_automaton
{
private:

  V_UP_INPUT (display, clock, int, t) {
    std::cout << "t = " << t << std::endl;
  }
public:

  void init () {
    // Do nothing.
  }

  display () :
    ACTION (display, clock)
  { }
};

class composer :
  public ioa::dispatching_automaton
{
private:
  typedef ioa::automaton_helper<composer, trigger> trigger_helper;
  trigger_helper* m_trigger;
  typedef ioa::automaton_helper<composer, ioa_clock> ioa_clock_helper;
  ioa_clock_helper* m_ioa_clock;
  typedef ioa::automaton_helper<composer, display> display_helper;
  display_helper* m_display;
  typedef ioa::bind_helper<composer, trigger_helper, trigger::request_type, ioa_clock_helper, ioa_clock::request_type> bind1_helper;
  bind1_helper* m_bind1;
  typedef ioa::bind_helper<composer, ioa_clock_helper, ioa_clock::clock_type, display_helper, display::clock_type> bind2_helper;
  bind2_helper* m_bind2;

public:
  
  composer ()
  { }

  void init () {
    m_trigger = new trigger_helper (this, ioa::make_instance_generator<trigger> ());
    m_ioa_clock = new ioa_clock_helper (this, ioa::make_instance_generator<ioa_clock> ());
    m_display = new display_helper (this, ioa::make_instance_generator<display> ());
    m_bind1 = new bind1_helper (this, m_trigger, &trigger::request, m_ioa_clock, &ioa_clock::request);
    m_bind2 = new bind2_helper (this, m_ioa_clock, &ioa_clock::clock, m_display, &display::clock);
  }

};


int
main () {
  ioa::scheduler.run (ioa::make_instance_generator<composer> ());
  return 0; 
}
