#ifndef _MACE_STUB_CALCULATOR_HPP
#define _MACE_STUB_CALCULATOR_HPP
#include <mace/stub/vtable.hpp>
#include <string>

//! [Define interfaces]
struct service {
  std::string name()const;
  int         exit();
};
struct calculator : service {
  double add( double v, double v2 );
  double sub( double v, double v2 );           
};
//! [Define interfaces]

//! [Expose interfaces]
MACE_STUB( service, (name)(exit) )
MACE_STUB_DERIVED( calculator, (service), (add)(sub) )
//! [Expose interfaces]

#endif // _MACE_STUB_CALCULATOR_HPP
