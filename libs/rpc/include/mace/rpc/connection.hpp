#ifndef _MACE_RPC_CONNECTION_HPP_
#define _MACE_RPC_CONNECTION_HPP_
#include <mace/rpc/connection_base.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/make_shared.hpp>
#include <mace/rpc/detail/pending_result.hpp>
#include <mace/rpc/filter.hpp>
#include <mace/stub/ptr.hpp>
#include <boost/fusion/support/deduce.hpp>
#include <boost/fusion/support/deduce_sequence.hpp>
#include <boost/fusion/include/make_fused.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>

#include <boost/signals.hpp>

namespace mace { namespace rpc { 

  /**
   *  @tparam IODelegate must implement the following expressions
   *
   *    datavec IODelegate::pack<T>    ( Filter&, const T& )
   *    size_t  IODelegate::packsize<T>( Filter&, const T& )
   *    R       IODelegate::unpack<R>  ( Filter&, const datavec& )
   *
   *  @tparam Filter must implement the following expressions
   *    template<typename T,typename R>
   *    R operator()( const T& v ) -> returns the data to pack in place of v
   *
   *    template<typename T, typename R>
   *    void operator()( R& r, T& v )  -> given r, assign to v. 
   *    
   *    template<typename T>
   *    const bool is_filtered(const T*)const { return false; }
   *
   *    template<typename Signature>
   *    const bool is_filtered(const boost::function<Signature>*)const { return true; }
   *
   *    unpack( Filter& f, Stream& s, T& v ) {
   *      if( f.is_filtered(&v) ) { // optimizing compiler should reduce this to 1 use case.
   *        boost::remove_reference<decltype(f(T()))>::type r;
   *        unpack(f,s,r)
   *        f( r, v );
   *      }
   *      else {
   *        // do your thing...
   *      }
   *    }
   */
  template<typename IODelegate >
  class connection : public connection_base {
    public:
      typedef std::shared_ptr<connection> ptr;
      typedef IODelegate io_delegate_type;

      boost::signal<void()> closed;

      template<typename R, typename ParamSeq>
      cmt::future<R> call_fused( const std::string& id, ParamSeq&& params ) {
        return call_fused<R>( std::string(id), std::forward<ParamSeq>(params) );
      }
      template<typename ParamSeq>
      void notice_fused( const std::string& id, ParamSeq&& params ) {
        notice_fused( std::string(id), std::move(params) );
      }

      template<typename R, typename ParamSeq>
      cmt::future<R> call_fused( std::string&& id, ParamSeq&& params ) {
        BOOST_STATIC_ASSERT( boost::fusion::traits::is_sequence<typename std::decay<ParamSeq>::type>::value );
        // TODO: filter params for non-const references and add them as additional 'return values'
        //       then pass the extra references to the pending_result impl.
        //       References must remain valid until pr->prom->wait() returns.
        auto pr = std::make_shared<detail::pending_result_impl<R,connection,IODelegate> >( std::ref(*this), mace::cmt::promise<R>::make() );
        function_filter<connection> f(*this);
        raw_call( std::move(id), IODelegate::pack(f, params), pr );
        return pr->prom;
      }

      template<typename ParamSeq>
      void notice_fused( std::string&& id, ParamSeq&& params ) {
        BOOST_STATIC_ASSERT( boost::fusion::traits::is_sequence<typename std::decay<ParamSeq>::type>::value );
        function_filter<connection> f(*this);
        raw_call( std::move(id), IODelegate::pack(f, params), detail::pending_result::ptr() );
      }

      using connection_base::add_method;
      /**
       *  Adds a new 'anonymous' method and returns the name generated for it.
       */
      template<typename Signature>
      std::string add_method( const boost::function<Signature>& m ) {
        typedef typename boost::function_types::parameter_types<Signature>::type  mpl_param_types;
        typedef typename boost::fusion::result_of::as_vector<mpl_param_types>::type param_types;

        // TODO:  convert m into a rpc::method and add it.
        std::string mid = create_method_id();
        typedef decltype( boost::fusion::make_fused( m ) ) fused_func;
        //add_method( mid, rpc_recv_functor<param_types,boost::function<Signature> >( m, *this ) );
        add_method( mid, rpc_recv_functor<param_types,fused_func >( boost::fusion::make_fused(m), *this ) );
        return  mid;
      }

      /**
       *  @return a functor that will call this->call(name, params) 
       */
      template<typename Signature>
      boost::function<Signature> create_callback( const std::string& name ) {
        typedef typename boost::function_types::parameter_types<Signature>::type  mpl_param_types;
        typedef typename boost::fusion::result_of::as_vector<mpl_param_types>::type param_types;
        typedef typename boost::function_types::result_type<Signature>::type  R;

        return boost::fusion::make_unfused( boost::bind<R>(
           [=]( const param_types& p ) { return this->call_fused<R>(name,p); },
            _1) );
      }

      /**
       *  Converts fused functor into rpc::method compatiable functor for
       *  this connection type.
       */
      template<typename Seq, typename Functor>
      struct rpc_recv_functor {
        rpc_recv_functor( Functor f, connection& c )
        :m_func(f),m_con(c){ }
      
        message operator()( message& m ) {
          message reply;
          reply.id = m.id;
          try {
            Seq paramv;
            if( boost::fusion::size(paramv) ) {
               function_filter<connection> f(m_con);
               if( m.id ) {
                  reply.data = 
                      IODelegate::pack(f, m_func(IODelegate::template unpack<Seq, function_filter<connection> >( f, m.data )) );
               } else {
                 m_func(IODelegate::template unpack<Seq,function_filter<connection> >( f, m.data ));
               }
            }
          } catch ( ... ) {
            if( m.id ) {
              function_filter<connection> f(m_con);
              reply.err  = rpc::message::exception_thrown;
              reply.data = IODelegate::pack( f, boost::current_exception_diagnostic_information() );
            }
          }
          return reply;
        }
      
        Functor     m_func;
        connection& m_con;
      };



      /**
       *  Visits each method on the any_ptr<InterfaceType> and adds it to the connection object.
       */
      template<typename InterfaceType>
      struct add_interface_visitor {
        add_interface_visitor( connection& c, mace::stub::ptr<InterfaceType>& s )
        :m_con(c),m_aptr(s){}
      
        template<typename MemberPtr,  MemberPtr m>
        void operator()(const char* name )const  {
            typedef typename boost::function_types::result_type<MemberPtr>::type MemberRef;
            typedef typename boost::remove_reference<MemberRef>::type Member;
            typedef typename boost::fusion::traits::deduce_sequence<typename Member::fused_params>::type param_type;
            typedef decltype( boost::bind( &Member::call_fused, (*m_aptr).*m, _1 ) ) functor;
           // m_con.add_method( std::string(name), method(rpc_recv_functor<param_type, Member&>( (*m_aptr).*m, m_con )) );
            m_con.add_method( std::string(name), method(rpc_recv_functor<param_type, functor>( boost::bind( &Member::call_fused, (*m_aptr).*m,_1) , m_con )) );
        }
        connection&                       m_con;
        mace::stub::ptr<InterfaceType>&   m_aptr;
      };

    protected:
      connection( detail::connection_base* b ):connection_base(b){ }
      connection();
  };

} }

#endif 
