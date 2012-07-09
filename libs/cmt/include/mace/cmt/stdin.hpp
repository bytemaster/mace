#ifndef _MACE_CMT_STDIN_HPP
#define _MACE_CMT_STDIN_HPP
#include <iostream>

namespace mace { namespace cmt {
  
  /**
   *  Provides fiber-friendly blockin on stdin.
   */
  extern std::istream& cin;

} }
#endif // _MACE_CMT_STDIN_HPP
