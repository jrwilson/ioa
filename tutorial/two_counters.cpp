#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class count_to_ten_automaton :
  public virtual ioa::automaton
{
private:
  int m_count;

public:
  count_to_ten_automaton () :
    m_count (1)
  {
    increment_schedule ();
  }
  
private:
  bool increment_precondition () const {
    return m_count <= 10;
  }

  void increment_effect () {
    std::cout <<
      "automaton: " << ioa::get_aid () <<
      " count: " << m_count << std::endl;
    ++m_count;
  }

  void increment_schedule () const {
    if (increment_precondition ()) {
      ioa::schedule (&count_to_ten_automaton::increment);
    }
  }

  UP_INTERNAL (count_to_ten_automaton, increment);
};

class two_counter_automaton :
  public virtual ioa::automaton
{
public:
  two_counter_automaton () {
    ioa::make_automaton_manager (this,
        ioa::make_generator<count_to_ten_automaton> ());
    ioa::make_automaton_manager (this,
	ioa::make_generator<count_to_ten_automaton> ());
  }

};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<two_counter_automaton> ());
  return 0; 
}
