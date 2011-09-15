#ifndef __ast_automaton_hpp__
#define __ast_automaton_hpp__

/*
  AsynchSpanningTree Automaton
  Distributed Algorithms, p. 496-497.
*/

#include <ioa/ioa.hpp>

#include <set>
#include <map>

typedef enum {
  SEARCH_MESSAGE
} search_t;

class asynch_spanning_tree_automaton :
  public ioa::automaton
{

private:
  typedef enum {
    SEARCH,
    NOSEARCH
  } mode_t;
  size_t m_parent;
  bool m_reported;
  std::map<size_t, mode_t> m_send;
  std::set<size_t> m_nbrs;
  size_t m_i;
  size_t m_i0;

  bool parent_precondition () const {
    return m_parent != size_t(-1) && !m_reported && ioa::binding_count (&asynch_spanning_tree_automaton::parent) != 0;
  }

  size_t parent_effect () {
    m_reported = true;
    size_t retval = m_parent;
    return retval;
  }

  void parent_schedule () const {
    schedule ();
  }

  bool send_precondition (size_t j) const {
    //might be a different syntax than binding_count (j)
    return m_send.find (j)->second == SEARCH && ioa::binding_count (&asynch_spanning_tree_automaton::send, j) != 0;
  }

  search_t send_effect (size_t j) {
    m_send[j] = NOSEARCH;
    search_t retval = SEARCH_MESSAGE;
    return retval;
  }

  void send_schedule (size_t) const {
    schedule ();
  }

  void receive_effect (const search_t& ignored, size_t j) {
    if(m_i != m_i0 && m_parent == size_t(-1)) {
      m_parent = j;

      for(std::set<size_t>::const_iterator k = m_nbrs.begin ();
        k != m_nbrs.end ();
        ++k) {
        if(*k != j) {
          m_send[*k] = SEARCH;
        }
      }
    }
  }

  void receive_schedule (size_t) const {
    schedule ();
  }

  void schedule () const {
    if (parent_precondition ()) {
      ioa::schedule (&asynch_spanning_tree_automaton::parent);
    }
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin();
        pos != m_nbrs.end();
        ++pos) {
      if (send_precondition (*pos)) {
        ioa::schedule (&asynch_spanning_tree_automaton::send, *pos);
      }
    }
  }

public:
  asynch_spanning_tree_automaton (size_t i, size_t i0, const std::set<size_t>& nbrs) :
    m_parent(-1),
    m_reported(false),
    m_nbrs(nbrs),
    m_i(i),
    m_i0(i0)
  {
    for(std::set<size_t>::const_iterator pos = m_nbrs.begin();
        pos != m_nbrs.end();
        ++pos) {
      if(i == i0) {
        m_send[*pos] = SEARCH;
      }
      else {
        m_send[*pos] = NOSEARCH;
      }
    }

    schedule ();
  }

  V_UP_OUTPUT (asynch_spanning_tree_automaton, parent, size_t);
  V_P_OUTPUT (asynch_spanning_tree_automaton, send, search_t, size_t);
  V_P_INPUT (asynch_spanning_tree_automaton, receive, search_t, size_t);

};

#endif
