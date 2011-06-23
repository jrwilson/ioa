#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>

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
  bool increment_precondition () const {
    return m_count <= 10;
  }

  void increment_effect () {
    std::cout <<
      "automaton: " << ioa::get_aid () <<
      " count: " << m_count << std::endl;
    ++m_count;
    schedule ();
  }

  UP_INTERNAL (count_to_ten_automaton, increment);

  void schedule () const {
    if (increment_precondition ()) {
      ioa::schedule (&count_to_ten_automaton::increment);
    }
  }
};

class two_counter_automaton :
  public ioa::automaton
{
public:
  two_counter_automaton () {
    new ioa::automaton_manager<count_to_ten_automaton> (
      this,
      ioa::make_generator<count_to_ten_automaton> ()
    );

    new ioa::automaton_manager<count_to_ten_automaton> (
      this,
      ioa::make_generator<count_to_ten_automaton> ()
    );
  }

};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<two_counter_automaton> ());
  return 0; 
}

