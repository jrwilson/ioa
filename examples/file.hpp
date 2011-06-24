#ifndef __file_hpp__
#define __file_hpp__

#include "mftp.hpp"

#include <vector>

namespace mftp {
  
  class file {
  private:
    mfileid m_mfileid;
    uint8_t* m_data;
    std::vector<bool> m_have;
    uint32_t m_have_count;
    uint32_t m_start_idx;

  public:
    file (const char*,
	  const uint32_t);
    file (const fileid& f);
    file (const file& other);
    ~file ();
    const mfileid& get_mfileid () const;
    unsigned char* get_data_ptr ();
    bool complete () const;
    bool empty () const;
    bool have (const uint32_t offset) const;
    void write_chunk (const uint32_t offset,
		      const uint8_t* data);
    std::pair<uint32_t, uint32_t> get_next_range ();
    uint32_t get_random_index () const;
  };
}

#endif
