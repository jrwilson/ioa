#include <ioa/ioa.hpp>
#include <ioa/simple_scheduler.hpp>

#include <iostream>

class count_to_ten :
  public virtual ioa::automaton
{
private:
  int m_count;

  void schedule () const {
    if (increment_precondition ()) {
      ioa::schedule_after (&count_to_ten::increment, ioa::time (1, 0));
    }
  }

  bool increment_precondition () const {
    return m_count <= 10;
  }

  void increment_effect () {
    std::cout << m_count << std::endl;
    if (increment_precondition ()) {
      ++m_count;
    }
  }

  void increment_schedule () const {
    schedule ();
  }

  UP_INTERNAL (count_to_ten, increment);

public:
  count_to_ten () :
    m_count (1)
  {
    schedule ();
  }
};

int
main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<count_to_ten> ());
  return 0; 
}

