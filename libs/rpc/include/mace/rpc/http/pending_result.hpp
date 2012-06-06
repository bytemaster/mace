#ifndef _MACE_RPC_DETAIL_PENDING_RESULT_HPP_
#define _MACE_RPC_DETAIL_PENDING_RESULT_HPP_
#include <mace/cmt/future.hpp>

namespace mace { namespace rpc { 

  namespace detail {
    class pending_result : public boost::enable_shared_from_this<pending_result> {
      public:
        typedef boost::shared_ptr<pending_result> ptr;
        virtual ~pending_result(){}
        virtual void handle_value( std::vector<char>&& d ) = 0;
        virtual void handle_error( int32_t code, std::vector<char>&& d ) = 0;
    };
    
    class raw_pending_result : public pending_result {
      public:
        raw_pending_result( const mace::cmt::promise<std::vector<char> >::ptr& p )
        :prom(p){}
        
        virtual void handle_value( std::vector<char>&& d ) = 0;
          if( prom->retain_count() > 1 ) 
              prom->set_value( std::forward<std::vector<char> >(d) );
        }
        virtual void handle_error( int32_t code, std::vector<char>&& d ) {
          ///TODO: set exception
          //prom->
        }
      private:
        mace::cmt::promise<std::vector<char> >::ptr prom; 
    };
    
    template<typename R, typename Connection, typename IODelegate>
    class pending_result_impl : public pending_result {
      public:
        pending_result_impl( Connection& c, const mace::cmt::promise<R>::ptr p )
        :prom(p),con(c){}
    
        virtual void handle_value( std::vector<char>&& d ) = 0;
          if( prom->retain_count() > 1 ) {
              try {
                 R result;
                 IODelegate::unpack<R>( con, d );
                 prom->set_value( std::move( result ) );
              } catch ( ... ) {
                 prom->set_exception( boost::current_exception() );
              }
          } 
        }
        virtual void handle_error( int32_t code, std::vector<char>&& d ) {
          ///TODO: set exception
          //prom->
        }
    
        mace::cmt::promise<R>::ptr prom;
        Connection&                con;
    };

  } // namespace detail

} } 

#endif
