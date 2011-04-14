#include "action.hpp"

std::ostream& ioa::operator<<(std::ostream& output, const ioa::action_interface& ai) {
  output << "action";
  return output;
}
