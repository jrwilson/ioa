#include <cstdlib>
#include <iostream>

#include <ioa.hpp>

class count_to_ten {
private:
  int m_count;

  void init_ (const ioa::automaton_handle<count_to_ten>) {
    ioa::scheduler.schedule_output (&count_to_ten::increment);
  }

  std::pair<bool, int> increment_ () {
    ++m_count;
    std::cout << m_count << std::endl;
    if (m_count < 10) {
      ioa::scheduler.schedule_output (&count_to_ten::increment);
    }
    return std::make_pair (true, m_count);
  }
  
public:

  ioa::input_wrapper<count_to_ten, ioa::automaton_handle<count_to_ten>, &count_to_ten::init_> init;
  ioa::output_wrapper<count_to_ten, int, &count_to_ten::increment_> increment;
  
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
