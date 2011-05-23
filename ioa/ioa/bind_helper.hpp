#ifndef __bind_helper_hpp__
#define __bind_helper_hpp__

namespace ioa {

  template <class T, class OH, class OM, class IH, class IM>
  class bind_helper
  {
  private:
    typedef typename OH::instance OI;
    typedef typename IH::instance II;

  public:
    bind_helper (const T* t,
		 OH& output_helper,
		 OM OI::*output_member_ptr,
		 IH& input_helper,
		 IM II::*input_member_ptr)
    { }

    void bind () {
    }

  };

}

#endif
