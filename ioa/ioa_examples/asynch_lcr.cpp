#include <cstdlib>
#include <iostream>

#include <ioa.hpp>
#include <queue>
#include <uuid/uuid.h>

#include <boost/type_traits.hpp>

struct uuid {
  uuid_t u;

  uuid () {
    uuid_generate (u);
  }

  uuid (const uuid& x) {
    uuid_copy (u, x.u);
  }

  uuid& operator= (const uuid& x) {
    if (this != &x) {
      uuid_copy (u, x.u);
    }
    return *this;
  }

  bool operator> (const uuid& x) const {
    return uuid_compare (u, x.u) > 0;
  }

  bool operator== (const uuid& x) const {
    return uuid_compare == 0;
  }
};

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
    return !m_queue.empty ();
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
      ioa::scheduler.schedule (this, &channel_automaton::receive);
    }
  }

public:

  channel_automaton () :
    ACTION (channel_automaton, send),
    ACTION (channel_automaton, receive)
  { }
  
  void init () { }
};

/*
  AsyncLCR Automaton
  Distributed Algorithms, p. 204.
*/

class asynch_lcr_automaton :
  public ioa::dispatching_automaton
{
private:
  typedef enum {
    UNKNOWN,
    CHOSEN,
    REPORTED
  } status_t;

  uuid m_u;
  std::queue<uuid> m_send;
  status_t m_status;

  V_UP_INPUT (asynch_lcr_automaton, receive, uuid, v) {
    std::cout << __func__ << std::endl;
    
    if (v > m_u) {
      m_send.push (v);
    }
    else if (v == m_u) {
      m_status = CHOSEN;
    }
    else {
      // Do nothing.
    }
    
    schedule ();
  }

  bool send_precondition () const {
    return !m_send.empty ();
  }

  V_UP_OUTPUT (asynch_lcr_automaton, send, uuid) {
    std::cout << __func__ << std::endl;

    std::pair<bool, uuid> retval;

    if (send_precondition ()) {
      retval = std::make_pair (true, m_send.front ());
      m_send.pop ();
    }

    schedule ();
    return retval;
  }

  bool leader_precondition () const {
    return m_status == CHOSEN;
  }

  UV_UP_OUTPUT (asynch_lcr_automaton, leader) {
    std::cout << __func__ << std::endl;

    bool retval;

    if (leader_precondition ()) {
      retval = true;
      m_status = REPORTED;
    }

    schedule ();
    return retval;
  }

  void schedule () {
    if (send_precondition ()) {
      ioa::scheduler.schedule (this, &asynch_lcr_automaton::send);
    }

    if (leader_precondition ()) {
      ioa::scheduler.schedule (this, &asynch_lcr_automaton::leader);
    }
  }

public:

  asynch_lcr_automaton () :
    m_status (UNKNOWN),
    ACTION (asynch_lcr_automaton, receive),
    ACTION (asynch_lcr_automaton, send),
    ACTION (asynch_lcr_automaton, leader)
  {
    m_send.push (m_u);
  }

  void init () {
    schedule ();
  }

};

class composer :
  public ioa::dispatching_automaton
{
private:
  typedef ioa::automaton_helper<composer, ioa::instance_generator<asynch_lcr_automaton> > asynch_lcr_automaton_helper;
  typedef ioa::automaton_helper<composer, ioa::instance_generator<channel_automaton<uuid> > > channel_automaton_helper;

  asynch_lcr_automaton_helper p1;
  asynch_lcr_automaton_helper p2;
  channel_automaton_helper c1;
  channel_automaton_helper c2;
  
  typedef ioa::bind_helper<composer, asynch_lcr_automaton_helper, asynch_lcr_automaton::send_type, channel_automaton_helper, channel_automaton<uuid>::send_type> send_bind_helper;
  typedef ioa::bind_helper<composer, channel_automaton_helper, channel_automaton<uuid>::receive_type, asynch_lcr_automaton_helper, asynch_lcr_automaton::receive_type> receive_bind_helper;

  send_bind_helper p1_send;
  receive_bind_helper p1_receive;
  send_bind_helper p2_send;
  receive_bind_helper p2_receive;

public:

  composer () :
    p1 (this, ioa::instance_generator<asynch_lcr_automaton>()),
    p2 (this, ioa::instance_generator<asynch_lcr_automaton>()),
    c1 (this, ioa::instance_generator<channel_automaton<uuid> >()),
    c2 (this, ioa::instance_generator<channel_automaton<uuid> >()),
    p1_send (this, &p1, &asynch_lcr_automaton::send, &c1, &channel_automaton<uuid>::send),
    p1_receive (this, &c2, &channel_automaton<uuid>::receive, &p1, &asynch_lcr_automaton::receive),
    p2_send (this, &p2, &asynch_lcr_automaton::send, &c2, &channel_automaton<uuid>::send),
    p2_receive (this, &c1, &channel_automaton<uuid>::receive, &p2, &asynch_lcr_automaton::receive)
  {  }

  void init () {
    p1.create ();
    p2.create ();
    c1.create ();
    c2.create ();

    p1_send.bind ();
    p1_receive.bind ();
    p2_send.bind ();
    p2_receive.bind ();
  }

};

int
main () {
  ioa::scheduler.run (ioa::instance_generator<composer> ());
  return 0; 
}
