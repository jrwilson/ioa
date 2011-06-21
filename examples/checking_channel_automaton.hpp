#ifndef __checking_channel_automaton_hpp__
#define	__checking_channel_automaton_hpp__

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

  std::queue<T> m_queue;

  void send_effect (const T& t) {
    m_queue.push (t);
    schedule ();
  }

  bool receive_precondition () const {
    return channel_state == RECV_READY && ioa::bind_count (&checking_channel_automaton::receive) != 0;
  }

  T receive_effect () {
    T retval =  m_queue.front ();
    m_queue.pop ();
    schedule ();
    return retval;
  }

  bool send_complete_precondition () const {
      return (channel_state == SEND_COMPLETE_READY && ioa::bind_count (&checking_channel_automaton::send_complete) != 0);
  }

  void send_complete_effect () {
    channel_state = SEND_WAIT;
    schedule ();
  }

  void schedule () {
    if (receive_precondition ()) {
      ioa::schedule (&checking_channel_automaton::receive);
    }
    if(send_complete_precondition ()) {
        ioa::schedule (&checking_channel_automaton::send_complete);
    }
  }

public:

  checking_channel_automaton () :
    channel_state(SEND_WAIT)
  {
    schedule ();
  }

  V_UP_INPUT (checking_channel_automaton, send, T);
  V_UP_OUTPUT (checking_channel_automaton, receive, T);
  UV_UP_OUTPUT (checking_channel_automaton, send_complete);
};

#endif
