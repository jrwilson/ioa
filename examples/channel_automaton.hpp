#ifndef __channel_automaton_hpp__
#define __channel_automaton_hpp__

#include <queue>
#include <ioa.hpp>

/*
  Channel I/O Automaton
  Distributed Algorithms, p. 204.
*/

template <class T>
class channel_automaton :
  public ioa::automaton
{
private:
  std::queue<T> m_queue;

  void send_action (const T& t) {
    m_queue.push (t);
    schedule ();
  }
  
  bool receive_precondition () const {
    return !m_queue.empty () && ioa::bind_count (&channel_automaton::receive) != 0;
  }

  T receive_action () {
    T retval =  m_queue.front ();
    m_queue.pop ();
    schedule ();
    return retval;
  }

  void schedule () {
    if (receive_precondition ()) {
      ioa::schedule (&channel_automaton::receive);
    }
  }

public:

  channel_automaton ()
  {
    schedule ();
  }

  V_UP_INPUT (channel_automaton, send, T);
  V_UP_OUTPUT (channel_automaton, receive, T);

};

#endif
