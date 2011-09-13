#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class producer_automaton :
  public virtual ioa::automaton
{
private:
  int m_count;

public:
  producer_automaton () :
    m_count (1) { }
  
private:
  bool produce_precondition () const {
    return m_count <= 10 &&
      ioa::binding_count (&producer_automaton::produce) == 2;
  }

  int produce_effect () {
    int retval = m_count++;
    std::cout << "producing " << retval << std::endl;
    return retval;
  }

  void produce_schedule () const {
    if (produce_precondition ()) {
      ioa::schedule (&producer_automaton::produce);
    }
  }

public:
  V_UP_OUTPUT (producer_automaton, produce, int);
};

class consumer_automaton :
  public virtual ioa::automaton
{
private:
  void consume_effect (const int& val) {
    std::cout << ioa::get_aid () << " consuming " << val << std::endl;
  }

  void consume_schedule () const { }

public:
  V_UP_INPUT (consumer_automaton, consume, int);
};

class producer_consumer_automaton :
  public virtual ioa::automaton
{
public:
  producer_consumer_automaton () {
    ioa::automaton_manager<producer_automaton>* producer =
      ioa::make_automaton_manager (this,
	  ioa::make_generator<producer_automaton> ());

    ioa::automaton_manager<consumer_automaton>* consumer1 =
      ioa::make_automaton_manager (this,
          ioa::make_generator<consumer_automaton> ());

    ioa::automaton_manager<consumer_automaton>* consumer2 =
      ioa::make_automaton_manager (this,
          ioa::make_generator<consumer_automaton> ());

    ioa::make_binding_manager (this,
			       producer, &producer_automaton::produce,
			       consumer1, &consumer_automaton::consume);

    ioa::make_binding_manager (this,
			       producer, &producer_automaton::produce,
			       consumer2, &consumer_automaton::consume);
  }

};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<producer_consumer_automaton> ());
  return 0; 
}
