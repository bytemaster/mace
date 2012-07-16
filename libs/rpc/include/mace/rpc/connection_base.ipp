 #ifndef _MACE_RPC_CONNECTION_BASE_IPP_ 
 #define _MACE_RPC_CONNECTION_BASE_IPP_ 
  #include <mace/rpc/connection_base.ipp>
  namespace mace { namespace rpc {

  template<typename Derived>
  void connection_base<Derived>::raw_call(  method_id_type&& meth, param_type&& param, 
                  const typename detail::pending_result<result_type,error_type>::ptr& pr ) {
    if( pr ) {
        auto rid = id_factory.create_request_id(); 
        derived_self.send( message_type( std::move(meth), std::move(param), rid ) );
        results[rid] = pr;
    } else  {
        derived_self.send( message_type( std::move(meth), std::move(param) ) );
    }
  }



  template<typename Derived>
  void connection_base<Derived>::handle( message_type&& m ) {
    if( m.has_method_id() ) {
      auto itr = methods.find( m.get_method_id() );
      if( itr != methods.end() ) {
         derived_self.send( itr->second( m ) );
      } else {
         derived_self.handle_error( connection_error::unknown_method, std::move(m) );
      }
      return;
    } else if( m.has_request_id() ) {
      auto itr = results.find( m.get_request_id() );
      if( itr != results.end() ) {
        if( !m.has_error() ) {
          itr->second->handle_value( m.take_result() ); 
        } else {
          itr->second->handle_error( m.take_error() );
        }
        results.erase(itr);
        return;
      } else {
        elog( "unknown request %1%", m.get_request_id() );
      }
    } 
    elog( "invalid message type.. req %1%  method %2%", m.has_request_id(), m.has_method_id() );
    derived_self.handle_error( connection_error::invalid_message_type, std::move(m) );
  }
  template<typename Derived>
  void connection_base<Derived>::handle_error( error_type&& e, message_type&& msg ) {
    //elog( "%1%", mace::reflect::reflector<message_type_error_type>::to_string(e) );
    derived_self.send( message_type( std::move(msg.take_request_id()), std::move(e) ) );
  }
  template<typename Derived>
  connection_base<Derived>::~connection_base() {
     try { derived_self.close(); }catch(...){}
  }

  template<typename Derived>
  template<typename InterfaceType>
  template<typename MemberPtr, MemberPtr m >
  void connection<Derived>::add_interface_visitor<InterfaceType>::operator()( const char* name ) const {
      typedef typename boost::function_types::result_type<MemberPtr>::type MemberRef;
      typedef typename boost::remove_reference<MemberRef>::type Member;
      typedef typename boost::fusion::traits::deduce_sequence<typename Member::fused_params>::type param_type;
      typedef decltype( boost::bind( &Member::call_fused, (*m_aptr).*m, _1 ) ) functor;

      typename connection_base<connection<Derived> >::method  meth(rpc_recv_functor<param_type, functor>( 
                          boost::bind( &Member::call_fused, (*m_aptr).*m,_1) , m_con ));
      m_con.add_method( std::string(name), std::move(meth) );
  }
  template<typename Derived>
  template<typename Seq, typename Functor>
  typename connection<Derived>::message_type connection<Derived>::rpc_recv_functor<Seq,Functor>::operator()( message_type& m ) {
     typedef typename Derived::io_delegate_type iod;
     message reply;
     reply.set_request_id( m.take_request_id() );
     try {
       Seq paramv;
       if( boost::fusion::size(paramv) ) {
          function_filter<connection> f(m_con);
          if( reply.has_request_id() ) {
             reply.set_result(  
                 iod::pack(f, m_func(iod::template unpack<Seq, function_filter<connection> >( f, m.take_params() ))  ));
          } else {
            m_func(iod::template unpack<Seq,function_filter<connection> >( f, m.take_params() ));
          }
       } else {
         // TODO:  why don't I do anything here???
         elog( "what now... paramsize 0..." ); 
       }
     } catch ( ... ) {
       if(reply.has_request_id() ) {
         function_filter<connection> f(m_con);
         reply.set_error( error_object( connection_error::exception_thrown, boost::current_exception_diagnostic_information() ) );
       }
     }
     return reply;
  }


  } }
#endif
