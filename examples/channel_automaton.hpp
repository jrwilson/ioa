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
  public ioa::dispatching_automaton
{
private:
  std::queue<T> m_queue;

  V_UP_INPUT (channel_automaton, send, T, t) {
    m_queue.push (t);
    schedule ();
  }
  
  bool receive_precondition () const {
    return !m_queue.empty () && receive.is_bound ();
  }

  V_UP_OUTPUT (channel_automaton, receive, T) {
    std::pair<bool, T> retval;

    if (receive_precondition ()) {
      retval = std::make_pair (true, m_queue.front ());
      m_queue.pop ();
    }

    schedule ();
    return retval;
  }


  void schedule () {
    if (receive_precondition ()) {
      ioa::scheduler::schedule (this, &channel_automaton::receive);
    }
  }

public:

  channel_automaton () :
    ACTION (channel_automaton, send),
    ACTION (channel_automaton, receive)
  { }
  
  void init () { }
};

#endif
