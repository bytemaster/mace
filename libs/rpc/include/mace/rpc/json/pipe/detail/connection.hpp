#ifndef _MACE_RPC_JSON_PIPE_DETAIL_CONNECTION_HPP
#define _MACE_RPC_JSON_PIPE_DETAIL_CONNECTION_HPP
#include <mace/rpc/pipe/detail/connection.hpp>
#include <mace/rpc/raw/message.hpp>
#include <mace/rpc/raw/raw_io.hpp>
#include <mace/cmt/mutex.hpp>

namespace mace { namespace rpc { namespace json { namespace pipe {  namespace detail {

  class connection : public mace::rpc::pipe::detail::connection  {
    public:
      connection( mace::rpc::connection_base& cb, std::istream&, std::ostream& );
      ~connection();

      virtual void         send_message( rpc::message&& m );
      virtual rpc::message read_message();

      mace::cmt::mutex wmutex;
  };

} } } } } // mace::rpc::raw::pipe::deatil

#endif // _MACE_RPC_RAW_PIPE_DETAIL_CONNECTION_HPP
