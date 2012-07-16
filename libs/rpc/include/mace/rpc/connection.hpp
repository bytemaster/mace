#ifndef _MACE_RPC_CONNECTION_HPP_
#define _MACE_RPC_CONNECTION_HPP_
#include <mace/rpc/connection_base.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/fusion/include/make_fused.hpp>
#include <boost/fusion/support/deduce_sequence.hpp>

namespace mace { namespace rpc { 

  /**
   *  @tparam io_delegate_type must implement the following expressions
   *
   *    datavec io_delegate_type::pack<T>    ( Filter&, const T& )
   *    size_t  io_delegate_type::packsize<T>( Filter&, const T& )
   *    R       io_delegate_type::unpack<R>  ( Filter&, const datavec& )
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
  template<typename Derived>
  class connection : public connection_base< connection<Derived> > {
    public:
      typedef mace::rpc::message  message_type;
      typedef connection_base<connection<Derived> >            base_type;
      friend class connection_base<connection<Derived> >;

      // forward these calls to derived class
      void send( message&& m ) { static_cast<Derived*>(this)->send( std::move(m) ); }
      void close()             { static_cast<Derived*>(this)->close();              }
      void handle_error( connection_error e, message_type&& m ) { 
        static_cast<Derived*>(this)->handle_error( e, std::move(m) ); 
      }

      template<typename R, typename ParamSeq>
      cmt::future<R> call_fused( const message_type::method_id_type& id, ParamSeq&& params ) {
        return call_fused<R>( std::string(id), std::forward<ParamSeq>(params) );
      }
      template<typename ParamSeq>
      void notice_fused( const message_type::method_id_type& id, ParamSeq&& params ) {
        notice_fused( std::string(id), std::move(params) );
      }

      template<typename R, typename ParamSeq>
      cmt::future<R> call_fused( message_type::method_id_type&& id, ParamSeq&& params ) {
        BOOST_STATIC_ASSERT( boost::fusion::traits::is_sequence<typename std::decay<ParamSeq>::type>::value );
        typedef typename Derived::io_delegate_type iod;
        typedef message_type::result_type          result_type;
        typedef message_type::error_type          error_type;
        // TODO: filter params for non-const references and add them as additional 'return values'
        //       then pass the extra references to the pending_result impl.
        //       References must remain valid until pr->prom->wait() returns.
        auto pr = std::make_shared<detail::pending_result_impl<R,connection,iod,result_type,error_type> >( std::ref(*this), mace::cmt::promise<R>::make() );
        function_filter<connection> f(*this);
        this->raw_call( std::move(id), Derived::io_delegate_type::pack(f, params), pr );
        return pr->prom;
      }

      template<typename ParamSeq>
      void notice_fused( message_type::method_id_type&& id, ParamSeq&& params ) {
        BOOST_STATIC_ASSERT( boost::fusion::traits::is_sequence<typename std::decay<ParamSeq>::type>::value );
        typedef typename Derived::io_delegate_type iod;
        function_filter<connection> f(*this);
        raw_call( std::move(id), iod::pack(f, params), detail::pending_result<message_type::result_type,message_type::error_type>::ptr() );
      }

      //using connection_base<connection<Derived> >::add_method;
      using base_type::add_method;

      /**
       *  Adds a new 'anonymous' method and returns the name generated for it.
       */
      template<typename Signature>
      message_type::method_id_type add_method( const boost::function<Signature>& m ) {
        typedef typename boost::function_types::parameter_types<Signature>::type  mpl_param_types;
        typedef typename boost::fusion::result_of::as_vector<mpl_param_types>::type param_types;

        message_type::method_id_type mid = this->id_factory.create_method_id();
        typedef decltype( boost::fusion::make_fused( m ) ) fused_func;
        add_method( mid, rpc_recv_functor<param_types,fused_func >( boost::fusion::make_fused(m), *this ) );
        return  mid;
      }

      /**
       *  @return a functor that will call this->call(name, params) 
       */
      template<typename Signature>
      boost::function<Signature> create_callback( const message_type::method_id_type& name ) {
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
      
        message_type operator()( message_type& m );
      
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
        void operator()(const char* name )const;

        connection&                       m_con;
        mace::stub::ptr<InterfaceType>&   m_aptr;
      };

    protected:
      connection()
      :base_type(*this){}
  };

} } // mace::rpc

#endif //  _MACE_RPC_CONNECTION_HPP_
