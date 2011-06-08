#ifndef __automaton2_hpp__
#define __automaton2_hpp__

#include <ioa/action_wrapper.hpp>

class automaton2 :
  public ioa::automaton_interface
{
private:
  float m_pole;
  int m_vole;

  UV_UP_INPUT (automaton2, uv_up_input) { }
  UV_P_INPUT (automaton2, uv_p_input, float, pole) { m_pole += pole; }
  V_UP_INPUT (automaton2, v_up_input, int, vole) { m_vole += vole; }
  V_P_INPUT (automaton2, v_p_input, int, vole, float, pole) { m_vole += vole; m_pole += pole; }

  UV_UP_OUTPUT (automaton2, uv_up_output) { return false; }
  UV_P_OUTPUT (automaton2, uv_p_output, float, pole) { m_pole += pole; return false; }
  V_UP_OUTPUT (automaton2, v_up_output, int) { return std::pair<bool, int> (); }
  V_P_OUTPUT (automaton2, v_p_output, int, float, pole) { m_pole += pole; return std::pair<bool, int> (); }

  DECLARE_UP_INTERNAL (automaton2, up_internal);

  P_INTERNAL (automaton2, p_internal, float, pole) { m_pole += pole; }
  
  UV_UP_INPUT (automaton2, uv_up_input2) { }
public:
  automaton2 () :
    ACTION (automaton2, uv_up_input),
    ACTION (automaton2, uv_p_input),
    ACTION (automaton2, v_up_input),
    ACTION (automaton2, v_p_input),
    ACTION (automaton2, uv_up_output),
    ACTION (automaton2, uv_p_output),
    ACTION (automaton2, v_up_output),
    ACTION (automaton2, v_p_output),
    ACTION (automaton2, p_internal),
    ACTION (automaton2, uv_up_input2)
  { }

};

DEFINE_UP_INTERNAL (automaton2, up_internal) { }

#endif
