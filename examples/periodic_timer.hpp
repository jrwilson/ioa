#ifndef __periodic_timer_hpp__
#define __periodic_timer_hpp__

class periodic_timer :
  public ioa::automaton
{

private:
  enum clock_state {
    SET_WAIT,
    INTERRUPT_READY
  };

  const ioa::time m_interval;
  clock_state m_state;
public:
  periodic_timer (const ioa::time& t) :
    m_interval (t),
    m_state (SET_WAIT)
  {
    schedule ();
  }

private:
    
  bool interrupt_precondition () const {
    return ioa::bind_count (&periodic_timer::interrupt) != 0 && m_state == INTERRUPT_READY;
  }

  void interrupt_effect () { 
    m_state = SET_WAIT;
    schedule ();
  }

  void set_effect () {
    if (m_state == SET_WAIT) {
      m_state = INTERRUPT_READY;
    }
    schedule ();
  }
  
  void schedule () const {
    if (interrupt_precondition ()) {
      ioa::schedule_after (&periodic_timer::interrupt, m_interval);
    }
  }

public:
  UV_UP_OUTPUT (periodic_timer, interrupt);
  UV_UP_INPUT (periodic_timer, set);
};

#endif
