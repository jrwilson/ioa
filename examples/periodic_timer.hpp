#ifndef __periodic_timer_hpp__
#define __periodic_timer_hpp__

class periodic_timer :
  public ioa::automaton
{

private:
  const ioa::time m_interval;

public:
  periodic_timer (const ioa::time& t) :
    m_interval (t)
  {
    schedule ();
  }

private:
    
  bool interrupt_precondition () const {
    return true;
  }

  void interrupt_effect () {
    schedule ();
  }

  void schedule () const {
    if (interrupt_precondition ()) {
      ioa::schedule_after (&periodic_timer::interrupt, m_interval);
    }
  }

public:
  
  UV_UP_OUTPUT (periodic_timer, interrupt);
};

#endif
