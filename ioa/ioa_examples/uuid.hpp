#ifndef __uuid_hpp__
#define __uuid_hpp__

#include <uuid/uuid.h>
#include <ostream>

struct uuid {
  uuid_t u;

  uuid () {
    uuid_clear (u);
  }

  uuid (bool t) {
    if (t) {
      uuid_generate (u);
    }
    else {
      uuid_clear (u);
    }
  }

  uuid (const uuid& x) {
    uuid_copy (u, x.u);
  }

  uuid& operator= (const uuid& x) {
    if (this != &x) {
      uuid_copy (u, x.u);
    }
    return *this;
  }

  bool operator> (const uuid& x) const {
    return uuid_compare (u, x.u) > 0;
  }

  bool operator== (const uuid& x) const {
    return uuid_compare (u, x.u) == 0;
  }

  void print_on (std::ostream& os) const {
    char s[37];
    uuid_unparse (u, s);
    os << s;
  }
};

std::ostream& operator<< (std::ostream& strm,
			  const uuid& u) {
  u.print_on (strm);
  return strm;
}

#endif
