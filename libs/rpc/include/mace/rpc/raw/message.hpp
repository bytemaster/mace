#ifndef _MACE_RPC_RAW_MESSAGE_HPP_
#define _MACE_RPC_RAW_MESSAGE_HPP_
#include <mace/rpc/varint.hpp>
#include <mace/rpc/message.hpp>
#include <mace/reflect/reflect.hpp>

namespace mace { namespace rpc { namespace raw {
  typedef std::vector<char>   datavec;
  struct message {
    message(){};
    message( mace::rpc::message&& m )
    :id( m.id ? *m.id : 0 ),method(std::move(m.meth)),error_code(m.err),data(std::move(m.data))
    {}

    operator mace::rpc::message() {
      return mace::rpc::message(
                    std::move(method), 
                    std::move(data),
                    id.value,
                    mace::rpc::message::error_type(error_code.value)
                    );
    }

    signed_int      id;
    std::string     method;
    signed_int      error_code;
    datavec         data;
  };

} } } 

MACE_REFLECT( mace::rpc::raw::message, (id)(method)(error_code)(data) )

#endif
