#include "minunit.h"

#include <iostream>

extern const char* all_tests ();

int
main (
      int argc,
      char **argv
      )
{
  const char* result = all_tests();
  if (result != 0) {
    std::cout << result << std::endl;
  }

  return result != 0;
}
