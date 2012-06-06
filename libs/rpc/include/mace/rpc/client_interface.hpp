#ifndef BOOST_PP_IS_ITERATING
  #ifndef BOOST_CLIENT_INTERFACE_HPP
  #define BOOST_CLIENT_INTERFACE_HPP
  #include <boost/bind.hpp>
  #include <boost/preprocessor/repetition.hpp>
  #include <boost/preprocessor/seq/for_each.hpp>

  #include <boost/fusion/container/vector.hpp>
  #include <boost/fusion/container/generation/make_vector.hpp>
  #include <boost/fusion/functional/generation/make_fused_function_object.hpp>
  #include <boost/fusion/functional/generation/make_unfused.hpp>
  #include <mace/stub/void.hpp>
  #include <mace/stub/vtable.hpp>

  namespace mace { namespace rpc { 
    /**
     *  @brief Specialized to mirror the member 
     *         variable/method pointed to by MemberPtr
     */
    template<typename MemberPtr, typename ConnectionType>
    struct client_member;

    template<typename ConnectionType>
    struct client_member_base {
      client_member_base(){}
      void set( const std::string& mid, const typename ConnectionType::ptr& con ) {
        m_method_id    = mid;
        m_con          = con;
      }
      protected:
        std::string                      m_method_id;
        typename ConnectionType::ptr     m_con;
    };
    
    /**
     *  @brief Interface Delegate that adapts an interface for asynchronous
     *         calls over the network
     *  
     *  To specialize how a particular member is mirrored define
     *  the partial specialization of client_member for your type.
     *
     *  @code
     *  template<typename Type, typename Class>
     *  client_member<Type(Class::*)> 
     *  @endcode
     */
    template<typename ConnectionType>
    struct client_interface {
      /**
       * @brief Implements the InterfaceDelegate meta-function to 
       *      determine what type to create to mirror MemberPointer 
       *      in mace::reflect::vtable used by mace::stub::ptr
       */
      template<typename MemberPointer>
      struct calculate_type {
        typedef client_member<MemberPointer,ConnectionType>  type; 
      };

      template<typename ClientType>
      static void set_vtable( ClientType& cl, typename ConnectionType::ptr& t ) {
        mace::stub::visit( cl, set_visitor<typename ClientType::vtable_type>( *cl, t ) );
      }

      template<typename VTableType>
      class set_visitor {
        public:
          set_visitor( VTableType& vt, typename ConnectionType::ptr& con )
          :m_con(con),vtbl(vt){}

          template<typename M, M m>
          void operator()( const char* name )const {
            (vtbl.*m).set( name, m_con );
          }
        private:
          typename ConnectionType::ptr&  m_con;
          VTableType&                    vtbl;
      };
    };


  #define PARAM_NAME(z,n,type)         BOOST_PP_CAT(a,n)
  #define PARAM_TYPE_NAME(z,n,type)   BOOST_PP_CAT(typename A,n)
  #define PARAM_TYPE(z,n,type)   BOOST_PP_CAT(A,n)
  #define PARAM_ARG(z,n,type)     PARAM_TYPE(z,n,type) PARAM_NAME(z,n,type)

#        ifndef BOOST_CLIENT_IMPL_SIZE
#           define BOOST_CLIENT_IMPL_SIZE 8
#        endif

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, BOOST_CLIENT_IMPL_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <mace/rpc/client_interface.hpp>
#       include BOOST_PP_ITERATE()

  #undef PARAM_NAME
  #undef PARAM_TYPE
  #undef PARAM_ARG

   } } // namespace mace::rpc
  #endif // BOOST_CLIENT_INTERFACE_HPP

#else // BOOST_PP_IS_ITERATING

#define n BOOST_PP_ITERATION()
#define PARAM_NAMES          BOOST_PP_ENUM(n,PARAM_NAME,A) // name_N
#define PARAM_ARGS           BOOST_PP_ENUM(n,PARAM_ARG,A) // TYPE_N name_N
#define PARAM_TYPE_NAMES     BOOST_PP_ENUM(n,PARAM_TYPE_NAME,A) // typename TYPE_N
#define PARAM_TYPES          BOOST_PP_ENUM(n,PARAM_TYPE,A) // TYPE_N

template<typename ConnectionType, typename R, typename Class BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct client_member<R(Class::*)(PARAM_TYPES)const,ConnectionType> : public client_member_base<ConnectionType> {
  typedef typename mace::stub::adapt_void<R>::result_type    result_type;
  typedef mace::cmt::future<result_type>                        future_type;
  typedef client_member                                          self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                     fused_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>       traits;
  static const bool                                              is_const = true;

  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type   signature;

  inline future_type operator()( PARAM_ARGS )const {
    return (*this)( boost::fusion::make_vector(PARAM_NAMES) );
  }
  inline future_type operator() ( const fused_params& fp )const {
    BOOST_ASSERT(this->m_con);
    return this->m_con->template call_fused<result_type,fused_params>( std::string(this->m_method_id), fp );
  }
  inline future_type operator() ( fused_params&& fp )const {
    BOOST_ASSERT(this->m_con);
    return this->m_con->template call_fused<result_type,fused_params>( std::string(this->m_method_id), std::move(fp) );
  }
  inline void notice( PARAM_ARGS )const {
    notice( boost::fusion::make_vector(PARAM_NAMES) );
  }
  inline void notice( const fused_params& fp )const {
    BOOST_ASSERT(this->m_con);
    this->m_con->notice_fused( this->m_method_id, fp );
  }
};

template<typename ConnectionType, typename R, typename Class  BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct client_member<R(Class::*)(PARAM_TYPES),ConnectionType>  : public client_member_base<ConnectionType> {
  typedef typename mace::stub::adapt_void<R>::result_type   result_type;
  typedef mace::cmt::future<result_type>                       future_type;
  typedef client_member                                         self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                    fused_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>      traits;
  typedef boost::function<result_type(const fused_params&)>     delegate_type;
  static const bool                                             is_const  = false;

  // boost::result_of
  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type signature;

  inline future_type operator()( PARAM_ARGS ) {
    return (*this)( boost::fusion::make_vector(PARAM_NAMES) );
  }
  inline future_type operator() ( const fused_params& fp ) {
    BOOST_ASSERT(this->m_con);
    return this->m_con->template call_fused<result_type,fused_params>( this->m_method_id, fp );
  }

  inline future_type operator() ( fused_params&& fp ) {
    BOOST_ASSERT(this->m_con);
    return this->m_con->template call_fused<result_type,fused_params>( std::string(this->m_method_id), std::move(fp) );
  }

  inline void notice( PARAM_ARGS )const {
    return notice( boost::fusion::make_vector(PARAM_NAMES) );
  }
  inline void notice( const fused_params& fp )const {
    BOOST_ASSERT(this->m_con);
    this->m_con->notice_fused( this->m_method_id, fp );
  }
};


#undef n
#undef PARAM_NAMES         
#undef PARAM_ARGS        
#undef PARAM_TYPE_NAMES 
#undef PARAM_TYPES     

#endif // BOOST_PP_IS_ITERATING
