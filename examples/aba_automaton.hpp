#ifndef __aba_automaton_hpp__
#define __aba_automaton_hpp__

#include <ioa.hpp>

#include <iostream>
#include <set>
#include <map>
#include <queue>

typedef enum {
    BCAST,
    ACK
  } message_t;

bool debug = true;
bool debug2 = false;
bool debug3 = false;

class aba_automaton:
  public ioa::automaton_interface {
  
private:

  
  typedef std::queue<message_t> msgq;
  
  size_t val;			//initially the value to be broadcast if i=i0, else 'null' (FIXME!!!!!)
  size_t m_parent;		//the id of my parent
  bool m_reported;		// true if everyone (except my parent) has acknowledged me
  std::map<size_t, msgq *> m_send;	//set of message queues; if i=i0 then each queue initially contains the single elt. ("bcast",w); otherwise each queue is empty
  std::set<size_t> m_acked;	//the set of (the IDs of) all nodes that have acknowledged me
  std::set<size_t> m_nbrs;	//the set of (the IDS of) all my neighbors
  
  size_t m_i;		//my ID
  size_t m_i0;	//the ID of the root
  
  
  bool send_precondition (size_t j) const {
    if (debug3) std::cout << "Node " << m_i << " entering send_precondition" << std::endl;
    msgq* q = m_send.find(j)->second;
    if (debug2) std::cout << "Found j's queue in m_send" << std::endl;
    bool b =  (ioa::scheduler::bind_count (&aba_automaton::send, j) != 0) && q->size() > 0;
    if (debug3) std::cout << "i: " << m_i << " and j: " << j << ": " << b << std::endl;
    return b;
  }

  message_t send_action (size_t j) {
    message_t retm = m_send.find(j)-> second->front();
    m_send.find(j)->second->pop();
    if (debug) std::cout << "Node "<< m_i << " scheduling send " << retm << " to node " << j << std::endl;

    schedule();
    return retm;
  }

  void receive_action (const message_t& m, size_t j) {
    if(m == BCAST){          //if receiving a broadcast:
      if (debug) std::cout << "Node " << m_i << " receiving broadcast from node " << j << std::endl;
      if(m_i != m_i0 && m_parent == static_cast<size_t>(-1)){   //if I am not the root and I have no parent yet:
	if (debug) std::cout << "Node " << m_i << "'s parent is node " << j << std::endl;
	m_parent = j;
	for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
	  //Add a BCAST message to each message queue except that of the parent.
	  if (*pos != j)
	    m_send[*pos]->push(BCAST);
	}
      }
      else{   //I have a parent:
	if (debug) std::cout << "Node "<< m_i << " will acknowledge node " << j << std::endl;
	m_send.find(j)->second->push(ACK);    //Acknowledge the node that has broadcast to me.
      }
    }

    else{      //if receiving an acknowledgment:
      if (debug) std::cout << "Node " << m_i << " receiving acknowledgement from node " << j << std::endl;
      m_acked.insert(j);
    }

    schedule();
  }
  
  bool report_precondition () const {
    if (debug3) std::cout << "Node " << m_i << " entering report_precondition" << std::endl;
    if (m_i == m_i0){
      //ASSERTION: if m_nbrs.size() is 1 greater than m_acked.size(), then all my neighbors except my parent have acknowledged me
      return m_parent != static_cast<size_t>(-1) && (m_nbrs.size() - m_acked.size() == 1) && !m_reported;
    }
    else{
      //ASSERTION: if m_nbrs.size() and m_acked.size() are equal, then all my neighbors have acknowledged me
      return (m_nbrs.size() == m_acked.size())&& !m_reported;
    }
  }

  void report_action ()  {
    if (debug3) std::cout << "Node " << m_i << " entering report_action" << std::endl;
    if (m_parent != static_cast<size_t>(-1)) m_send.find(m_parent)->second->push(ACK);
    m_reported = true;
    if (m_i == m_i0) std::cout << "Root received all acknowledgements" << std::endl;
    schedule();
  }

  void schedule(){
    if (debug3) std::cout <<"Node "<< m_i << " entering schedule" << std::endl;
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
      if (debug2) std::cout << *pos << std::endl;
      if (send_precondition (*pos)) {
        ioa::scheduler::schedule (&aba_automaton::send, *pos);
      }
    }

    if (report_precondition()) ioa::scheduler::schedule(&aba_automaton::report);
  }	
  
  UP_INTERNAL (aba_automaton, report);  
  
public:
  aba_automaton (size_t i, size_t i0, const std::set<size_t>& nbrs) :
    m_parent(-1),
    m_reported(false),
    m_nbrs(nbrs),
    m_i(i),
    m_i0(i0)
  {
    //If this is the root, broadcast to everyone; otherwise, do nothing.    
    
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
      m_send.insert(make_pair(*pos, new msgq ()));
      //Add a BCAST message to each message queue.
      if(i == i0) {
	m_send[*pos]->push(BCAST);
      }
    }
    
    if (debug) std::cout << "I am node " << m_i << std::endl;
    
    schedule();
  }

  ~aba_automaton ()
  {
     for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
       delete m_send[*pos];
    }
  }
  V_P_OUTPUT (aba_automaton, send, message_t, size_t);             //MANUAL NEEDS TO SAY WHAT THESE ARE
  V_P_INPUT (aba_automaton, receive, message_t, size_t);

  
  
};

#endif
