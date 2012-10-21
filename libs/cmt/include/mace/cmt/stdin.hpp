#ifndef _MACE_CMT_STDIN_HPP
#define _MACE_CMT_STDIN_HPP
#include <iostream>

namespace mace { namespace cmt {
  
  /**
   *  Provides fiber-friendly blockin on stdin.
   */
  std::istream& get_cin_stream();

} }
#endif // _MACE_CMT_STDIN_HPP
