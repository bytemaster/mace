#ifndef BOOST_PP_IS_ITERATING
  #ifndef BOOST_CMT_ACTOR_INTERFACE_HPP
  #define BOOST_CMT_ACTOR_INTERFACE_HPP
  #include <boost/bind.hpp>
  #include <boost/cmt/future.hpp>
  #include <boost/preprocessor/repetition.hpp>
  #include <boost/preprocessor/seq/for_each.hpp>

  #include <boost/fusion/container/vector.hpp>
  #include <boost/fusion/container/generation/make_vector.hpp>
  #include <boost/fusion/functional/generation/make_fused_function_object.hpp>
  #include <boost/fusion/functional/generation/make_unfused.hpp>
  #include <boost/reflect/void.hpp>
  #include <boost/signals.hpp>
  #include <boost/reflect/vtable.hpp>

  namespace boost { namespace cmt {
    /**
     *  @brief Specialized to mirror the member 
     *         variable/method pointed to by MemberPtr
     */
    template<typename MemberPtr>
    struct future_member;

    namespace detail {
        class future_member_base {
          public:
            void set_strand( boost::asio::strand* s){m_strand = s;}
          protected:
           boost::asio::strand* m_strand;
        };

        namespace future_interface {
          #ifndef DOXYGEN
          /**
           *  Assigns VTableType's functors with values from T
           */
          template<typename T, typename VTableType>
          class set_visitor {
            public:
              set_visitor( VTableType& vt, T& self, boost::asio::strand* s )
              :m_strand(s),m_self(self),vtbl(vt){}

              template<typename MemberPtr, MemberPtr m>
              void operator()( const char* name )const {
                typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
                typedef typename boost::remove_reference<member_ref>::type member;
                (vtbl.*m).set_delegate( &m_self, member::template get_member_ptr<T>(), m_strand );
              }
            private:
              boost::asio::strand*   m_strand;
              T&                     m_self;
              VTableType&            vtbl;
          };

        
          template<typename Interface, typename Delegate, typename VTableType>
          class set_visitor<boost::reflect::vtable<Interface,Delegate>, VTableType> {
            public:
              typedef boost::reflect::vtable<Interface,Delegate> T;

              set_visitor( VTableType& vt, T& self, boost::asio::strand* s )
              :m_strand(s), m_self(self),vtbl(vt){}

              template<typename MemberPtr, MemberPtr m>
              void operator()( const char* name )const {
                typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
                typedef typename boost::remove_reference<member_ref>::type member;
                (vtbl.*m) = boost::bind(boost::ref(m_self.* member::template get_member_ptr<T>()), _1 );
                (vtbl.*m).set_strand( m_strand );
              }
            private:
              boost::asio::strand*  m_strand;
              T&                    m_self;
              VTableType&           vtbl;
          };
         
          #endif 
        }
    } // namespace detail
    
    /**
     *  @brief Interface Delegate that mirrors the 
     *         reflected interface without any transformation.
     *  
     *  To specialize how a particular member is mirrored define
     *  the partial specialization of future_member for your type.
     *
     *  @code
     *  template<typename Type, typename Class>
     *  future_member<Type(Class::*)> 
     *  @endcode
     */
    struct future_interface 
    {
      /**
       * @brief Implements the InterfaceDelegate meta-function to 
       *      determine what type to create to mirror MemberPointer 
       *      in boost::reflect::vtable used by boost::reflect::any_ptr
       */
      template<typename MemberPointer>
      struct calculate_type {
        typedef future_member<MemberPointer>  type; 
      };

      template<typename T, typename VTableType>
      static void set_vtable( VTableType& vtable, T& value, boost::asio::strand* ab ) {
        boost::reflect::vtable_reflector<typename VTableType::interface_type>::visit( &vtable,
                    detail::future_interface::set_visitor<T,VTableType>(vtable,value,ab) );
      }
      template<typename T, typename VTableType>
      static void set_vtable( VTableType& vtable, const T& value, boost::asio::strand* ab ) {
        boost::reflect::vtable_reflector<typename VTableType::interface_type>::visit( &vtable,
                    detail::future_interface::set_visitor<T,VTableType>(vtable,value,ab) );
      }
    };

  #define PARAM_NAME(z,n,type)         BOOST_PP_CAT(a,n)
  #define PARAM_PLACE_HOLDER(z,n,type) BOOST_PP_CAT(_,BOOST_PP_ADD(n,1) )
  #define PARAM_TYPE_NAME(z,n,type)   BOOST_PP_CAT(typename A,n)
  #define PARAM_TYPE(z,n,type)   BOOST_PP_CAT(A,n)
  #define PARAM_ARG(z,n,type)     PARAM_TYPE(z,n,type) PARAM_NAME(z,n,type)
  #define DEDUCE_PARAM_TYPE(z,in,Type)  typename boost::remove_const<typename boost::remove_reference<BOOST_PP_CAT(A,in)>::type >::type

#        ifndef BOOST_CMT_ACTOR_IMPL_SIZE
#           define BOOST_CMT_ACTOR_IMPL_SIZE 8
#        endif

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, BOOST_CMT_ACTOR_IMPL_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <boost/cmt/future_interface.hpp>
#       include BOOST_PP_ITERATE()

  #undef PARAM_NAME
  #undef PARAM_TYPE
  #undef PARAM_ARG
  #undef DEDUCE_PARAM_TYPE

  } } // namespace boost::cmt
  #endif // BOOST_CMT_ACTOR_INTERFACE_HPP

#else // BOOST_PP_IS_ITERATING

#define n BOOST_PP_ITERATION()
#define PARAM_NAMES          BOOST_PP_ENUM(n,PARAM_NAME,A) // name_N
#define PARAM_PLACE_HOLDERS  BOOST_PP_ENUM_TRAILING(n,PARAM_PLACE_HOLDER,A) // _(N+1)
#define PARAM_ARGS           BOOST_PP_ENUM(n,PARAM_ARG,A) // TYPE_N name_N
#define PARAM_TYPE_NAMES     BOOST_PP_ENUM(n,PARAM_TYPE_NAME,A) // typename TYPE_N
#define PARAM_TYPES          BOOST_PP_ENUM(n,PARAM_TYPE,A) // TYPE_N
#define DEDUCED_PARAM_TYPES  BOOST_PP_ENUM(n,DEDUCE_PARAM_TYPE,A) // TYPE_N

template<typename R, typename Class BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct future_member<R(Class::*)(PARAM_TYPES)const> : public detail::future_member_base {
  typedef typename boost::reflect::adapt_void<R>::result_type    result_type;
  typedef boost::unique_future<result_type>                      future_type;
  typedef future_member                                           self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                     fused_params;
  typedef boost::fusion::vector<DEDUCED_PARAM_TYPES>             deduced_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>       traits;
  static const bool                                              is_const = true;
  static const bool                                              is_signal = false;

  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type   signature;

  future_type operator()( PARAM_ARGS )const {
    return (*this)( boost::fusion::make_vector(PARAM_NAMES) );
  }
  future_type operator() ( const fused_params& fp )const {
    boost::packaged_task<result_type> pt( boost::bind( boost::ref(m_delegate), fp ) );
    future_type f = pt.get_future();
    this->m_strand->post( boost::move( pt ) );
    return f;
  }
  future_member& operator=( const future_member& d )  {
    m_delegate = d.m_delegate;
    return *this;
  }
  template<typename T>
  future_member& operator=( const T& d )  {
    m_delegate = boost::reflect::adapt_void<R,T>(d);
    return *this;
  }
  template<typename C, typename M>
  void set_delegate(  C* s, M m, boost::asio::strand* s ) {
    this->m_strand = s;
    m_delegate = boost::reflect::adapt_void<R, boost::function<R(const fused_params&)> >(
                    boost::fusion::make_fused_function_object( 
                                    boost::bind(m,s PARAM_PLACE_HOLDERS ) ));
  }
  private:
    boost::function<result_type(const fused_params&)> m_delegate; 
};

template<typename R, typename Class  BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct future_member<R(Class::*)(PARAM_TYPES)> : public detail::future_member_base {
  typedef typename boost::reflect::adapt_void<R>::result_type    result_type;
  typedef boost::unique_future<result_type>                      future_type;
  typedef future_member                                           self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                     fused_params;
  typedef boost::fusion::vector<DEDUCED_PARAM_TYPES>             deduced_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>       traits;
  typedef boost::function<result_type(const fused_params&)>      delegate_type;
  static const bool                                              is_const  = false;
  static const bool                                              is_signal = false;

  // boost::result_of
  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type signature;

  future_type operator()( PARAM_ARGS ) {
    return (*this)( boost::fusion::make_vector(PARAM_NAMES) );
  }
  future_type operator() ( const fused_params& fp ) {
    boost::packaged_task<result_type> pt( boost::bind( boost::ref(m_delegate), fp ) );
    future_type f = pt.get_future();
    this->m_strand->post( boost::move( pt ) );
    return f;
  }
  template<typename T>
  future_member& operator=( const T& d )  {
    m_delegate = boost::reflect::adapt_void<R,T>(d);
    return *this;
  }
  template<typename C, typename M>
  void set_delegate(  C* s, M m, boost::asio::strand* s ) {
    this->m_strand = s;
    m_delegate = boost::reflect::adapt_void<R, boost::function<R(const fused_params&)> >(
                      boost::fusion::make_fused_function_object( 
                                       boost::bind(m,s PARAM_PLACE_HOLDERS ) ));
  }
  private:
    delegate_type        m_delegate; 
};

#undef n
#undef PARAM_NAMES         
#undef PARAM_PLACE_HOLDERS
#undef PARAM_ARGS        
#undef PARAM_TYPE_NAMES 
#undef PARAM_TYPES     

#endif // BOOST_PP_IS_ITERATING
