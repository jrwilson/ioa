#include "Number.hpp"

Number::Number(int value)
  : value (value) { }

bool
Number::operator==(const Number& other) const
{
  return value == other.value;
}

Number
Number::add(const Number& other) const
{
  return Number (value + other.value);
}

Number
Number::subtract(const Number& other) const
{
  return Number (value - other.value);
}

int
Number::getValue() const
{
  return value;
}

std::ostream&
operator<<(std::ostream& output, const Number&n)
{
  output << n.getValue();
  return output;
}
