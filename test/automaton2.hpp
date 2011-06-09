#ifndef __automaton2_hpp__
#define __automaton2_hpp__

#include <ioa/action_wrapper.hpp>

class automaton2 :
  public ioa::automaton_interface
{
private:
  float m_pole;
  int m_vole;

  DECLARE_UV_UP_INPUT (automaton2, uv_up_input);
  DECLARE_UV_P_INPUT (automaton2, uv_p_input, float, pole);
  DECLARE_V_UP_INPUT (automaton2, v_up_input, int, vole);
  DECLARE_V_P_INPUT (automaton2, v_p_input, int, vole, float, pole);

  DECLARE_UV_UP_OUTPUT (automaton2, uv_up_output);
  DECLARE_UV_P_OUTPUT (automaton2, uv_p_output, float, pole);
  DECLARE_V_UP_OUTPUT (automaton2, v_up_output, int);
  DECLARE_V_P_OUTPUT (automaton2, v_p_output, int, float, pole);

  DECLARE_UP_INTERNAL (automaton2, up_internal);
  DECLARE_P_INTERNAL (automaton2, p_internal, float, pole);
  
  DECLARE_UV_UP_INPUT (automaton2, uv_up_input2);

public:
  automaton2 () :
    ACTION (automaton2, uv_up_output),
    ACTION (automaton2, uv_p_output),
    ACTION (automaton2, v_up_output),
    ACTION (automaton2, v_p_output)
  { }

};

DEFINE_UV_UP_INPUT (automaton2, uv_up_input) { }
DEFINE_UV_P_INPUT (automaton2, uv_p_input, float, pole) { m_pole += pole; }
DEFINE_V_UP_INPUT (automaton2, v_up_input, int, vole) { m_vole += vole; }
DEFINE_V_P_INPUT (automaton2, v_p_input, int, vole, float, pole) { m_vole += vole; m_pole += pole; }

DEFINE_UV_UP_OUTPUT (automaton2, uv_up_output) { return false; }
DEFINE_UV_P_OUTPUT (automaton2, uv_p_output, float, pole) { m_pole += pole; return false; }
DEFINE_V_UP_OUTPUT (automaton2, v_up_output, int) { return std::pair<bool, int> (); }
DEFINE_V_P_OUTPUT (automaton2, v_p_output, int, float, pole) { m_pole += pole; return std::pair<bool, int> (); }


DEFINE_UP_INTERNAL (automaton2, up_internal) { }
DEFINE_P_INTERNAL (automaton2, p_internal, float, pole) { m_pole += pole; }

DEFINE_UV_UP_INPUT (automaton2, uv_up_input2) { }

#endif
