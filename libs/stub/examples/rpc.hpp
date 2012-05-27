/**
 *  @file rpc.hpp
 */

/**
 *  @example rpc.hpp
 *  @brief Defines a simple UDP based RPC interface using Boost.Serialization
 */
#ifndef _MACE_STUB_RPC_HPP_
#define _MACE_STUB_RPC_HPP_
#include <mace/stub/mirror_interface.hpp>
#include <mace/stub/ptr.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio.hpp>
#include "vector_serialize.hpp"

/**
 *  For each method on the interface, create a functor that will accept the
 *  methods parameters, seralize them, and send them out the socket.
 */
//! [RPC Client Constructor]
namespace mace { namespace stub {

template<typename InterfaceType,typename InterfaceDelegate=mirror_interface>
class rpc_client : public stub::ptr<InterfaceType,InterfaceDelegate> {
  public:
    rpc_client()
    :m_ios(),m_sock(m_ios) {
      vtable_reflector<InterfaceType,InterfaceDelegate>::visit( set_visitor( *this ) );
    }
//! [RPC Client Constructor]

   bool connect_to( const std::string& host, uint16_t port ) {
       m_sock.open(boost::asio::ip::udp::v4());
       m_ep = boost::asio::ip::udp::endpoint( boost::asio::ip::address::from_string(host), port );
   }

   //! [RPC Client invoke]
   std::string invoke( const char* name, const std::string& params ) {
     std::ostringstream os;
     boost::archive::binary_oarchive oa(os);
     std::string n(name);
     oa << n;
     oa << params;

     m_sock.send_to( boost::asio::buffer( os.str() ), m_ep );

     boost::asio::ip::udp::endpoint rep;
     std::vector<char> recv_buf(2048);
     size_t len = m_sock.receive_from( boost::asio::buffer(recv_buf), rep );
     return std::string(&recv_buf.front(),len);
   }
   //! [RPC Client invoke]
   private:
     //! [RPC Client Visitor]
     struct set_visitor {
       set_visitor( rpc_client& ci )
       :c(ci){}

       template<typename MemberPtr, MemberPtr m>
       void operator()( const char* name )const {
         typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
         typedef typename boost::remove_reference<member_ref>::type member;
         ((*c).*m) = rpc_functor<typename member::fused_params, 
                                 typename member::result_type>( c, name );
       }
       rpc_client& c;
     };
     //! [RPC Client Visitor]

     //! [RPC Client Functor]
     template<typename Seq, typename ResultType>
     struct rpc_functor {
       rpc_functor( rpc_client& c, const char* name )
       :m_client(c),m_name(name){}

       ResultType operator()( const Seq& params )const {
          // serialize the parameters
          std::ostringstream os; 
          boost::archive::binary_oarchive oa(os);
          serialize_fusion_vector(oa, params);
          // make a call and store the result into the input stream; 
          std::istringstream is(m_client.invoke( m_name, os.str() ) );

          // unpack the result type
          ResultType  ret_val;
          boost::archive::binary_iarchive ia(is);
          ia >> ret_val;
          return ret_val;
       }
       const char* m_name;
       rpc_client& m_client;
     };
     //! [RPC Client Functor]
       boost::asio::ip::udp::endpoint m_ep;
       boost::asio::io_service        m_ios;
       boost::asio::ip::udp::socket   m_sock;
};


/**
 *  Create a server socket that accepts new messages and then
 *  unpacks the parameters and then invokes them on the object.
 */
class rpc_server {
    public:
       template<typename T>
       friend struct get_visitor;

       template<typename InterfaceType>
       rpc_server( const mace::stub::ptr<InterfaceType>& v )
       :stub_ptr(v) {
           mace::stub::ptr<InterfaceType>& i = 
              boost::any_cast<mace::stub::ptr<InterfaceType>&>(stub_ptr);

           mace::stub::vtable_reflector<InterfaceType>::visit( get_visitor<InterfaceType>(*i, *this ) );
       }

       void listen( uint16_t port ) {
            using namespace boost::asio::ip;
            boost::asio::io_service io_service;
            udp::socket  socket( io_service, udp::endpoint(udp::v4(), port ) );
            std::vector<char>  recv_buf(2048);
            for( ;; )
            {
                udp::endpoint remote_ep;
                boost::system::error_code err;
                size_t bytes_recv = socket.receive_from( boost::asio::buffer(recv_buf),
                                     remote_ep, 0, err );
                if( err && err != boost::asio::error::message_size )
                    throw boost::system::system_error(err);

                std::string         buf(&recv_buf.front(),bytes_recv );
                std::string         method;
                std::string         params;
                {
                    std::istringstream iss( buf );
                    boost::archive::binary_iarchive ia(iss);
                    ia >> method;
                    ia >> params;
                }
                boost::system::error_code ignored_error;
                socket.send_to( boost::asio::buffer( methods[method](params) ),
                                remote_ep, 0, ignored_error );
            }
       }

       boost::function<std::string(const std::string)>& operator[]( const std::string& name ) 
       { return methods[name]; }

    private:

  //! [RPC Server Functor and Visitor]
      // rpc_server
       template<typename Seq, typename Functor>
       struct rpc_functor {
           rpc_functor( Functor f )
           :m_func(f){}

           std::string operator()( const std::string& params )const {
                Seq paramv;
                std::istringstream is(params);
                boost::archive::binary_iarchive ia(is);
                deserialize_fusion_vector(ia,paramv);                    

                std::ostringstream os;
                boost::archive::binary_oarchive oa(os);
                typename boost::remove_reference<Functor>::type::result_type r = m_func(paramv);
                oa << r;
                return os.str();
           }
           Functor m_func;
       };


       template<typename T>
       struct get_visitor {
          get_visitor( vtable<T>& vt, rpc_server& si )
          :v(vt),s(si){}

          template<typename MemberPtr, MemberPtr m>
          void operator()( const char* name )const {
            typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
            typedef typename boost::remove_reference<member_ref>::type member;
            s.methods[name] = rpc_functor<typename member::fused_params, 
                                BOOST_TYPEOF( v.*m )&>(v.*m);
          }
          vtable<T>& v;
          rpc_server& s;
       };

       std::map<std::string, boost::function<std::string(const std::string)> > methods;
  //! [RPC Server Functor and Visitor]
       boost::any stub_ptr;
};

} } // namespace mace::stub

#endif
