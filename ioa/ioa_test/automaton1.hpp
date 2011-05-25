#ifndef __automaton1_hpp__
#define __automaton1_hpp__

#include <action.hpp>
#include <binding_handle.hpp>
#include "instance_generator.hpp"

template <class T>
struct bindable
{
  bool bound_;
  T* bound_parameter;
  bool unbound_;
  T* unbound_parameter;
  
  void bound () {
    bound_ = true;
  }
  
  void unbound () {
    unbound_ = true;
  }

  void bound (T* param) {
    bound_ = true;
    bound_parameter = param;
  }
  
  void unbound (T* param) {
    unbound_ = true;
    unbound_parameter = param;
  }
  
  bindable () :
    bound_ (false),
    unbound_ (false)
  { }
    
};

struct automaton1 :
  public ioa::automaton_interface
{
  struct up_uv_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::no_parameter,
    public bindable<int>
  {
    bool state;

    up_uv_input_action () :
      state (false)
    { }

    void operator() () {
      state = true;
    }
  };
  up_uv_input_action up_uv_input;

  struct p_uv_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::parameter<int>,
    public bindable<int>
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

  };
  p_uv_input_action p_uv_input;

  struct up_v_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::no_parameter,
    public bindable<int>
  {
    int value;

    up_v_input_action () :
      value (0) { }

    void operator() (const int t) {
      value = t;
    }
  };
  up_v_input_action up_v_input;

  struct p_v_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::parameter<int>,
    public bindable<int>
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
  };
  p_v_input_action p_v_input;

  struct up_uv_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::no_parameter,
    public bindable<int>
  {
    bool state;

    up_uv_output_action () :
      state (false) { }

    bool operator() () {
      state = true;
      return true;
    }
  };
  up_uv_output_action up_uv_output;

  struct p_uv_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::parameter<int>,
    public bindable<int>
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
  };
  p_uv_output_action p_uv_output;

  struct up_v_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::no_parameter,
    public bindable<int>
  {
    bool state;

    up_v_output_action () :
      state (false)
    { }

    std::pair<bool, int> operator() () {
      state = true;
      return std::make_pair (true, 9845);
    }
  };
  up_v_output_action up_v_output;

  struct p_v_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::parameter<int>,
    public bindable<int>
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

  bool inited;
  const automaton1* existing_instance;
  ioa::automaton_handle<automaton1> created_handle;
  bool m_automaton_destroyed;
  bool parameter_existed;
  ioa::parameter_handle<int> declared_handle;
  bool m_parameter_rescinded;
  bool m_output_automaton_dne;
  bool m_input_automaton_dne;
  bool m_output_parameter_dne;
  bool m_input_parameter_dne;
  bool m_binding_exists;
  bool m_input_action_unavailable;
  bool m_output_action_unavailable;
  bool m_bid;
  bool m_unbound;
  bool m_binding_dne;
  bool m_parameter_dne;
  bool m_target_automaton_dne;
  bool m_destroyer_not_creator;

  automaton1 () :
    inited (false),
    existing_instance (0),
    m_automaton_destroyed (false),
    parameter_existed (false),
    m_parameter_rescinded (false),
    m_output_automaton_dne (false),
    m_input_automaton_dne (false),
    m_output_parameter_dne (false),
    m_input_parameter_dne (false),
    m_binding_exists (false),
    m_input_action_unavailable (false),
    m_output_action_unavailable (false),
    m_bid (-1),
    m_unbound (false),
    m_binding_dne (false),
    m_parameter_dne (false),
    m_target_automaton_dne (false),
    m_destroyer_not_creator (false)
  { }

  void init () {
    inited = true;
  }

  template <class D>
  void instance_exists (const automaton1* i,
			D&) {
    existing_instance = i;
  }

  template <class D>
  void automaton_created (const ioa::automaton_handle<automaton1>& handle,
			  D&) {
    created_handle = handle;
  }

  template <class D>
  void automaton_destroyed (D&) {
    m_automaton_destroyed = true;
  }

  template <class D>
  void parameter_exists (D&) {
    parameter_existed = true;
  }

  template <class D>
  void parameter_declared (const ioa::parameter_handle<int>& handle,
			   D&) {
    declared_handle = handle;
  }

  template <class D>
  void parameter_rescinded (D&) {
    m_parameter_rescinded = true;
  }

  template <class D>
  void output_automaton_dne (D&) {
    m_output_automaton_dne = true;
  }

  template <class D>
  void input_automaton_dne (D&) {
    m_input_automaton_dne = true;
  }

  template <class D>
  void output_parameter_dne (D&) {
    m_output_parameter_dne = true;
  }

  template <class D>
  void input_parameter_dne (D&) {
    m_input_parameter_dne = true;
  }

  template <class D>
  void binding_exists (D&) {
    m_binding_exists = true;
  }

  template <class D>
  void input_action_unavailable (D&) {
    m_input_action_unavailable = true;
  }

  template <class D>
  void output_action_unavailable (D&) {
    m_output_action_unavailable = true;
  }

  template <class D>
  void bound (const ioa::bid_t bid,
	      D&) {
    m_bid = bid;
  }

  template <class D>
  void unbound (D&) {
    m_unbound = true;
  }

  template <class D>
  void binding_dne (D&) {
    m_binding_dne = true;
  }

  template <class D>
  void parameter_dne (D&) {
    m_parameter_dne = true;
  }

  template <class D>
  void target_automaton_dne (D&) {
    m_target_automaton_dne = true;
  }

  template <class D>
  void destroyer_not_creator (D&) {
    m_destroyer_not_creator = true;
  }

};

struct automaton1_generator :
  public ioa::instance_generator <automaton1>
{

};

#endif
