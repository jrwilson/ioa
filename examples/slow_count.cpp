#include <ioa.hpp>

#include <iostream>

class count_to_ten :
  public ioa::automaton_interface
{
private:
  int m_count;

  bool increment_precondition () const {
    return m_count <= 10;
  }

  void increment_action () {
    std::cout << m_count << std::endl;
    if (increment_precondition ()) {
      ++m_count;
    }
    schedule ();
  }

  UP_INTERNAL (count_to_ten, increment);

  void schedule () const {
    if (increment_precondition ()) {
      ioa::scheduler::schedule_after (&count_to_ten::increment, ioa::time (1, 0));
    }
  }

public:
  count_to_ten () :
    m_count (1)
  {
    schedule ();
  }
};

int
main () {
  ioa::scheduler::run (ioa::make_generator<count_to_ten> ());
  return 0; 
}

