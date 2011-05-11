#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class trigger :
  public ioa::automaton_interface
{
private:

  void init () {
    ioa::scheduler.schedule (this, &trigger::request);
  }

  bool request_ () {
    // std::cout << "trigger request" << std::endl;
    ioa::scheduler.schedule (this, &trigger::request);
    return true;
  }

public:
  ioa::void_output_wrapper<trigger, &trigger::request_> request;

  trigger () :
    request (*this)
  { }
};

class ioa_clock :
  public ioa::automaton_interface
{
private:
  int m_counter;
  int m_flag;

  void init () {
    ioa::scheduler.schedule (this, &ioa_clock::tick);
  }

  void request_ () {
    // std::cout << "ioa_clock request" << std::endl;
    m_flag = true;
    ioa::scheduler.schedule (this, &ioa_clock::clock);
  }
public:
  ioa::void_input_wrapper<ioa_clock, &ioa_clock::request_> request;

private:
  void tick_ () {
    // std::cout << "ioa_clock tick" << std::endl;
    m_counter = m_counter + 1;
    ioa::scheduler.schedule (this, &ioa_clock::tick);
  }
  ioa::internal_wrapper<ioa_clock, &ioa_clock::tick_> tick;

  std::pair<bool, int> clock_ () {
    // std::cout << "ioa_clock clock" << std::endl;
    if (m_flag) {
      m_flag = false;
      return std::make_pair (true, m_counter);
    }
    else {
      return std::make_pair (false, 0);
    }
  }
public:
  ioa::output_wrapper<ioa_clock, int, &ioa_clock::clock_> clock;

  ioa_clock () :
    m_counter (0),
    m_flag (false),
    request (*this),
    tick (*this),
    clock (*this)
  { }
};

class display :
  public ioa::automaton_interface
{
private:

  void init () {
    // Do nothing.
  }

  void clock_ (int t) {
    std::cout << "t = " << t << std::endl;
  }
public:
  ioa::input_wrapper<display, int, &display::clock_> clock;

  display () :
      clock (*this)
  { }
};

class composer :
  public ioa::automaton_interface
{
private:
  enum state_type {
    START,
    CREATE_TRIGGER_SENT,
    CREATE_TRIGGER_RECV,
    CREATE_IOA_CLOCK_SENT,
    CREATE_IOA_CLOCK_RECV,
    CREATE_DISPLAY_SENT,
    CREATE_DISPLAY_RECV,
    BIND1_SENT,
    BIND1_RECV,
    BIND2_SENT,
    BIND2_RECV,
    STOP
  };
  state_type m_state;
  ioa::automaton_handle<trigger> m_trigger_handle;
  ioa::automaton_handle<ioa_clock> m_ioa_clock_handle;
  ioa::automaton_handle<display> m_display_handle;

  void transition_ () {
    switch (m_state) {
    case START:
      ioa::scheduler.create (this, new trigger ());
      m_state = CREATE_TRIGGER_SENT;
      break;
    case CREATE_TRIGGER_RECV:
      ioa::scheduler.create (this, new ioa_clock ());
      m_state = CREATE_IOA_CLOCK_SENT;
      break;
    case CREATE_IOA_CLOCK_RECV:
      ioa::scheduler.create (this, new display ());
      m_state = CREATE_DISPLAY_SENT;
      break;
    case CREATE_DISPLAY_RECV:
      ioa::scheduler.bind (this, m_trigger_handle, &trigger::request, m_ioa_clock_handle, &ioa_clock::request);
      m_state = BIND1_SENT;
      break;
    case BIND1_RECV:
      ioa::scheduler.bind (this, m_ioa_clock_handle, &ioa_clock::clock, m_display_handle, &display::clock);
      m_state = BIND2_SENT;
      break;
    case BIND2_RECV:
      m_state = STOP;
      break;
    default:
      BOOST_ASSERT (false);
      break;
    }
  }
  ioa::internal_wrapper<composer, &composer::transition_> transition;

  void init () {
    ioa::scheduler.schedule (this, &composer::transition);
  }

  void automaton_created (const ioa::generic_automaton_handle& automaton) {
    switch (m_state) {
    case CREATE_TRIGGER_SENT:
      m_trigger_handle = ioa::cast_automaton<trigger> (automaton);
      m_state = CREATE_TRIGGER_RECV;
      ioa::scheduler.schedule (this, &composer::transition);
      break;
    case CREATE_IOA_CLOCK_SENT:
      m_ioa_clock_handle = ioa::cast_automaton<ioa_clock> (automaton);
      m_state = CREATE_IOA_CLOCK_RECV;
      ioa::scheduler.schedule (this, &composer::transition);
      break;
    case CREATE_DISPLAY_SENT:
      m_display_handle = ioa::cast_automaton<display> (automaton);
      m_state = CREATE_DISPLAY_RECV;
      ioa::scheduler.schedule (this, &composer::transition);
      break;
    default:
      BOOST_ASSERT (false);
      break;
    }
  }

  void bound () {
    switch (m_state) {
    case BIND1_SENT:
      m_state = BIND1_RECV;
      ioa::scheduler.schedule (this, &composer::transition);
      break;
    case BIND2_SENT:
      m_state = BIND2_RECV;
      ioa::scheduler.schedule (this, &composer::transition);
      break;
    default:
      BOOST_ASSERT (false);
      break;
    }
  }

public:

  composer () :
    m_state (START),
    transition (*this)
  { }
};


int
main () {
  ioa::scheduler.run (new composer ());
  return 0; 
}
