#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class count_to_ten :
  public ioa::dispatching_automaton
{
private:
  int m_count;

  UP_INTERNAL (count_to_ten, increment) {
    ++m_count;
    std::cout << m_count << std::endl;
    if (m_count < 10) {
      ioa::scheduler.schedule (this, &count_to_ten::increment, ioa::time (1, 0));
    }
  }

public:
  count_to_ten ()
    : m_count (0),
      ACTION (count_to_ten, increment)
  { }

  void init () {
    ioa::scheduler.schedule (this, &count_to_ten::increment);
  }
};

int
main () {
  ioa::scheduler.run (ioa::instance_generator<count_to_ten> ());
  return 0; 
}
