#ifndef MY_LIBRARY_H
#define MY_LIBRARY_H

#include <iostream>

class Number {
public:
  Number(int value);

  bool operator==(const Number& other) const;

  Number add(const Number& other) const;
  Number subtract(const Number& other) const;

  int getValue() const;

private:
  int value;
};

std::ostream& operator<<(std::ostream& output, const Number&n);

#endif
