#ifndef __channel_automaton_hpp__
#define __channel_automaton_hpp__

/*
  Channel I/O Automaton
  Distributed Algorithms, p. 204.
*/

#include <queue>
#include <ioa/ioa.hpp>

template <class T>
class channel_automaton :
  public virtual ioa::automaton
{
private:
  std::queue<T> m_queue;

  void send_effect (const T& t) {
    m_queue.push (t);
  }
  
  void send_schedule () const {
    receive_schedule ();
  }

public:
  V_UP_INPUT (channel_automaton, send, T);

private:
  bool receive_precondition () const {
    return !m_queue.empty () && ioa::binding_count (&channel_automaton::receive) != 0;
  }

  T receive_effect () {
    T retval =  m_queue.front ();
    m_queue.pop ();
    return retval;
  }

  void receive_schedule () const {
    if (receive_precondition ()) {
      ioa::schedule (&channel_automaton::receive);
    }
  }

public:
  V_UP_OUTPUT (channel_automaton, receive, T);
};

#endif
