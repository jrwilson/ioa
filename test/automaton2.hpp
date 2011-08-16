#ifndef __automaton2_hpp__
#define __automaton2_hpp__

#include <ioa/automaton.hpp>
#include <ioa/action_wrapper.hpp>

class automaton2 :
  public ioa::automaton
{
private:
  int m_pole;
  int m_vole;

private:
  void uv_up_input_effect () { }
  void uv_up_input_schedule () const { }
  void uv_p_input_effect (int pole) { m_pole += pole; }
  void uv_p_input_schedule (int) const { }
  void uv_ap_input_effect (ioa::aid_t aid) { }
  void uv_ap_input_schedule (ioa::aid_t) const { }
  void v_up_input_effect (int const & vole) { m_vole += vole; }
  void v_up_input_schedule () const { }
  void v_p_input_effect (int const & vole, int pole) { m_vole += vole; m_pole += pole; }
  void v_p_input_schedule (int) const { }
  void v_ap_input_effect (int const & vole, ioa::aid_t aid) { m_vole += vole; }
  void v_ap_input_schedule (ioa::aid_t) const { }

  bool uv_up_output_precondition () const { return true; }
  void uv_up_output_effect () { }
  void uv_up_output_schedule () const { }
  bool uv_p_output_precondition (int pole) const { return true; }
  void uv_p_output_effect (int pole) { m_pole += pole; }
  void uv_p_output_schedule (int) const { }
  bool uv_ap_output_precondition (ioa::aid_t aid) const { return true; }
  void uv_ap_output_effect (ioa::aid_t aid) { }
  void uv_ap_output_schedule (ioa::aid_t) const { }
  bool v_up_output_precondition () const { return true; }
  int v_up_output_effect () { return 0; }
  void v_up_output_schedule () const { }
  bool v_p_output_precondition (int pole) const { return true; }
  int v_p_output_effect (int pole) { m_pole += pole; return 0; }
  void v_p_output_schedule (int) const { }
  bool v_ap_output_precondition (ioa::aid_t aid) const { return true; }
  int v_ap_output_effect (ioa::aid_t aid) { return 0; }
  void v_ap_output_schedule (ioa::aid_t) const { }

  bool up_internal_precondition () const { return true; }
  void up_internal_effect () { }
  void up_internal_schedule () const { }
  bool p_internal_precondition (int pole) const { return true; }
  void p_internal_effect (int pole) { m_pole += pole; }
  void p_internal_schedule (int) const { }

  void uv_up_input2_effect () { }
  void uv_up_input2_schedule () const { }

public:
  UV_UP_INPUT (automaton2, uv_up_input);
  UV_P_INPUT (automaton2, uv_p_input, int);
  UV_AP_INPUT (automaton2, uv_ap_input);
  V_UP_INPUT (automaton2, v_up_input, int);
  V_P_INPUT (automaton2, v_p_input, int, int);
  V_AP_INPUT (automaton2, v_ap_input, int);

  UV_UP_OUTPUT (automaton2, uv_up_output);
  UV_P_OUTPUT (automaton2, uv_p_output, int);
  UV_AP_OUTPUT (automaton2, uv_ap_output);
  V_UP_OUTPUT (automaton2, v_up_output, int);
  V_P_OUTPUT (automaton2, v_p_output, int, int);
  V_AP_OUTPUT (automaton2, v_ap_output, int);

  UP_INTERNAL (automaton2, up_internal);
  P_INTERNAL (automaton2, p_internal, int);
  
  UV_UP_INPUT (automaton2, uv_up_input2);

};

#endif
