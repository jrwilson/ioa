#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class trigger {
private:

  void init_ () {
    ioa::scheduler.schedule_output (&trigger::request);
  }

  bool request_ () {
    //std::cout << "trigger request" << std::endl;
    ioa::scheduler.schedule_output (&trigger::request);
    return true;
  }

public:

  // TODO: This should have a trait that prevents composition.
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
    ioa::scheduler.schedule_internal (&ioa_clock::tick);
  }

  void request_ () {
    //std::cout << "ioa_clock request" << std::endl;
    m_flag = true;
    ioa::scheduler.schedule_output (&ioa_clock::clock);
  }

  void tick_ () {
    //std::cout << "ioa_clock tick" << std::endl;
    m_counter = m_counter + 1;
    ioa::scheduler.schedule_internal (&ioa_clock::tick);
  }

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

  ioa::internal_wrapper<ioa_clock, &ioa_clock::tick_> tick;
  
public:

  // TODO: This should have a trait that prevents composition.
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

  // TODO: This should have a trait that prevents composition.
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

  ioa::automaton_handle<trigger> m_trigger_handle;
  ioa::automaton_handle<ioa_clock> m_ioa_clock_handle;
  ioa::automaton_handle<display> m_display_handle;

  class trigger_callback {
  private:
    composer& m_composer;
    
  public:
    trigger_callback (composer& composer) :
      m_composer (composer)
    { }
    
    void operator() (const ioa::create_result<trigger>& r) {
      assert (r.type == ioa::AUTOMATON_CREATED);
      m_composer.m_trigger_handle = r.handle;
      ioa_clock_callback cb (m_composer);
      ioa::scheduler.schedule_create<composer, ioa_clock, ioa_clock_callback> (new ioa_clock(), cb);
    }
  };

  class ioa_clock_callback {
  private:
    composer& m_composer;
    
  public:
    ioa_clock_callback (composer& composer) :
      m_composer (composer)
    { }
    
    void operator() (const ioa::create_result<ioa_clock>& r) {
      assert (r.type == ioa::AUTOMATON_CREATED);
      m_composer.m_ioa_clock_handle = r.handle;
      display_callback cb (m_composer);
      ioa::scheduler.schedule_create<composer, display, display_callback> (new display(), cb);
    }
  };

  class display_callback {
  private:
    composer& m_composer;
    
  public:
    display_callback (composer& composer) :
      m_composer (composer)
    { }
    
    void operator() (const ioa::create_result<display>& r) {
      assert (r.type == ioa::AUTOMATON_CREATED);
      m_composer.m_display_handle = r.handle;
      compose_callback1 cb (m_composer);
      ioa::scheduler.schedule_compose<composer, trigger, trigger::request_type, ioa_clock, ioa_clock::request_type, compose_callback1> (m_composer.m_trigger_handle, &trigger::request, m_composer.m_ioa_clock_handle, &ioa_clock::request, cb);
    }
  };

  class compose_callback1 {
  private:
    composer& m_composer;

  public:
    compose_callback1 (composer& composer) :
      m_composer (composer)
    { }

    void operator() (const ioa::compose_result& r) {
      assert (r.type == ioa::COMPOSED);
      compose_callback2 cb (m_composer);
      ioa::scheduler.schedule_compose<composer, ioa_clock, ioa_clock::clock_type, display, display::clock_type, compose_callback2> (m_composer.m_ioa_clock_handle, &ioa_clock::clock, m_composer.m_display_handle, &display::clock, cb);

    }
  };

  class compose_callback2 {
  private:
    composer& m_composer;

  public:
    compose_callback2 (composer& composer) :
      m_composer (composer)
    { }

    void operator() (const ioa::compose_result& r) {
      assert (r.type == ioa::COMPOSED);
      //compose_callback2 cb (m_composer);
      //ioa::scheduler.schedule_compose<composer, ioa_clock, ioa_clock::clock_type, display, display::clock_type, compose_callback2> (m_composer.m_ioa_clock_handle, &ioa_clock::clock, m_composer.m_display_handle, &display::clock, cb);

    }
  };


  void init_ () {
    trigger_callback cb (*this);
    ioa::scheduler.schedule_create<composer, trigger, trigger_callback> (new trigger(), cb);
  }

public:

  // TODO: This should have a trait that prevents composition.
  ioa::internal_wrapper<composer, &composer::init_> init;  

  composer () :
    init (*this)
  { }
};


int
main () {
  ioa::scheduler.run (new composer ());
  return 0; 
}
