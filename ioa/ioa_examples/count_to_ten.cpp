#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class count_to_ten :
  public ioa::automaton_interface
{
private:
  int m_count;

  void init () {
    ioa::scheduler.schedule (this, &count_to_ten::increment);
  }

  void increment_ () {
    ++m_count;
    std::cout << m_count << std::endl;
    if (m_count < 10) {
      ioa::scheduler.schedule (this, &count_to_ten::increment);
    }
  }
  ioa::internal_wrapper<count_to_ten, &count_to_ten::increment_> increment;

public:
  count_to_ten ()
    : m_count (0),
      increment (*this)
  { }
};

int
main () {
  ioa::scheduler.run (new count_to_ten ());
  return 0; 
}
