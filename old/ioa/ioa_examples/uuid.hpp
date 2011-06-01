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

  void clear () {
    uuid_clear (u);
  }

  bool is_null () const {
    return uuid_is_null (u);
  }

  uuid& operator= (const uuid& x) {
    if (this != &x) {
      uuid_copy (u, x.u);
    }
    return *this;
  }

  bool operator< (const uuid& x) const {
    return uuid_compare (u, x.u) < 0;
  }

  bool operator> (const uuid& x) const {
    return uuid_compare (u, x.u) > 0;
  }

  bool operator== (const uuid& x) const {
    return uuid_compare (u, x.u) == 0;
  }

  bool operator<= (const uuid& x) const {
    return uuid_compare (u, x.u) <= 0;
  }

  /*
    UUIDs are 128-bit identifiers (16 bytes).
    When you unparse them you get a string like
    FFD40331-98CD-41EA-AA27-8A66D33B6789.
    Thus, you need a 16 * 2 (hex characters) + 4 (hyphens) + 1 ('\0')
    = 37 byte buffer to hold the string.
  */
  static const size_t UUID_STRING_SIZE = 37;  

  void print_on (std::ostream& os) const {
    char s[UUID_STRING_SIZE];
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
