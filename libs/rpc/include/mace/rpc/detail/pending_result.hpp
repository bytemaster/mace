#ifndef _MACE_RPC_DETAIL_PENDING_RESULT_HPP_
#define _MACE_RPC_DETAIL_PENDING_RESULT_HPP_
#include <mace/cmt/future.hpp>
#include <mace/rpc/filter.hpp>
#include <mace/rpc/error.hpp>


namespace mace { namespace rpc { 

  namespace detail {
    template<typename ResultType, typename ErrorType>
    class pending_result : public std::enable_shared_from_this<pending_result<ResultType,ErrorType> > {
      public:
        typedef std::shared_ptr<pending_result> ptr;
        virtual ~pending_result(){}
        virtual void handle_value( ResultType&& d ) = 0;
        virtual void handle_error( ErrorType&& d ) = 0;
    };
    
    template<typename ResultType, typename ErrorType>
    class raw_pending_result : public pending_result<ResultType,ErrorType> {
      public:
        raw_pending_result( const typename mace::cmt::promise<ResultType>::ptr& p )
        :prom(p){}
        
        virtual void handle_value( ResultType&& d ) {
          if( prom->retain_count() > 1 )  // we only care if someone is waiting
              prom->set_value( std::move<ResultType>(d) );
        }
        virtual void handle_error( ErrorType&& e ) {
          if( prom->retain_count() > 1 )  // we only care if someone is waiting
              prom->set_exception( boost::copy_exception(std::move(e)) );
        }
      private:
        typename mace::cmt::promise<ResultType>::ptr prom; 
    };
    
    /**
     *  requires IODelegate::unpack<R,FF >( f, RT&& )
     *  requires ET::get_message()  ET::code
     */
    template<typename R, typename Connection, typename IODelegate, typename RT, typename ET>
    class pending_result_impl : public pending_result<RT,ET> {
      public:
        pending_result_impl( Connection& c, const typename mace::cmt::promise<R>::ptr p )
        :prom(p),con(c){}
    
        virtual void handle_value( RT&& d ) {
          if( prom->retain_count() > 1 ) {
              try {
                 R result;
                 function_filter<Connection> f(con);
                 result = IODelegate::template unpack<R,decltype(f)>( f, std::move(d) );
                 prom->set_value( std::move( result ) );
              } catch ( ... ) {
                 prom->set_exception( boost::current_exception() );
              }
          } 
        }
        virtual void handle_error( ET&& e ) {
          prom->set_exception( 
            boost::copy_exception( 
              mace::rpc::exception() << mace::rpc::error_message( e.message ? *e.message : std::string("") )
                                     << mace::rpc::error_code( e.code ) ) ); 
        }
    
        typename mace::cmt::promise<R>::ptr prom;
        Connection&                         con;
    };

  } // namespace detail

} } 

#endif
