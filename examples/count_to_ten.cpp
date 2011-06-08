/*
  An automaton that counts to ten and stops.
*/

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

  DECLARE_UP_INTERNAL (count_to_ten, increment);

  void schedule () const {
    if (increment_precondition ()) {
      ioa::scheduler::schedule (&count_to_ten::increment);
    }
  }

public:
  count_to_ten () :
    m_count (1)
  {
    schedule ();
  }
};

DEFINE_UP_INTERNAL (count_to_ten, increment) {
  std::cout << m_count << std::endl;
  if (increment_precondition ()) {
    ++m_count;
  }
  
  schedule ();
}

int
main () {
  ioa::scheduler::run (ioa::make_generator<count_to_ten> ());
  return 0; 
}

