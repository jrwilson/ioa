#include "action.hpp"

std::ostream& ioa::operator<<(std::ostream& output, const ioa::action_interface& ai) {
  output << "instance=" << ai.get_automaton ()->get_instance () << " action=" << ai.get_member_ptr () << " ";
  ai.print_on (output);
  return output;
}
