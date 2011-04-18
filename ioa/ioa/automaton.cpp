#include "automaton.hpp"

std::ostream& ioa::operator<<(std::ostream& output, const ioa::generic_automaton_handle& ai) {
  return (output << "generic_automaton_handle");
}
