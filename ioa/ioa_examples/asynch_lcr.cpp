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
    return uuid_compare (u, x.u) == 0;
  }

  void print_on (std::ostream& os) const {
    char s[37];
    uuid_unparse (u, s);
    os << s;
  }
};

std::ostream& operator<< (std::ostream& strm,
			  const uuid& u) {
  u.print_on (strm);
  return strm;
}

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
    return !m_send.empty () && send.is_bound ();
  }

  V_UP_OUTPUT (asynch_lcr_automaton, send, uuid) {

    std::pair<bool, uuid> retval;

    if (send_precondition ()) {
      retval = std::make_pair (true, m_send.front ());
      m_send.pop ();
    }

    schedule ();
    return retval;
  }

  bool leader_precondition () const {
    return m_status == CHOSEN && leader.is_bound ();
  }

  UV_UP_OUTPUT (asynch_lcr_automaton, leader) {
    bool retval = false;

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

template <size_t N>
class composer :
  public ioa::dispatching_automaton
{
private:

  UV_P_INPUT (composer, leader, size_t, i) {
    std::cout << i << " is the leader." << std::endl;
  }

  typedef ioa::instance_generator<asynch_lcr_automaton> asynch_lcr_automaton_generator_type;
  typedef ioa::instance_generator<channel_automaton<uuid> > channel_automaton_generator_type;

  typedef ioa::automaton_helper<composer, asynch_lcr_automaton_generator_type> asynch_lcr_automaton_helper_type;
  typedef ioa::automaton_helper<composer, channel_automaton_generator_type> channel_automaton_helper_type;
  typedef ioa::self_helper<composer> composer_helper_type;

  typedef ioa::bind_helper<composer, asynch_lcr_automaton_helper_type, asynch_lcr_automaton::send_type, channel_automaton_helper_type, channel_automaton<uuid>::send_type> send_bind_helper_type;
  typedef ioa::bind_helper<composer, channel_automaton_helper_type, channel_automaton<uuid>::receive_type, asynch_lcr_automaton_helper_type, asynch_lcr_automaton::receive_type> receive_bind_helper_type;
  typedef ioa::bind_helper<composer, asynch_lcr_automaton_helper_type, asynch_lcr_automaton::leader_type, composer_helper_type, typename composer::leader_type> leader_bind_helper_type;

  std::vector<asynch_lcr_automaton_helper_type*> asynch_lcr_automaton_helpers;
  std::vector<channel_automaton_helper_type*> channel_automaton_helpers;
  composer_helper_type composer_helper;
  std::vector<send_bind_helper_type*> send_bind_helpers;
  std::vector<receive_bind_helper_type*> receive_bind_helpers;
  std::vector<leader_bind_helper_type*> leader_bind_helpers;
  
public:

  composer () :
    ACTION (composer, leader),
    composer_helper (this)
  {
    for (size_t i = 0; i < N; ++i) {
      asynch_lcr_automaton_helpers.push_back (new asynch_lcr_automaton_helper_type (this, asynch_lcr_automaton_generator_type ()));
      channel_automaton_helpers.push_back (new channel_automaton_helper_type (this, channel_automaton_generator_type ()));
    }

    for (size_t i = 0; i < N; ++i) {
      send_bind_helpers.push_back (new send_bind_helper_type (this,
							      asynch_lcr_automaton_helpers[i],
							      &asynch_lcr_automaton::send,
							      channel_automaton_helpers[i],
							      &channel_automaton<uuid>::send));
      receive_bind_helpers.push_back (new receive_bind_helper_type (this,
								    channel_automaton_helpers[i],
								    &channel_automaton<uuid>::receive,
								    asynch_lcr_automaton_helpers[(i + 1) % N],
								    &asynch_lcr_automaton::receive));
      leader_bind_helpers.push_back (new leader_bind_helper_type (this,
								  asynch_lcr_automaton_helpers[i],
								  &asynch_lcr_automaton::leader,
								  &composer_helper,
								  &composer::leader,
								  i));
    }

  }

  void init () {
    for (size_t i = 0; i < N; ++i) {
      asynch_lcr_automaton_helpers[i]->create ();
      channel_automaton_helpers[i]->create ();
      send_bind_helpers[i]->bind ();
      receive_bind_helpers[i]->bind ();
      leader_bind_helpers[i]->bind ();
    }
    composer_helper.create ();
  }

};

int
main () {
  ioa::scheduler.run (ioa::instance_generator<composer<100> > ());
  return 0; 
}
