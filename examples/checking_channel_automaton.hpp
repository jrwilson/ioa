#ifndef __checking_channel_automaton_hpp__
#define	__checking_channel_automaton_hpp__

#include <ioa/ioa.hpp>
#include <queue>

template <class T>
class checking_channel_automaton :
  public ioa::automaton
{
private:
  enum channel_state {
      SEND_WAIT,
      RECV_READY,
      SEND_COMPLETE_READY
  };

  channel_state m_state;
  std::queue<T> m_queue;

public:
  checking_channel_automaton () :
    m_state (SEND_WAIT)
  {
    schedule ();
  }

private:
  void send_effect (const T& t) {
    if (m_state == SEND_WAIT) {
      m_queue.push (t);
      m_state = RECV_READY;
    }
    schedule ();
  }

public:

  V_UP_INPUT (checking_channel_automaton, send, T);

private:

  bool receive_precondition () const {
    return m_state == RECV_READY && ioa::bind_count (&checking_channel_automaton::receive) != 0;
  }

  T receive_effect () {
    T retval =  m_queue.front ();
    m_queue.pop ();
    m_state = SEND_COMPLETE_READY;
    schedule ();
    return retval;
  }

public:

  V_UP_OUTPUT (checking_channel_automaton, receive, T);

private:

  bool send_complete_precondition () const {
      return (m_state == SEND_COMPLETE_READY && ioa::bind_count (&checking_channel_automaton::send_complete) != 0);
  }

  void send_complete_effect () {
    m_state = SEND_WAIT;
    schedule ();
  }

public:

  UV_UP_OUTPUT (checking_channel_automaton, send_complete);

private:

  void schedule () const {
    if (receive_precondition ()) {
      ioa::schedule (&checking_channel_automaton::receive);
    }
    if(send_complete_precondition ()) {
        ioa::schedule (&checking_channel_automaton::send_complete);
    }
  }

};

#endif
