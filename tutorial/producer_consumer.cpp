#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class producer_automaton :
  public ioa::automaton
{
private:
  int m_count;

public:
  producer_automaton () :
    m_count (1) {
    produce_schedule ();
  }
  
private:
  bool produce_precondition () const {
    return m_count <= 10;
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
  public ioa::automaton
{
private:
  void consume_effect (const int& val) {
    std::cout << "consuming " << val << std::endl;
  }

  void consume_schedule () const { }

public:
  V_UP_INPUT (consumer_automaton, consume, int);
};

class producer_consumer_automaton :
  public ioa::automaton
{
public:
  producer_consumer_automaton () {
    ioa::automaton_manager<producer_automaton>* producer =
      ioa::make_automaton_manager (this,
	  ioa::make_generator<producer_automaton> ());

    ioa::automaton_manager<consumer_automaton>* consumer =
      ioa::make_automaton_manager (this,
          ioa::make_generator<consumer_automaton> ());

    ioa::make_binding_manager (this,
			       producer, &producer_automaton::produce,
			       consumer, &consumer_automaton::consume);
  }

};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<producer_consumer_automaton> ());
  return 0; 
}

