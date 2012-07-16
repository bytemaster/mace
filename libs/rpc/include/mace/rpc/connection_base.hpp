#ifndef _MACE_RPC_CONNECTION_BASE_HPP_
#define _MACE_RPC_CONNECTION_BASE_HPP_
#include <mace/rpc/message.hpp>
#include <mace/rpc/detail/pending_result.hpp>
#include <boost/unordered_map.hpp>
#include <boost/signals.hpp>

namespace mace { namespace rpc { 

  struct connection_error {
    enum connection_error_enum {
      none                 = 0,
      invalid_message_type = 2,
      invalid_response     = 3,
      unknown_method       = 4,
      exception_thrown     = 5,
      broken_promise       = 6
    };
    connection_error( connection_error_enum e = none ):value(e){}
    operator connection_error_enum()const { return value; }

    private:
      connection_error_enum value;
  };

  /**
   *
   *
   *  @tparam Derived implements:
   *    typedef type message_type;
   *    void send( message_type&& m )
   *    void handle_error( connection_error_enum, message_type&& m )
   *      
   */
  template<typename Derived>
  class connection_base : public boost::noncopyable {
    public:
      typedef mace::rpc::message                     message_type; 
      typedef boost::function<message_type(message_type&)>   method;
      typedef message_type::method_id_type           method_id_type;
      typedef message_type::param_type               param_type;
      typedef message_type::result_type              result_type;
      typedef message_type::request_id_type          request_id_type;
      typedef message_type::error_type               error_type;
      typedef detail::pending_result<param_type,error_type>   pending_result;

      
       void send( message_type&& m ) { derived_self.send( std::move(m) ); }
       void close()                  { derived_self.close();              }

      /**
       *  @return null if no method with ID known.
       */
      const method* get_method( const method_id_type& name )const {
        auto i = methods.find(name);
        if( i != methods.end() ) return &i->second;
        return NULL;
      }

      /**
       *  Add a method with the given ID
       */
      void          add_method( const method_id_type& name, method&& m ) {
        methods[name] = std::move(m);
      }

      /**
       *  Add a method and generate an ID
       */
      method_id_type   add_method( method&& m ) {
        auto id = create_method_id();
        add_method( id, std::move(m) );
        return id;
      }

      /**
       *  raw interface, returns the serialized data given the 
       *              serialized parameters.
       *
       *  @param mid method name to call
       */
      cmt::future<result_type> raw_call( method_id_type&& mid, param_type&& param ) {
          typename mace::cmt::promise<result_type>::ptr prom(new mace::cmt::promise<result_type>() );
          typename detail::pending_result<result_type,error_type>::ptr pr( new detail::raw_pending_result<result_type,error_type>(prom) );
          raw_call( std::move(mid), std::move(param), pr );
          return prom;
      }

      method_id_type create_method_id() {
        return id_factory.create_method_id();
      }

      /**
       *  Emited when read loop exits.
       */
      boost::signal<void()> closed;
  
      ~connection_base(); 

    protected:
      /**
       *  Call method ID with param and use the given result handler if specified.  
       *
       *  @param pr - result handler, if not specified result will be ignored.
       */
      void raw_call(  method_id_type&& meth, param_type&& param, 
                      const typename detail::pending_result<result_type,error_type>::ptr& pr );

      connection_base(Derived& s):derived_self(s){}

      void break_promises() {
        auto itr = results.begin();
        while( itr != results.end() ) {
          itr->second->handle_error( error_type(connection_error::broken_promise)  );
          ++itr;
        }
        results.clear();
      }

      void handle( message_type&& m );

      void handle_error( error_type&& e, message_type&& msg );

    private:
      connection_base(); // must pass ref to derived class to get proper alignment
      typedef boost::unordered_map<method_id_type,method> method_map;
      typedef std::map<int32_t,typename detail::pending_result<result_type,error_type>::ptr>       pending_map;
      message_type::id_factory                            id_factory;

      Derived&                                            derived_self;
      method_map                                          methods;
      pending_map                                         results;
  };
} } // mace::rpc 

#endif //_MACE_RPC_CONNECTION_BASE_HPP_
