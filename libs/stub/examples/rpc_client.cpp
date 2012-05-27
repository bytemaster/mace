#include "calculator.hpp"
#include "rpc.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

using namespace mace;

/**
 *  This is a simple RPC client that will connect to an
 *  RPC server that implements the Calculator interface.
 *
 *  In this case, the actual rpc interface is also wrapped in the
 *  command line interface for Calculator.  So the flow of control
 *  becomes:
 *
 *  User    -> CLI -> RPC Client -> RPC Server -> Implementation
 *  Display <- CLI <- RPC Client <- RPC Server <- 
 *
 *
 */
int main( int argc, char** argv ) {
    if( argc <= 2 ) {
        std::cerr << "Usage: rpc_client IP PORT\n";
        return -1;
    }
    using namespace boost;
    try {
        //! [Using RPC Client]
        stub::rpc_client<calculator> calc;
        calc.connect_to( argv[1], boost::lexical_cast<uint16_t>(argv[2]) );

        // or cast it to a generic stub::ptr<calculator>
        stub::ptr<calculator> generic_calc(calc);
        //! [Using RPC Client]
        
        boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
        double sum = 0;
        int i = 0;
        for( i = 0; i < 100000; ++i ) {
            sum += generic_calc->add(5,5);
        }
        boost::posix_time::ptime end = boost::posix_time::microsec_clock::universal_time();
        uint64_t us = (end-start).total_microseconds();
        std::cerr << i << " add(5) took  " << us << "us   " << double(i) / (us/1000000.0) << "invoke/sec\n";
    } catch ( const boost::exception& e ) {
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }
    return 0;
}


