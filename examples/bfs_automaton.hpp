#ifndef __bfs_automaton_hpp__
#define __bfs_automaton_hpp__

#include <ioa/ioa.hpp>

#include <iostream>
#include <set>
#include <map>
#include <queue>

class bfs_automaton:
  public ioa::automaton {

private:

  typedef std::queue<size_t> distq;

  size_t m_i;
  size_t m_i0;
  std::set<size_t> m_nbrs;

  size_t m_dist;
  size_t m_parent;
  std::map<size_t, distq *> m_send;

  bool send_precondition(size_t j) const {
    distq * q = m_send.find(j)->second;
    return (ioa::bind_count (&bfs_automaton::send, j) != 0) && !q->empty();
  }

  size_t send_effect (size_t j) {
    size_t retval = m_send[j]->front();
    m_send[j]->pop();

    schedule();
    return retval;
  }

  void receive_effect (const size_t& m, size_t j){
    if (m+1 < m_dist){
      std::cerr << "Node " << m_i << "'s old dist: " << m_dist << " and new dist: " << (m+1) << std::endl;
      m_dist = m+1;
      std::cerr << "Node " << m_i << "'s old parent: " << m_parent << " and new parent: " << j << std::endl;
      m_parent = j;
      for (std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos!= m_nbrs.end(); pos++){
	if (*pos != j){
	  m_send[*pos]->push(m_dist);
	}
      }
    }

    schedule();
  }

  void schedule(){
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); pos++){
      if (send_precondition (*pos)) {
	ioa::schedule(&bfs_automaton::send, *pos);
      }
    }
  }

public:
  bfs_automaton (size_t i, size_t i0, const std::set<size_t> &nbrs) :
    m_i(i),
    m_i0(i0),
    m_nbrs(nbrs),
    m_dist(-1),
    m_parent(-1)
  {
    if (m_i == m_i0) m_dist = 0;

    for (std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); pos++){
      m_send.insert(make_pair(*pos, new distq()));
      if (i==i0){
	m_send[*pos]->push(0);
      }
    }
    schedule();
  }

  ~bfs_automaton()
  {
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin(); pos != m_nbrs.end(); pos++){
      delete m_send[*pos];
    }
  }

  V_P_OUTPUT (bfs_automaton, send, size_t, size_t);
  V_P_INPUT (bfs_automaton, receive, size_t, size_t);

};

#endif
