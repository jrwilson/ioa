#ifndef __aba_automaton_hpp__
#define __aba_automaton_hpp__

/*
  AsynchBcastAck Automaton
  Distributed Algorithms, p. 499-500.
*/

#include <ioa/ioa.hpp>
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <queue>

enum message_type_t {
    BCAST,
    ACK
};

typedef std::pair<message_type_t, size_t> message_t;

class asynch_bcast_ack_automaton:
  public ioa::automaton {

private:

  typedef std::queue<message_t> msgq;

  size_t m_i;		//my ID
  size_t m_i0;	//the ID of the root
  std::set<size_t> m_nbrs;	//the set of (the IDS of) all my neighbors

  size_t m_val;			//initially the value to be broadcast if i=i0, else 'null'
  size_t m_parent;		//the id of my parent
  bool m_reported;		// true if everyone (except my parent) has acknowledged me
  std::set<size_t> m_acked;	//the set of (the IDs of) all nodes that have acknowledged me
  std::map<size_t, msgq *> m_send;	//set of message queues; if i=i0 then each queue initially contains the single elt. ("bcast",w); otherwise each queue is empty

public:
  asynch_bcast_ack_automaton (size_t i, size_t i0, const std::set<size_t>& nbrs) :
    m_i(i),
    m_i0(i0),
    m_nbrs(nbrs),
    m_val (-1),
    m_parent(-1),
    m_reported(false)
  {
    // Initialize m_val because we are the root.
    if (m_i == m_i0) {
      m_val = rand ();
    }

    //If this is the root, broadcast to everyone; otherwise, do nothing.
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
      m_send.insert(make_pair(*pos, new msgq ()));
      //Add a BCAST message to each message queue.
      if(i == i0) {
	m_send[*pos]->push (message_t (BCAST, m_val));
      }
    }

    schedule ();
  }

  ~asynch_bcast_ack_automaton ()
  {
     for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
       delete m_send[*pos];
    }
  }

private:
  void schedule () const {
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
      if (send_precondition (*pos)) {
        ioa::schedule (&asynch_bcast_ack_automaton::send, *pos);
      }
    }

    if (report_precondition()) ioa::schedule(&asynch_bcast_ack_automaton::report);
  }

  bool send_precondition (size_t j) const {
    msgq* q = m_send.find(j)->second;
    bool b =  (ioa::binding_count (&asynch_bcast_ack_automaton::send, j) != 0) && !q->empty ();
    return b;
  }

  message_t send_effect (size_t j) {
    message_t retm = m_send[j]->front();
    m_send[j]->pop();
    return retm;
  }

  void send_schedule (size_t) const {
    schedule ();
  }

public:
  V_P_OUTPUT (asynch_bcast_ack_automaton, send, message_t, size_t);             //MANUAL NEEDS TO SAY WHAT THESE ARE

private:
  void receive_effect (const message_t& m, size_t j) {

    if (m.first == BCAST) {          //if receiving a broadcast:

      if (m_val == static_cast<size_t> (-1)) {
	m_val = m.second;
	m_parent = j;
	std::cout << m_i << " received BCAST from new parent " << j << std::endl;
	for (std::set<size_t>::const_iterator pos = m_nbrs.begin (); pos != m_nbrs.end (); ++pos) {
	  if (*pos != j) {
	    m_send[*pos]->push (message_t (BCAST, m_val));
	  }
	}
      }
      else {
	std::cout << m_i << " received BCAST from non-parent " << j << std::endl;
	m_send[j]->push (message_t (ACK, 0));
      }
    }
    else {
      std::cout << m_i << " received ACK from " << j << std::endl;
      m_acked.insert (j);
    }
  }

  void receive_schedule (size_t) const {
    schedule ();
  }

public:
  V_P_INPUT (asynch_bcast_ack_automaton, receive, message_t, size_t);

private:
  bool report_precondition () const {
    if (m_i != m_i0){
      //ASSERTION: if m_nbrs.size() is 1 greater than m_acked.size(), then all my neighbors except my parent have acknowledged me
      return m_parent != static_cast<size_t>(-1) &&
	(m_nbrs.size() - m_acked.size() == 1) &&
	m_acked.find (m_parent) == m_acked.end () &&
	!m_reported;
    }
    else{
      //ASSERTION: if m_nbrs.size() and m_acked.size() are equal, then all my neighbors have acknowledged me
      return (m_nbrs.size() == m_acked.size()) && !m_reported;
    }
  }

  void report_effect ()  {
    if (m_i != m_i0) {
      // I am not the root.
      m_send[m_parent]->push (message_t (ACK, 0));
      m_reported = true;
    }
    else {
      m_reported = true;
    }
  }

  void report_schedule () const {
    schedule ();
  }

  UP_INTERNAL (asynch_bcast_ack_automaton, report);
};

#endif
