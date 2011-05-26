#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class trigger :
  public ioa::dispatching_automaton
{
private:

  bool request_ () {
    //std::cout << "trigger request" << std::endl;
    ioa::scheduler.schedule (this, &trigger::request);
    return true;
  }

public:
  typedef ioa::void_output_wrapper<trigger, &trigger::request_> request_t;
  request_t request;

  trigger () :
    request (*this)
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

  void request_ () {
    //std::cout << "ioa_clock request" << std::endl;
    m_flag = true;
    ioa::scheduler.schedule (this, &ioa_clock::clock);
  }
public:
  typedef ioa::void_input_wrapper<ioa_clock, &ioa_clock::request_> request_t;
  request_t request;

private:
  void tick_ () {
    //std::cout << "ioa_clock tick" << std::endl;
    m_counter = m_counter + 1;
    ioa::scheduler.schedule (this, &ioa_clock::tick);
  }
  ioa::internal_wrapper<ioa_clock, &ioa_clock::tick_> tick;

  std::pair<bool, int> clock_ () {
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
  typedef ioa::output_wrapper<ioa_clock, int, &ioa_clock::clock_> clock_t;
  clock_t clock;

  ioa_clock () :
    m_counter (0),
    m_flag (false),
    request (*this),
    tick (*this),
    clock (*this)
  { }

  void init () {
    ioa::scheduler.schedule (this, &ioa_clock::tick);
  }
};

class display :
  public ioa::dispatching_automaton
{
private:

  void clock_ (int t) {
    std::cout << "t = " << t << std::endl;
  }
public:
  typedef ioa::input_wrapper<display, int, &display::clock_> clock_t;
  clock_t clock;

  void init () {
    // Do nothing.
  }

  display () :
      clock (*this)
  { }
};

class composer :
  public ioa::dispatching_automaton
{
private:
  typedef ioa::automaton_helper<composer, ioa::instance_generator<trigger> > trigger_helper;
  trigger_helper m_trigger;
  typedef ioa::automaton_helper<composer, ioa::instance_generator<ioa_clock> > ioa_clock_helper;
  ioa_clock_helper m_ioa_clock;
  typedef ioa::automaton_helper<composer, ioa::instance_generator<display> > display_helper;
  display_helper m_display;
  typedef ioa::bind_helper<composer, trigger_helper, trigger::request_t, ioa_clock_helper, ioa_clock::request_t> bind1_helper;
  bind1_helper m_bind1;
  typedef ioa::bind_helper<composer, ioa_clock_helper, ioa_clock::clock_t, display_helper, display::clock_t> bind2_helper;
  bind2_helper m_bind2;

public:
  
  composer () :
    m_trigger (this, trigger_helper::generator ()),
    m_ioa_clock (this, ioa_clock_helper::generator ()),
    m_display (this, display_helper::generator ()),
    m_bind1 (this, &m_trigger, &trigger::request, &m_ioa_clock, &ioa_clock::request),
    m_bind2 (this, &m_ioa_clock, &ioa_clock::clock, &m_display, &display::clock)
  { }

  void init () { 
    m_trigger.create ();
    m_ioa_clock.create ();
    m_display.create ();
    m_bind1.bind ();
    m_bind2.bind ();
  }

};


int
main () {
  ioa::scheduler.run (ioa::instance_generator<composer> ());
  return 0; 
}
