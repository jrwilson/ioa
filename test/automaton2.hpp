#ifndef __automaton2_hpp__
#define __automaton2_hpp__

#include <ioa/automaton_interface.hpp>
#include <ioa/action_wrapper.hpp>

class automaton2 :
  public ioa::automaton_interface
{
private:
  int m_pole;
  int m_vole;

private:
  void uv_up_input_action () { }
  void uv_p_input_action (int pole) { m_pole += pole; }
  void v_up_input_action (int const & vole) { m_vole += vole; }
  void v_p_input_action (int const & vole, int pole) { m_vole += vole; m_pole += pole; }

  bool uv_up_output_precondition () const { return true; }
  void uv_up_output_action () { }
  bool uv_p_output_precondition (int pole) const { return true; }
  void uv_p_output_action (int pole) { m_pole += pole; }
  bool v_up_output_precondition () const { return true; }
  int v_up_output_action () { return 0; }
  bool v_p_output_precondition (int pole) const { return true; }
  int v_p_output_action (int pole) { m_pole += pole; return 0; }

  bool up_internal_precondition () const { return true; }
  void up_internal_action () { }
  bool p_internal_precondition (int pole) const { return true; }
  void p_internal_action (int pole) { m_pole += pole; }

  void uv_up_input2_action () { }

public:
  UV_UP_INPUT (automaton2, uv_up_input);
  UV_P_INPUT (automaton2, uv_p_input, int);
  V_UP_INPUT (automaton2, v_up_input, int);
  V_P_INPUT (automaton2, v_p_input, int, int);

  UV_UP_OUTPUT (automaton2, uv_up_output);
  UV_P_OUTPUT (automaton2, uv_p_output, int);
  V_UP_OUTPUT (automaton2, v_up_output, int);
  V_P_OUTPUT (automaton2, v_p_output, int, int);

  UP_INTERNAL (automaton2, up_internal);
  P_INTERNAL (automaton2, p_internal, int);
  
  UV_UP_INPUT (automaton2, uv_up_input2);

};

#endif
