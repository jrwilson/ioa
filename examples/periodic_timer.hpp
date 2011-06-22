#ifndef __periodic_timer_hpp__
#define __periodic_timer_hpp__

class periodic_timer :
  public ioa::automaton
{

public:
  periodic_timer ()
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
      ioa::schedule_after (&periodic_timer::interrupt, ioa::time (1, 0));
    }
  }

public:
  
  UV_UP_OUTPUT (periodic_timer, interrupt);
};

#endif
