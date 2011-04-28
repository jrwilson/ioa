#ifndef __automaton_test_hpp__
#define __automaton_test_hpp__

#include <action.hpp>

struct automaton {

  struct up_ut_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::no_parameter
  {
    bool state;

    up_ut_input_action () :
      state (false) { }

    void operator() () {
      state = true;
    }

  };
  up_ut_input_action up_ut_input;

  struct p_ut_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::parameter<int>
  {
    bool state;
    int* last_parameter;

    p_ut_input_action () :
      state (false),
      last_parameter (0)
    { }

    void operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
    }

  };
  p_ut_input_action p_ut_input;

  struct up_t_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::no_parameter
  {
    int value;

    up_t_input_action () :
      value (0) { }

    void operator() (const int t) {
      value = t;
    }

  };
  up_t_input_action up_t_input;

  struct p_t_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::parameter<int>
  {
    int value;
    int* last_parameter;

    p_t_input_action () :
      value (0),
      last_parameter (0)
    { }

    void operator() (const int t, int* parameter) {
      value = t;
      last_parameter = parameter;
    }

  };
  p_t_input_action p_t_input;

  struct up_ut_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::no_parameter
  {
    bool state;

    up_ut_output_action () :
      state (false) { }

    bool operator() () {
      state = true;
      return true;
    }

  };
  up_ut_output_action up_ut_output;

  struct p_ut_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::parameter<int>
  {
    bool state;
    int* last_parameter;

    p_ut_output_action () :
      state (false),
      last_parameter (0)
    { }

    bool operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
      return true;
    }

  };
  p_ut_output_action p_ut_output;

  struct up_t_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::no_parameter
  {
    bool state;

    up_t_output_action () :
      state (false)
    { }

    std::pair<bool, int> operator() () {
      state = true;
      return std::make_pair (true, 9845);
    }

  };
  up_t_output_action up_t_output;

  struct p_t_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::parameter<int>
  {
    bool state;
    int* last_parameter;

    p_t_output_action () :
      state (false),
      last_parameter (0)
    { }

    std::pair<bool, int> operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
      return std::make_pair (true, 9845);
    }

  };
  p_t_output_action p_t_output;
  
  // struct internal_action {
  //   bool state;
  //   internal_action()
  //     : state(false) { }
  //   void operator()() {
  //     state = true;
  //   }
  // };
  // internal_action internal;

};

#endif
