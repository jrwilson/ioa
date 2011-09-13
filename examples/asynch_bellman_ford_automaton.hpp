#ifndef __bf_automaton_hpp__
#define __bf_automaton_hpp__

/*
  AsynchBellmanFord Automaton
  Distributed Algorithms, p. 507.
*/

#include <ioa/ioa.hpp>

#include <queue>
#include <map>

class asynch_bellman_ford_automaton :
  public virtual ioa::automaton {
private:
  typedef std::queue<size_t> distq;

  size_t m_i;
  size_t m_i0;
  std::set<size_t> m_nbrs;
  std::map<size_t, size_t> m_weight;

  size_t m_dist;
  size_t m_parent;
  std::map<size_t, distq *> m_send;

  bool send_precondition (size_t j) const {
    distq* q = m_send.find(j)->second;
    bool b =  (ioa::binding_count (&asynch_bellman_ford_automaton::send, j) != 0) && !q->empty ();
    return b;
  }

  size_t send_effect (size_t j) {
    size_t retval = m_send[j]->front();
    m_send[j]->pop();
    return retval;
  }

  void send_schedule (size_t) const {
    schedule ();
  }

  void receive_effect (const size_t& w, size_t j) {
    size_t wght = m_weight.find(j)->second;
    if (w + wght < m_dist) {
      std::cout << m_i << ": (parent=" << m_parent << ",dist=" << m_dist << ") -> ";
      m_dist = w + wght;
      m_parent = j;
      std::cout << "(parent=" << m_parent << ",dist=" << m_dist << ")" << std::endl;
      
      for (std::set<size_t>::const_iterator pos = m_nbrs.begin (); pos != m_nbrs.end (); ++pos) {
	if (*pos != j) {
	  m_send[*pos]->push (m_dist);
	}
      }  
    }
  }

  void receive_schedule (size_t) const {
    schedule ();
  }

  void schedule () const {
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
      if (send_precondition (*pos)) {
        ioa::schedule (&asynch_bellman_ford_automaton::send, *pos);
      }
    }
  }

public:
  asynch_bellman_ford_automaton (size_t i, size_t i0, const std::set<size_t>& nbrs, const std::map<size_t, size_t>& weight) :
    m_i(i),
    m_i0(i0),
    m_nbrs(nbrs),
    m_weight(weight),
    m_dist(-1),
    m_parent(-1)
  {
    if(m_i == m_i0) {
      m_dist = 0;
    }

    //we add a distq to each member of m_send
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
      m_send.insert(std::make_pair(*pos, new distq ()));
      //If we are the root, add 0 to each message queue
      if(i == i0) {
        m_send[*pos]->push (0);
      }
    }
    schedule ();
  }

  ~asynch_bellman_ford_automaton () {
     for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
       delete m_send[*pos];
    }
  }


  V_P_OUTPUT (asynch_bellman_ford_automaton, send, size_t, size_t);
  V_P_INPUT (asynch_bellman_ford_automaton, receive, size_t, size_t);
};

#endif

