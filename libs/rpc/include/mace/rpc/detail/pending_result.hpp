#ifndef _MACE_RPC_DETAIL_PENDING_RESULT_HPP_
#define _MACE_RPC_DETAIL_PENDING_RESULT_HPP_
#include <mace/cmt/future.hpp>
#include <mace/rpc/filter.hpp>

namespace mace { namespace rpc { 

  typedef std::vector<char> datavec;

  namespace detail {
    class pending_result : public std::enable_shared_from_this<pending_result> {
      public:
        typedef std::shared_ptr<pending_result> ptr;
        virtual ~pending_result(){}
        virtual void handle_value( datavec&& d ) = 0;
        virtual void handle_error( int32_t code, datavec&& d ) = 0;
    };
    
    class raw_pending_result : public pending_result {
      public:
        raw_pending_result( const mace::cmt::promise<datavec >::ptr& p )
        :prom(p){}
        
        virtual void handle_value( datavec&& d ) {
          if( prom->retain_count() > 1 ) 
              prom->set_value( std::forward<datavec >(d) );
        }
        virtual void handle_error( int32_t code, datavec&& d ) {
          ///TODO: set exception
          //prom->
        }
      private:
        mace::cmt::promise<datavec >::ptr prom; 
    };
    
    template<typename R, typename Connection, typename IODelegate>
    class pending_result_impl : public pending_result {
      public:
        pending_result_impl( Connection& c, const typename mace::cmt::promise<R>::ptr p )
        :prom(p),con(c){}
    
        virtual void handle_value( datavec&& d ) {
          if( prom->retain_count() > 1 ) {
              try {
                 R result;
                 function_filter<Connection> f(con);
                 result = IODelegate::template unpack<R,BOOST_TYPEOF(f)>( f, d );
                 prom->set_value( std::move( result ) );
              } catch ( ... ) {
                 prom->set_exception( boost::current_exception() );
              }
          } 
        }
        virtual void handle_error( int32_t code, datavec&& d ) {
          ///TODO: set exception
          //prom->
        }
    
        typename mace::cmt::promise<R>::ptr prom;
        Connection&                         con;
    };

  } // namespace detail

} } 

#endif
