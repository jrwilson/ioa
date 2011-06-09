/*
  An automaton that counts to ten and stops.
*/

// Includes all of the classes needed to define I/O Automata.
#include <ioa.hpp>
// For printing (cout).
#include <iostream>

// Automata are defined as classes that inherit from ioa::automaton_interface.
// ioa::dispatching_automaton is the standard implementation of ioa::automaton_interface.
// You should always use ioa::dispatching_automaton!
class count_to_ten :
  public ioa::automaton_interface
{
private:
  // The state of our automaton consists of an integer that holds the current count.
  int m_count;

  // Increments the counter when the count is less than or equal to 10.
  bool increment_precondition () const {
    return m_count <= 10;
  }

  void increment_action () {
    std::cout << m_count << std::endl;
    ++m_count;
    schedule ();
  }

  // Declares an unparamaterized internal action called "increment".
  UP_INTERNAL (count_to_ten, increment);

  // Tells the runtime scheduler that we would like it to execute the increment action.
  void schedule () const {
    if (increment_precondition ()) {
      ioa::scheduler::schedule (&count_to_ten::increment);
    }
  }

public:
  // Initializes our counter to one and initializes the ACTION data structure.

  count_to_ten () :
    m_count (1)
  {
    // Informs the scheduler that we would like to execute the increment action.
    // The scheduler cannot be invoked in the constructor.
    schedule ();
  }
};

int
main () {
  // To create an automaton the system requires a generator which returns a new automaton.
  // Make generator is a helper function which creates this generator.
  // Run starts the scheduler.
  ioa::scheduler::run (ioa::make_generator<count_to_ten> ());
  return 0; 
}

