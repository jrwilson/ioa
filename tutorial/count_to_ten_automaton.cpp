#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>

// For std::cout.
#include <iostream>

class count_to_ten_automaton :
  public ioa::automaton
{
private:
  int m_count;

public:
  count_to_ten_automaton () :
    m_count (1)
  {
    schedule ();
  }
  
private:
  void schedule () const {
    if (increment_precondition ()) {
      ioa::schedule (&count_to_ten_automaton::increment);
    }
  }

  bool increment_precondition () const {
    return m_count <= 10;
  }

  void increment_effect () {
    std::cout << m_count << std::endl;
    ++m_count;
  }

  void increment_schedule () const {
    schedule ();
  }

  UP_INTERNAL (count_to_ten_automaton, increment);
};

int main () {
  ioa::global_fifo_scheduler sched;
  std::auto_ptr<ioa::typed_generator_interface<count_to_ten_automaton> > x = ioa::make_generator<count_to_ten_automaton> ();
  ioa::run (sched, x);
  return 0; 
}

