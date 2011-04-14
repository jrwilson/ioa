#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class count_to_ten {
private:
  int m_count;

  void init_ () {
    ioa::scheduler.schedule_internal (&count_to_ten::increment);
  }

  void increment_ () {
    ++m_count;
    std::cout << m_count << std::endl;
    if (m_count < 10) {
      ioa::scheduler.schedule_internal (&count_to_ten::increment);
    }
  }
  
public:
  ioa::internal_wrapper<count_to_ten, &count_to_ten::init_> init;
  ioa::internal_wrapper<count_to_ten, &count_to_ten::increment_> increment;
  
  count_to_ten ()
    : m_count (0),
      init (*this),
      increment (*this) { }
};

int
main () {
  ioa::scheduler.run (new count_to_ten ());
  return 0; 
}
