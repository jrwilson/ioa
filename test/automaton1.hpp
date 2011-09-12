#ifndef __automaton1_hpp__
#define __automaton1_hpp__

#include <ioa/action.hpp>
#include <ioa/automaton_base.hpp>

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
  public ioa::automaton_base

{
  struct uv_up_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::no_parameter,
    public bindable<int>
  {
    bool state;

    uv_up_input_action () :
      state (false)
    { }

    void effect (automaton1&) {
      state = true;
    }

    void schedule (const automaton1&) const { }
  };
  uv_up_input_action uv_up_input;

  struct uv_p_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::parameter<int>,
    public bindable<int>
  {
    bool state;
    int last_parameter;

    uv_p_input_action () :
      state (false),
      last_parameter (0)
    { }

    void effect (automaton1&, int parameter) {
      state = true;
      last_parameter = parameter;
    }

    void schedule (const automaton1&, int parameter) const { }
  };
  uv_p_input_action uv_p_input;

  struct uv_ap_input_action :
    public ioa::input,
    public ioa::no_value,
    public ioa::auto_parameter,
    public bindable<ioa::aid_t>
  {
    bool state;
    ioa::aid_t last_parameter;

    uv_ap_input_action () :
      state (false),
      last_parameter (0)
    { }

    void effect (automaton1&, ioa::aid_t parameter) {
      state = true;
      last_parameter = parameter;
    }

    void schedule (const automaton1&, ioa::aid_t parameter) const { }
  };
  uv_ap_input_action uv_ap_input;

  struct v_up_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::no_parameter,
    public bindable<int>
  {
    int value;

    v_up_input_action () :
      value (0) { }

    void effect (automaton1&, const int t) {
      value = t;
    }

    void schedule (const automaton1&) const { }
  };
  v_up_input_action v_up_input;

  struct v_p_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::parameter<int>,
    public bindable<int>
  {
    int value;
    int last_parameter;

    v_p_input_action () :
      value (0),
      last_parameter (0)
    { }

    void effect (automaton1&, const int t, int parameter) {
      value = t;
      last_parameter = parameter;
    }

    void schedule (const automaton1&, int parameter) const { }
  };
  v_p_input_action v_p_input;

  struct v_ap_input_action :
    public ioa::input,
    public ioa::value<int>,
    public ioa::auto_parameter,
    public bindable<ioa::aid_t>
  {
    int value;
    ioa::aid_t last_parameter;

    v_ap_input_action () :
      value (0),
      last_parameter (0)
    { }

    void effect (automaton1&, const int t, ioa::aid_t parameter) {
      value = t;
      last_parameter = parameter;
    }

    void schedule (const automaton1&, ioa::aid_t parameter) const { }
  };
  v_ap_input_action v_ap_input;

  struct uv_up_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::no_parameter,
    public bindable<int>
  {
    bool state;

    uv_up_output_action () :
      state (false) { }

    bool precondition (const automaton1&) const {
      return true;
    }

    void effect (automaton1&) {
      state = true;
    }

    void schedule (const automaton1&) const { }
  };
  uv_up_output_action uv_up_output;

  struct uv_p_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::parameter<int>,
    public bindable<int>
  {
    bool state;
    int last_parameter;

    uv_p_output_action () :
      state (false),
      last_parameter (0)
    { }

    bool precondition (const automaton1&, int parameter) {
      return true;
    }

    void effect (automaton1&, int parameter) {
      state = true;
      last_parameter = parameter;
    }

    void schedule (const automaton1&, int parameter) const { }
  };
  uv_p_output_action uv_p_output;

  struct uv_ap_output_action :
    public ioa::output,
    public ioa::no_value,
    public ioa::auto_parameter,
    public bindable<ioa::aid_t>
  {
    bool state;
    ioa::aid_t last_parameter;

    uv_ap_output_action () :
      state (false),
      last_parameter (0)
    { }

    bool precondition (const automaton1&, ioa::aid_t parameter) {
      return true;
    }

    void effect (automaton1&, ioa::aid_t parameter) {
      state = true;
      last_parameter = parameter;
    }

    void schedule (const automaton1&, ioa::aid_t parameter) const { }
  };
  uv_ap_output_action uv_ap_output;

  struct v_up_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::no_parameter,
    public bindable<int>
  {
    bool state;

    v_up_output_action () :
      state (false)
    { }

    bool precondition (const automaton1&) const {
      return true;
    }

    int effect (automaton1&) {
      state = true;
      return 9845;
    }

    void schedule (const automaton1&) const { }
  };
  v_up_output_action v_up_output;

  struct v_p_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::parameter<int>,
    public bindable<int>
  {
    bool state;
    int last_parameter;

    v_p_output_action () :
      state (false),
      last_parameter (0)
    { }

    bool precondition (const automaton1&, int parameter) {
      return true;
    }

    int effect (automaton1&, int parameter) {
      state = true;
      last_parameter = parameter;
      return 9845;
    }

    void schedule (const automaton1&, int parameter) const { }
  };
  v_p_output_action v_p_output;

  struct v_ap_output_action :
    public ioa::output,
    public ioa::value<int>,
    public ioa::auto_parameter,
    public bindable<ioa::aid_t>
  {
    bool state;
    ioa::aid_t last_parameter;

    v_ap_output_action () :
      state (false),
      last_parameter (0)
    { }

    bool precondition (const automaton1&, ioa::aid_t parameter) {
      return true;
    }

    int effect (automaton1&, ioa::aid_t parameter) {
      state = true;
      last_parameter = parameter;
      return 9845;
    }

    void schedule (const automaton1&, ioa::aid_t parameter) const { }
  };
  v_ap_output_action v_ap_output;
  
  struct up_internal_action :
    public ioa::internal,
    public ioa::no_parameter
  {
    bool state;
    up_internal_action()
      : state(false) { }

    bool precondition (const automaton1&) const {
      return true;
    }

    void effect (automaton1&) {
      state = true;
    }

    void schedule (const automaton1&) const { }
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

    bool precondition (const automaton1&, int parameter) const {
      return true;
    }

    void effect (automaton1&, int parameter) {
      state = true;
      last_parameter = parameter;
    }

    void schedule (const automaton1&, int parameter) const { }
  };
  p_internal_action p_internal;

};

#endif
