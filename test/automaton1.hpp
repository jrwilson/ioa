#ifndef __automaton1_hpp__
#define __automaton1_hpp__

#include <ioa/action.hpp>
#include <ioa/automaton_interface.hpp>

template <class T>
struct bindable
{
  bool bound_;
  T bound_parameter;
  bool unbound_;
  T unbound_parameter;
  
  void bound () {
    bound_ = true;
  }
  
  void unbound () {
    unbound_ = true;
  }

  void bound (T param) {
    bound_ = true;
    bound_parameter = param;
  }
  
  void unbound (T param) {
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

    void operator() (automaton1&) {
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
    int last_parameter;

    p_uv_input_action () :
      state (false),
      last_parameter (0)
    { }

    void operator() (automaton1&, int parameter) {
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

    void operator() (automaton1&, const int t) {
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
    int last_parameter;

    p_v_input_action () :
      value (0),
      last_parameter (0)
    { }

    void operator() (automaton1&, const int t, int parameter) {
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

    bool precondition (automaton1&) const {
      return true;
    }

    void operator() (automaton1&) {
      state = true;
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
    int last_parameter;

    p_uv_output_action () :
      state (false),
      last_parameter (0)
    { }

    bool precondition (automaton1&, int parameter) {
      return true;
    }

    void operator() (automaton1&, int parameter) {
      state = true;
      last_parameter = parameter;
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

    bool precondition (automaton1&) const {
      return true;
    }

    int operator() (automaton1&) {
      state = true;
      return 9845;
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
    int last_parameter;

    p_v_output_action () :
      state (false),
      last_parameter (0)
    { }

    bool precondition (automaton1&, int parameter) {
      return true;
    }

    int operator() (automaton1&, int parameter) {
      state = true;
      last_parameter = parameter;
      return 9845;
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

    bool precondition (automaton1&) const {
      return true;
    }

    void operator() (automaton1&) {
      state = true;
    }
  };
  up_internal_action up_internal;

  struct p_internal_action :
    public ioa::internal,
    public ioa::parameter<int>
  {
    bool state;
    int last_parameter;

    p_internal_action()
      : state(false) { }

    bool precondition (automaton1&, int parameter) const {
      return true;
    }

    void operator() (automaton1&, int parameter) {
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
    uv_event_action()
      : state(false) { }
    void operator() (automaton1&) {
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
    v_event_action() :
      state (false),
      last_value (0)
    { }
    void operator() (automaton1&, int value) {
      state = true;
      last_value = value;
    }
  };
  v_event_action v_event;

};

#endif
