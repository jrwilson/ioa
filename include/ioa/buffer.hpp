#ifndef __buffer_hpp__
#define __buffer_hpp__

#include <cstring>

namespace ioa {
  
  class buffer_interface
  {
  public:
    virtual ~buffer_interface () { }
    virtual const void* data () const = 0;
    virtual size_t size () const = 0;
  };

  class buffer :
    public buffer_interface
  {
  private:
    unsigned char* m_data;
    size_t m_size;
    size_t m_capacity;

  public:
    buffer () :
      m_data (0),
      m_size (0),
      m_capacity (0)
    { }

    buffer (const size_t size) :
      m_size (size),
      m_capacity (size)
    {
      m_data = new unsigned char[m_capacity];
    }

    buffer (const void* data,
	    const size_t size) :
      m_size (size),
      m_capacity (size)
    {
      m_data = new unsigned char[m_capacity];
      memcpy (m_data, data, m_size);
    }

    buffer (const buffer& other) :
      m_size (other.m_size),
      m_capacity (other.m_size)
    {
      m_data = new unsigned char[m_capacity];
      memcpy (m_data, other.m_data, m_size);
    }

    buffer& operator= (const buffer& other) {
      if (this != &other) {
	if (m_capacity < other.m_size) {
	  delete[] m_data;
	  m_capacity = other.m_size;
	  m_data = new unsigned char[m_capacity];
	}
	m_size = other.m_size;
	memcpy (m_data, other.m_data, m_size);
      }
      return *this;
    }

    ~buffer () {
      delete[] m_data;
    }

    void resize (const size_t new_size) {
      if (new_size > m_capacity) {
	m_capacity = new_size;
	unsigned char* ptr = new unsigned char[m_capacity];
	memcpy (ptr, m_data, m_size);
	delete[] m_data;
	m_data = ptr;
      }
      m_size = new_size;
    }

    size_t size () const {
      return m_size;
    }
  
    void* data () {
      return m_data;
    }

    const void* data () const {
      return m_data;
    }
  };

}

#endif
