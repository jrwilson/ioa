#ifndef __automaton_test_hpp__
#define __automaton_test_hpp__

struct automaton {
  int value;

  struct output_action {
    bool state;
    output_action()
      : state(false) { }
    int operator()() {
      state = true;
      return 9845;
    }
  };
  output_action output;

  struct input_action {
    automaton& m_automaton;
    input_action(automaton& automaton)
      : m_automaton(automaton) { }
    void operator()(const int v) {
      m_automaton.value = v;
    }
  };
  input_action input;
  
  struct internal_action {
    bool state;
    internal_action()
      : state(false) { }
    void operator()() {
      state = true;
    }
  };
  internal_action internal;

  automaton()
    : value(0),
      input(*this) { }
};

#endif
