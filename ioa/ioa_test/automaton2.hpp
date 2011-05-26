#ifndef __automaton2_hpp__
#define __automaton2_hpp__

#include "instance_generator.hpp"

class automaton2 :
  public ioa::automaton_interface
{
private:
  bool output_ () {
    return false;
  }

  void input_ () {
  }

public:
  typedef ioa::void_output_wrapper<automaton2, &automaton2::output_> output_t;
  output_t output;
  typedef ioa::void_input_wrapper<automaton2, &automaton2::input_> input_t;
  input_t input;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input2;

  automaton2 () :
    output (*this),
    input (*this),
    input2 (*this)
  { }

  void init () { }

  template <class I, class D>
  void instance_exists (const I* i,
			D&) {
  }

  template <class I, class D>
  void automaton_created (const ioa::automaton_handle<I>& handle,
			  D&) {
  }

  template <class D>
  void automaton_destroyed (D&) {
  }

  template <class D>
  void output_automaton_dne (D&) {
  }

  template <class D>
  void input_automaton_dne (D&) {
  }

  template <class D>
  void binding_exists (D&) {
  }

  template <class D>
  void input_action_unavailable (D&) {
  }

  template <class D>
  void output_action_unavailable (D&) {
  }

  template <class D>
  void bound (D&) {
  }

  template <class D>
  void unbound (D&) {
  }

  template <class D>
  void binding_dne (D&) {
  }

  template <class D>
  void target_automaton_dne (D&) {
  }

  template <class D>
  void destroyer_not_creator (D&) {
  }

};

struct automaton2_generator :
  public ioa::instance_generator<automaton2>
{

};

#endif
