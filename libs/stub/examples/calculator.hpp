#ifndef _MACE_STUB_CALCULATOR_HPP
#define _MACE_STUB_CALCULATOR_HPP
#include <mace/stub/vtable.hpp>
#include <string>

struct Service {
    std::string name()const;
    int         exit();
};
struct Calculator : Service {
    double add( double v );           
    double add2( double v, double v2 );
    double sub( double v );           
    double mult( double v );           
    double div( double v );           
    double result()const;
};

MACE_STUB( Service, (name)(exit) )
MACE_STUB_DERIVED( Calculator, (Service), (add)(add2)(sub)(mult)(div)(result) )

#endif // _MACE_STUB_CALCULATOR_HPP
