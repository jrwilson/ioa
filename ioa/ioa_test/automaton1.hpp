#ifndef __automaton1_hpp__
#define __automaton1_hpp__

#include <action.hpp>

struct automaton1 :
  public ioa::automaton_interface
{

  void init () { }
  void instance_exists (const void*) { }
  void automaton_created (const ioa::generic_automaton_handle&) { }
  void parameter_exists (void*) { }
  void parameter_declared (const ioa::generic_parameter_handle&) { }
  void bind_output_automaton_dne () { }
  void bind_input_automaton_dne () { }
  void bind_output_parameter_dne () { }
  void bind_input_parameter_dne () { }
  void binding_exists () { }
  void input_action_unavailable () { }
  void output_action_unavailable () { }
  void bound () { }
  void unbind_output_automaton_dne () { }
  void unbind_input_automaton_dne () { }
  void unbind_output_parameter_dne () { }
  void unbind_input_parameter_dne () { }
  void binding_dne () { }
  void unbound () { }
  void parameter_dne (const ioa::generic_parameter_handle&) { }
  void parameter_rescinded (void*) { }
  void target_automaton_dne (const ioa::generic_automaton_handle&) { }
  void destroyer_not_creator (const ioa::generic_automaton_handle&) { }
  void automaton_destroyed (const ioa::generic_automaton_handle&) { }

  struct up_uv_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::no_parameter
  {
    bool state;

    up_uv_input_action () :
      state (false) { }

    void operator() () {
      state = true;
    }

    void bound () { }
    void unbound () { }
  };
  up_uv_input_action up_uv_input;

  struct p_uv_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::parameter<int>
  {
    bool state;
    int* last_parameter;

    p_uv_input_action () :
      state (false),
      last_parameter (0)
    { }

    void operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
    }

    void bound (int*) { }
    void unbound (int*) { }
  };
  p_uv_input_action p_uv_input;

  struct up_v_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::no_parameter
  {
    int value;

    up_v_input_action () :
      value (0) { }

    void operator() (const int t) {
      value = t;
    }

    void bound () { }
    void unbound () { }
  };
  up_v_input_action up_v_input;

  struct p_v_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::parameter<int>
  {
    int value;
    int* last_parameter;

    p_v_input_action () :
      value (0),
      last_parameter (0)
    { }

    void operator() (const int t, int* parameter) {
      value = t;
      last_parameter = parameter;
    }

    void bound (int*) { }
    void unbound (int*) { }
  };
  p_v_input_action p_v_input;

  struct up_uv_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::no_parameter
  {
    bool state;

    up_uv_output_action () :
      state (false) { }

    bool operator() () {
      state = true;
      return true;
    }

    void bound () { }
    void unbound () { }
  };
  up_uv_output_action up_uv_output;

  struct p_uv_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::parameter<int>
  {
    bool state;
    int* last_parameter;

    p_uv_output_action () :
      state (false),
      last_parameter (0)
    { }

    bool operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
      return true;
    }

    void bound (int*) { }
    void unbound (int*) { }
  };
  p_uv_output_action p_uv_output;

  struct up_v_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::no_parameter
  {
    bool state;

    up_v_output_action () :
      state (false)
    { }

    std::pair<bool, int> operator() () {
      state = true;
      return std::make_pair (true, 9845);
    }

    void bound () { }
    void unbound () { }
  };
  up_v_output_action up_v_output;

  struct p_v_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::parameter<int>
  {
    bool state;
    int* last_parameter;

    p_v_output_action () :
      state (false),
      last_parameter (0)
    { }

    std::pair<bool, int> operator() (int* parameter) {
      state = true;
      last_parameter = parameter;
      return std::make_pair (true, 9845);
    }

    void bound (int*) { }
    void unbound (int*) { }
  };
  p_v_output_action p_v_output;
  
  struct up_internal_action :
    public ioa::internal,
    public ioa::no_parameter
  {
    bool state;
    up_internal_action()
      : state(false) { }
    void operator()() {
      state = true;
    }
  };
  up_internal_action up_internal;

  struct p_internal_action :
    public ioa::internal,
    public ioa::parameter<int>
  {
    bool state;
    int* last_parameter;

    p_internal_action()
      : state(false) { }
    void operator()(int* parameter) {
      state = true;
      last_parameter = parameter;
    }
  };
  p_internal_action p_internal;

  struct uv_event_action :
    public ioa::event,
    public ioa::no_value
  {
    bool state;

    uv_event_action () :
      state (false) { }

    void operator () () {
      state = true;
    }
  };
  uv_event_action uv_event;

  struct v_event_action :
    public ioa::event,
    public ioa::value<int>
  {
    bool state;
    int last_value;

    v_event_action () :
      state (false) { }

    void operator () (const int value) {
      state = true;
      last_value = value;
    }
  };
  v_event_action v_event;
};

#endif
