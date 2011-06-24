#ifndef __buffer_hpp__
#define __buffer_hpp__

#include <cstring>

namespace ioa {

  class buffer {
  private:
    unsigned char* m_data;
    size_t m_size;

  public:
    buffer () :
      m_data (0),
      m_size (0)
    { }

    buffer (const void* data,
	    const size_t size) :
      m_size (size)
    {
      m_data = new unsigned char[m_size];
      memcpy (m_data, data, m_size);
    }

    buffer (const buffer& other) :
      m_size (other.m_size)
    {
      m_data = new unsigned char[m_size];
      memcpy (m_data, other.m_data, m_size);
    }

    buffer& operator= (const buffer& other) {
      if (this != &other) {
	delete[] m_data;
	m_size = other.m_size;
	m_data = new unsigned char[m_size];
	memcpy (m_data, other.m_data, m_size);
      }
      return *this;
    }

    ~buffer () {
      delete[] m_data;
    }

    size_t size () const {
      return m_size;
    }
  
    const unsigned char* data () const {
      return m_data;
    }

    const char* c_str () const {
      return reinterpret_cast<char* const> (m_data);
    }

  };

}

#endif
