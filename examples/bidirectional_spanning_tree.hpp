#ifndef __bidirectional_spanning_tree_hpp__
#define __bidirectional_spanning_tree_hpp__

#include "channel_automaton.hpp"

#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

template <class T, size_t N, unsigned long NUMERATOR, unsigned long DENOMINATOR>
class bidirectional_spanning_tree :
  public ioa::automaton
{
private:

  void parent_effect(const size_t& j, size_t i) {
    parents.insert(std::make_pair(i,j));
    std::cout << "The parent of " << i << " is " << j << std::endl;
  }

  V_P_INPUT (bidirectional_spanning_tree, parent, size_t, size_t);

  std::auto_ptr<ioa::self_helper<bidirectional_spanning_tree> > self;
  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;

  std::vector<std::set<size_t> > nbrhd; //nbrhd = neighborhood.  Collection of neighboring automata
  std::map<size_t, size_t> parents;
  std::ofstream out;

public:
  bidirectional_spanning_tree () :
    self (new ioa::self_helper<bidirectional_spanning_tree> ()),
    nbrhd(N)
  {
    srand ((unsigned)time(0));
    size_t i0 = rand() % N;
    std::cout << "The root of the spanning tree is " << i0 << std::endl;

    assert(DENOMINATOR != 0);
    assert(NUMERATOR <= DENOMINATOR);
    double rho = double(NUMERATOR) / double(DENOMINATOR);
    for (size_t i = 0; i < N - 1; i++) {
      for(size_t j = i + 1; j < N; j++) {
        if(rho != 0.0 && drand48() < rho) {
          // Link between i and j.
          nbrhd[i].insert(j);
          nbrhd[j].insert(i);
        }
      }
    }

    for (size_t i = 0; i < N; ++i) {
      T_helpers.push_back (new ioa::automaton_helper<T> (this, ioa::make_generator<T> (i, i0, nbrhd[i])));
      make_bind_helper (this,
                         T_helpers[i],
                         &T::parent,
                         self.get(),
                         &bidirectional_spanning_tree::parent,
                         i);
    }

    //Generate random network structure for the automata and bind channel automata to them
    for (size_t i = 0; i < N - 1; i++) {
      for (size_t j = i + 1; j < N; j++) {

        if (nbrhd[i].count (j) != 0) {
          // Link between i and j.
          ioa::automaton_helper<channel_automaton<search_t> >* i_to_j_channel = new ioa::automaton_helper<channel_automaton<search_t> > (this, ioa::make_generator<channel_automaton<search_t> > ());
          ioa::automaton_helper<channel_automaton<search_t> >* j_to_i_channel = new ioa::automaton_helper<channel_automaton<search_t> > (this, ioa::make_generator<channel_automaton<search_t> > ());

          make_bind_helper (this,
                            T_helpers[i],
                           &T::send,
                           j,
                           i_to_j_channel,
                           &channel_automaton<search_t>::send);

          make_bind_helper (this,
                             T_helpers[j],
                             &T::send,
                             i,
                             j_to_i_channel,
                             &channel_automaton<search_t>::send);


          make_bind_helper (this,
                            i_to_j_channel,
                            &channel_automaton<search_t>::receive,
                            T_helpers[j],
                            &T::receive,
                            i);

          make_bind_helper (this,
                            j_to_i_channel,
                            &channel_automaton<search_t>::receive,
                            T_helpers[i],
                            &T::receive,
                            j);

        }
      }
    }
  }

  //output the file to be used in GraphViz
  ~bidirectional_spanning_tree () {
    out.open("attempt.dot");

    out << "digraph spanning_tree {" <<  std::endl << std::endl;
    out << "label=\"Spanning Tree\"" << std::endl;
    out << "overlap=scale" << std::endl; //this line for neato use only. if using dot, comment out!
    out << "style=filled; bgcolor=\"#f1f1f1\";" << std::endl << std::endl << std::endl;

    out << "node [color=Red, nodesep=10.0, fontname=Courier,]" << std::endl << std::endl;

    for (size_t i = 0; i < N - 1; i++) {
      for (size_t j = i + 1; j < N; j++) {
        if (nbrhd[i].count (j) != 0) {
          assert (nbrhd[j].count (i) != 0);
          if (parents.find (i) != parents.end () && parents[i] == j) {
            // This link is in the tree.
            out << "\"AST " << i << "\" -> " << "\"AST " << j << "\"" << std::endl;
          }
          else if (parents.find (j) != parents.end () && parents[j] == i) {
            // So is this one.
            out << "\"AST " << j << "\" -> " << "\"AST " << i << "\"" << std::endl;
          }
          else {
            // This link is not special.
            out << "\"AST " << i << "\" -> " << "\"AST " << j << "\" [arrowhead=\"none\", color=Blue, style=dashed]" << std::endl;
          }
        }
      }
    }

    out << std::endl << "}" << std::endl;

    out.close();
  }
};


#endif
