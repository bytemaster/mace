#ifndef _MACE_RPC_DETAIL_CONNECTION_BASE_HPP_
#define _MACE_RPC_DETAIL_CONNECTION_BASE_HPP_
#include <mace/rpc/connection_base.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>

namespace mace { namespace rpc { namespace detail {
  typedef boost::unordered_map<std::string,method> method_map;
  typedef std::map<int32_t,pending_result::ptr>    pending_map;

  /**
   *  The 'protected implementation' of rpc::connection_base.  Derived
   *  classes should also follow the primple pattern and derive from
   *  detail::connection_base.
   */
  class connection_base {
    public:
      typedef std::shared_ptr<connection_base> ptr;
      mace::rpc::connection_base& self;
      method_map  methods;
      pending_map results;
      int32_t     next_method_id;
      int32_t     next_req_id;


      connection_base(mace::rpc::connection_base& mself):self(mself),next_method_id(0),next_req_id(0){}
      virtual ~connection_base(){}

      virtual void close() {}
      virtual void send( message&& m ) = 0;
      /**
       *  Default implementation replies with an error object.
       */
      virtual void handle_error( message::error_type, message&& m );

      void break_promises();

      /**
       *  This method should be called by derived classes when they
       *  decode a message.
       */
      void handle( message&& m );
  };

} //namespace detail

} } // namespace mace::rpc

#endif // _MACE_RPC_DETAIL_CONNECTION_BASE_HPP_
