#ifndef __bf_automaton_hpp__
#define __bf_automaton_hpp__

#include <ioa.hpp>

#include <queue>
#include <map>

class bf_automaton :
  public ioa::automaton_interface {
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
    bool b =  (ioa::bind_count (&bf_automaton::send, j) != 0) && !q->empty ();
    return b;
  }

  size_t send_action (size_t j) {
    size_t retval = m_send[j]->front();
    m_send[j]->pop();
    schedule();
    return retval;
  }

  void receive_action (const size_t& w, size_t j) {
    size_t wght = m_weight.find(j)->second;
    if (w + wght < m_dist) {
      std::cout << "Node " << m_i << "'s old parent is " << m_parent << " and old dist is " << m_dist << std::endl;
      m_dist = w + wght;
      m_parent = j;
      std::cout << "Node " << m_i << "'s new parent is " << j << " and new dist is " << m_dist << std::endl;
    }

    for (std::set<size_t>::const_iterator pos = m_nbrs.begin (); pos != m_nbrs.end (); ++pos) {
      if (*pos != j) {
        m_send[*pos]->push (m_dist);
      }
    }

    schedule();
  }

  void schedule () {
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
      if (send_precondition (*pos)) {
        ioa::schedule (&bf_automaton::send, *pos);
      }
    }
  }

public:
  bf_automaton (size_t i, size_t i0, const std::set<size_t>& nbrs, const std::map<size_t, size_t>& weight) :
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

    //we add a distq to each member of m_sent
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
      m_send.insert(std::make_pair(*pos, new distq ()));
      //If we are the root, add 0 to each message queue
      if(i == i0) {
        m_send[*pos]->push (0);
      }
    }

    schedule();
  }

  ~bf_automaton () {
     for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); ++pos) {
       delete m_send[*pos];
    }
  }


  V_P_OUTPUT (bf_automaton, send, size_t, size_t);
  V_P_INPUT (bf_automaton, receive, size_t, size_t);
};

#endif

