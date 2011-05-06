#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class trigger {
private:

  void init_ () {
    ioa::scheduler.schedule (this, &trigger::request);
  }

  bool request_ () {
    // std::cout << "trigger request" << std::endl;
    ioa::scheduler.schedule (this, &trigger::request);
    return true;
  }

public:

  ioa::internal_wrapper<trigger, &trigger::init_> init;  

  typedef ioa::void_output_wrapper<trigger, &trigger::request_> request_type;
  request_type request;

  trigger ()
    : init (*this),
      request (*this)
  { }
};

class ioa_clock {
private:
  int m_counter;
  int m_flag;

  void init_ () {
    ioa::scheduler.schedule (this, &ioa_clock::tick);
  }

  void request_ () {
    // std::cout << "ioa_clock request" << std::endl;
    m_flag = true;
    ioa::scheduler.schedule (this, &ioa_clock::clock);
  }

  void tick_ () {
    // std::cout << "ioa_clock tick" << std::endl;
    m_counter = m_counter + 1;
    ioa::scheduler.schedule (this, &ioa_clock::tick);
  }

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

  ioa::internal_wrapper<ioa_clock, &ioa_clock::tick_> tick;
  
public:

  ioa::internal_wrapper<ioa_clock, &ioa_clock::init_> init;  

  typedef ioa::void_input_wrapper<ioa_clock, &ioa_clock::request_> request_type;
  request_type request;
  typedef ioa::output_wrapper<ioa_clock, int, &ioa_clock::clock_> clock_type;
  clock_type clock;

  ioa_clock ()
    : m_counter (0),
      m_flag (false),
      tick (*this),
      init (*this),
      request (*this),
      clock (*this)
  { }
};

class display {
private:

  void init_ () {
    // Do nothing.
  }

  void clock_ (int t) {
    std::cout << "t = " << t << std::endl;
  }

public:

  ioa::internal_wrapper<display, &display::init_> init;  

  typedef ioa::input_wrapper<display, int, &display::clock_> clock_type;
  clock_type clock;

  display ()
    : init (*this),
      clock (*this)
  { }
};

class composer {
private:
  enum state_type {
    CREATE_TRIGGER,
    CREATE_IOA_CLOCK,
    CREATE_DISPLAY,
    BIND1,
  };
  state_type m_state;
  ioa::automaton_handle<trigger> m_trigger_handle;
  ioa::automaton_handle<ioa_clock> m_ioa_clock_handle;
  ioa::automaton_handle<display> m_display_handle;

  void init_ () {
    ioa::scheduler.create (this, new trigger ());
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case CREATE_TRIGGER:
      BOOST_ASSERT (r.type == ioa::system::CREATE_SUCCESS);
      m_trigger_handle = ioa::cast_automaton<trigger> (r.automaton);
      ioa::scheduler.create (this, new ioa_clock ());
      m_state = CREATE_IOA_CLOCK;
      break;
    case CREATE_IOA_CLOCK:
      BOOST_ASSERT (r.type == ioa::system::CREATE_SUCCESS);
      m_ioa_clock_handle = ioa::cast_automaton<ioa_clock> (r.automaton);
      ioa::scheduler.create (this, new display ());
      m_state = CREATE_DISPLAY;
      break;
    case CREATE_DISPLAY:
      BOOST_ASSERT (r.type == ioa::system::CREATE_SUCCESS);
      m_display_handle = ioa::cast_automaton<display> (r.automaton);
      ioa::scheduler.bind (this,
			   m_trigger_handle,
			   &trigger::request,
			   m_ioa_clock_handle,
			   &ioa_clock::request,
			   &composer::bound1);
      m_state = BIND1;
    case BIND1:
      break;
    }
  }

  void bound1_ (const ioa::system::bind_result& r) {
    BOOST_ASSERT (r.type == ioa::system::BIND_SUCCESS);
    ioa::scheduler.bind (this,
			 m_ioa_clock_handle,
			 &ioa_clock::clock,
			 m_display_handle,
			 &display::clock,
			 &composer::bound2);
  }

  void bound2_ (const ioa::system::bind_result& r) {
    BOOST_ASSERT (r.type == ioa::system::BIND_SUCCESS);
  }

public:

  ioa::internal_wrapper<composer, &composer::init_> init;
  ioa::system_event_wrapper<composer, ioa::system::create_result, &composer::created_> created;
  ioa::system_event_wrapper<composer, ioa::system::bind_result, &composer::bound1_> bound1;
  ioa::system_event_wrapper<composer, ioa::system::bind_result, &composer::bound2_> bound2;

  composer () :
    m_state (CREATE_TRIGGER),
    init (*this),
    created (*this),
    bound1 (*this),
    bound2 (*this)
  { }
};


int
main () {
  ioa::scheduler.run (new composer ());
  return 0; 
}
