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
      typedef boost::shared_ptr<connection_base> ptr;
      method_map  methods;
      pending_map results;
      int32_t     next_method_id;
      int32_t     next_req_id;

      connection_base():next_method_id(0),next_req_id(0){}
      virtual ~connection_base(){}

      virtual void close() {}
      virtual void send( message&& m ) = 0;
      virtual void handle_error( message::error_type, const std::string& message ) = 0;

      void break_promises() {
        auto itr = results.begin();
        while( itr != results.end() ) {
          itr->second->handle_error( message::broken_promise, datavec() );
          ++itr;
        }
        results.clear();
      }

      /**
       *  This method should be called by derived classes when they
       *  decode a message.
       */
      void handle( message&& m ) {
        if( m.meth.size() ) {
          auto itr = methods.find( m.meth );
          if( itr != methods.end() ) {
             send( itr->second( m ) );
          } else {
             handle_error( message::unknown_method, m.meth ); 
          }
        } else if( m.id ) {
          auto itr = results.find( *m.id );
          if( itr != results.end() ) {
            if( !m.err ) {
              itr->second->handle_value( std::move( m.data ) );
            } else {
              itr->second->handle_error( m.err, std::move(m.data) );
            }
            results.erase(itr);
          } else {
             handle_error( message::invalid_response, "Unexpected response id "+ boost::lexical_cast<std::string>(*m.id)  ); 
          }
        } else {
          handle_error( message::invalid_response, "no method or request id" );
        }
      }
  };
  
} } }

#endif // _MACE_RPC_DETAIL_CONNECTION_BASE_HPP_
