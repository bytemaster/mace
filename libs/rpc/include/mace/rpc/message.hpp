#ifndef _MACE_RPC_MESSAGE_HPP_
#define _MACE_RPC_MESSAGE_HPP_
#include <vector>
#include <boost/optional.hpp>
#include <utility>

namespace mace { namespace rpc {

  typedef std::vector<char> datavec;
  /**
   *  Generic message used to pass parameters across the network.
   */
  struct message {
    enum error_type {
      none              = 0,
      unknown_method    = 1,
      invalid_response  = 2,
      broken_promise    = 3,
      exception_thrown  = 4
    };

    boost::optional<int32_t>  id;         ///< Used to track response
    error_type                err;        ///< 0 for no error
    std::string               meth;       ///< NULL for response
    datavec                   data;       ///< parameters,result, or error message

    inline message( message&& m )
    :id(m.id),
     err(m.err),
     meth(std::move(m.meth)),
     data( std::move(m.data) ){}

    inline message( std::string&& m, datavec&& param, int rid, error_type e = none )
    :id(rid),err(e),meth(std::move(m)),data(std::move(param)) {}

    inline message( datavec&& param, int rid )
    :id(rid),err(none),data(std::move(param)) {}

    inline message( datavec&& result, int rid, error_type e )
    :id(rid),err(e),data(std::move(result)) {}

    inline message( std::string&& m, datavec&& param )
    :err(none), meth(std::move(m)),data(std::move(param)) {}
             
    inline message():err(none){};

    private:
      // we should not be copying these should we?
      message& operator = ( const message& m );
      // we should not be copying these should we?
      message( const message& ); 
  };


} }

#include <mace/reflect/reflect.hpp>
MACE_REFLECT_ENUM( mace::rpc::message::error_type,
    (mace::rpc::message::none)
    (mace::rpc::message::unknown_method)
    (mace::rpc::message::invalid_response)
    (mace::rpc::message::broken_promise)
    (mace::rpc::message::exception_thrown) 
)

#endif // _MACE_RPC_MESSAGE_HPP_
