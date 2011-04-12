#ifndef __automaton_test_hpp__
#define __automaton_test_hpp__

struct automaton {

  struct up_ut_output_action {
    bool state;

    up_ut_output_action () :
      state (false) { }

    bool operator() () {
      state = true;
      return true;
    }

  };
  up_ut_output_action up_ut_output;

  struct p_ut_output_action {
    bool state;
    int* last_parameter;

    p_ut_output_action () :
      state (false),
      last_parameter (0)
    { }

    typedef int parameter_type;

    bool operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
      return true;
    }

  };
  p_ut_output_action p_ut_output;

  struct up_t_output_action {
    bool state;

    up_t_output_action () :
      state (false)
    { }

    typedef int value_type;

    std::pair<bool, int> operator() () {
      state = true;
      return std::make_pair (true, 9845);
    }

  };
  up_t_output_action up_t_output;

  struct p_t_output_action {
    bool state;
    int* last_parameter;

    p_t_output_action () :
      state (false),
      last_parameter (0)
    { }

    typedef int value_type;
    typedef int parameter_type;

    std::pair<bool, int> operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
      return std::make_pair (true, 9845);
    }

  };
  p_t_output_action p_t_output;

  struct up_ut_input_action {
    bool state;

    up_ut_input_action () :
      state (false) { }

    void operator() () {
      state = true;
    }

  };
  up_ut_input_action up_ut_input;

  struct p_ut_input_action {
    bool state;
    int* last_parameter;

    p_ut_input_action () :
      state (false),
      last_parameter (0)
    { }

    typedef int parameter_type;

    void operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
    }

  };
  p_ut_input_action p_ut_input;

  struct up_t_input_action {
    int value;

    up_t_input_action () :
      value (0) { }

    typedef int value_type;

    void operator() (const int t) {
      value = t;
    }

  };
  up_t_input_action up_t_input;

  struct p_t_input_action {
    int value;
    int* last_parameter;

    p_t_input_action () :
      value (0),
      last_parameter (0)
    { }

    typedef int value_type;
    typedef int parameter_type;

    void operator() (const int t, int* parameter) {
      value = t;
      last_parameter = parameter;
    }

  };
  p_t_input_action p_t_input;

  // struct input_action {
  //   automaton& m_automaton;
  //   input_action(automaton& automaton)
  //     : m_automaton(automaton) { }
  //   void operator()(const int v) {
  //     m_automaton.value = v;
  //   }
  // };
  // input_action input;
  
  struct internal_action {
    bool state;
    internal_action()
      : state(false) { }
    void operator()() {
      state = true;
    }
  };
  internal_action internal;

};

struct null_scheduler {
  void set_current_automaton (ioa::automaton*) { }
};

#endif
