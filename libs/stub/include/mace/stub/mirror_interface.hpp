#ifndef BOOST_PP_IS_ITERATING
  #ifndef BOOST_STUB_MIRROR_INTERFACE_HPP
  #define BOOST_STUB_MIRROR_INTERFACE_HPP
  #include <boost/bind.hpp>
  #include <boost/preprocessor/repetition.hpp>
  #include <boost/preprocessor/seq/for_each.hpp>

  #include <boost/fusion/container/vector.hpp>
  #include <boost/fusion/container/generation/make_vector.hpp>
  #include <boost/fusion/functional/generation/make_fused_function_object.hpp>
  #include <boost/fusion/functional/generation/make_unfused.hpp>
  #include <mace/stub/void.hpp>
  #include <mace/stub/vtable.hpp>
  #include <boost/type_traits/function_traits.hpp>
  #include <boost/type_traits/remove_pointer.hpp>
  #include <boost/type_traits/remove_reference.hpp>
  #include <boost/function_types/result_type.hpp>
  #include <boost/function.hpp>

  namespace mace { namespace stub {
    /**
     *  @brief Specialized to mirror the member 
     *         variable/method pointed to by MemberPtr
     */
    template<typename MemberPtr>
    struct mirror_member;

    namespace detail {
        namespace mirror_interface {
          #ifndef DOXYGEN
          /**
           *  Assigns VTableType's functors with values from T
           */
          template<typename T, typename VTableType>
          class set_visitor {
            public:
              set_visitor( VTableType& vt, T& self )
              :m_self(self),vtbl(vt){}

              template<typename MemberPtr, MemberPtr m>
              void operator()( const char* name )const {
                typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
                typedef typename boost::remove_reference<member_ref>::type member;
                (vtbl.*m).set_delegate( &m_self, member::template get_member_ptr<T>() );
              }
            private:
              T&          m_self;
              VTableType& vtbl;
          };
          
          /**
           *  This specialization is for the case of one stub::ptr<T> pointing to
           *  another stub::ptr<Other> 
           */
          template<typename Interface, typename Delegate, typename VTableType>
          class set_visitor<vtable<Interface,Delegate>, VTableType> {
            public:
              typedef mace::stub::vtable<Interface,Delegate> T;

              set_visitor( VTableType& vt, T& self )
              :m_self(self),vtbl(vt){}

              template<typename MemberPtr, MemberPtr m> 
              void operator()( const char* name )const {
                typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
                typedef typename boost::remove_reference<member_ref>::type member;
                (vtbl.*m) = boost::bind( boost::ref( m_self.* member::template get_member_ptr<T>()), _1 );
              }
            private:
              T&    m_self;
              VTableType& vtbl;
          };
         
          #endif // DOXYGEN 
        }
    }
    
    /**
     *  @brief Interface Delegate that mirrors the 
     *         reflected interface without any transformation.
     *  
     *  To specialize how a particular member is mirrored define
     *  the partial specialization of mirror_member for your type.
     *
     *  @code
     *  template<typename Type, typename Class>
     *  mirror_member<Type(Class::*)> 
     *  @endcode
     */
    struct mirror_interface 
    {
      /**
       * @brief Implements the InterfaceDelegate meta-function to 
       *      determine what type to create to mirror MemberPointer 
       *      in mace::stub::vtable used by mace::stub::any_ptr
       */
      template<typename MemberPointer>
      struct calculate_type {
        typedef mirror_member<MemberPointer>  type; 
      };

      template<typename T, typename VTableType>
      static void set_vtable( VTableType& vtable, T& value ) {
        vtable_reflector<typename VTableType::interface_type,mirror_interface>::visit( detail::mirror_interface::set_visitor<T,VTableType>(vtable,value) );
      }
      template<typename T, typename VTableType>
      static void set_vtable( VTableType& vtable, const T& value ) {
        vtable_reflector<typename VTableType::interface_type,mirror_interface>::visit( detail::mirror_interface::set_visitor<T,VTableType>(vtable,value) );
      }
    };



  #define PARAM_NAME(z,n,type)         BOOST_PP_CAT(a,n)
  #define PARAM_PLACE_HOLDER(z,n,type) BOOST_PP_CAT(_,BOOST_PP_ADD(n,1) )
  #define PARAM_TYPE_NAME(z,n,type)   BOOST_PP_CAT(typename A,n)
  #define PARAM_TYPE(z,n,type)   BOOST_PP_CAT(A,n)
  #define PARAM_ARG(z,n,type)     PARAM_TYPE(z,n,type) PARAM_NAME(z,n,type)
  #define DEDUCE_PARAM_TYPE(z,in,Type)  typename boost::remove_const<typename boost::remove_reference<BOOST_PP_CAT(A,in)>::type >::type

#        ifndef BOOST_STUB_MIRROR_IMPL_SIZE
#           define BOOST_STUB_MIRROR_IMPL_SIZE 8
#        endif

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, BOOST_STUB_MIRROR_IMPL_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <mace/stub/mirror_interface.hpp>
#       include BOOST_PP_ITERATE()

  #undef PARAM_NAME
  #undef PARAM_TYPE
  #undef PARAM_ARG
  #undef DEDUCE_PARAM_TYPE

  } } // namespace mace::stub
  #endif // BOOST_STUB_MIRROR_INTERFACE_HPP

#else // BOOST_PP_IS_ITERATING

#define n BOOST_PP_ITERATION()
#define PARAM_NAMES          BOOST_PP_ENUM(n,PARAM_NAME,A) // name_N
#define PARAM_PLACE_HOLDERS  BOOST_PP_ENUM_TRAILING(n,PARAM_PLACE_HOLDER,A) // _(N+1)
#define PARAM_ARGS           BOOST_PP_ENUM(n,PARAM_ARG,A) // TYPE_N name_N
#define PARAM_TYPE_NAMES     BOOST_PP_ENUM(n,PARAM_TYPE_NAME,A) // typename TYPE_N
#define PARAM_TYPES          BOOST_PP_ENUM(n,PARAM_TYPE,A) // TYPE_N
#define DEDUCED_PARAM_TYPES  BOOST_PP_ENUM(n,DEDUCE_PARAM_TYPE,A) // TYPE_N

template<typename R, typename Class BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct mirror_member<R(Class::*)(PARAM_TYPES)const> {
  // boost::result_of
  typedef typename adapt_void<R>::result_type                    result_type;
  typedef mirror_member                                          self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                     fused_params;
  typedef boost::fusion::vector<DEDUCED_PARAM_TYPES>             deduced_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>       traits;
  static const bool                                              is_const = true;
  static const bool                                              is_signal = false;

  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type   signature;

  result_type operator()( PARAM_ARGS )const {
    return m_delegate( boost::fusion::make_vector(PARAM_NAMES) );
  }
  result_type operator() ( const fused_params& fp )const {
    return m_delegate( fp );
  }
  mirror_member& operator=( const mirror_member& d )  {
    m_delegate = d.m_delegate;
    return *this;
  }
  template<typename T>
  mirror_member& operator=( const T& d )  {
    m_delegate = adapt_void<R,T>(d);
    return *this;
  }
  template<typename C, typename M>
  void set_delegate(  C* s, M m ) {
    m_delegate = adapt_void<R, boost::function<R(const fused_params&)> >(
                    boost::fusion::make_fused_function_object( 
                                    boost::bind(m,s PARAM_PLACE_HOLDERS ) ));
  }
  private:
    boost::function<result_type(const fused_params&)> m_delegate; 
};

template<typename R, typename Class  BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct mirror_member<R(Class::*)(PARAM_TYPES)> 
{
  typedef typename adapt_void<R>::result_type                result_type;
                                                                                       
  typedef mirror_member                                      self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                 fused_params;
  typedef boost::fusion::vector<DEDUCED_PARAM_TYPES>         deduced_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>   traits;
  typedef boost::function<result_type(const fused_params&)>  delegate_type;
  static const bool                                          is_const  = false;
  static const bool                                          is_signal = false;

  // boost::result_of
  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type signature;

  result_type operator()( PARAM_ARGS ) {
    return m_delegate( boost::fusion::make_vector(PARAM_NAMES) );
  }
  result_type operator() ( const fused_params& fp ) {
    return m_delegate( fp );
  }
  template<typename T>
  mirror_member& operator=( const T& d )  {
    m_delegate = adapt_void<R,T>(d);
    return *this;
  }
  template<typename C, typename M>
  void set_delegate(  C* s, M m ) {
    m_delegate = adapt_void<R, boost::function<R(const fused_params&)> >(
                      boost::fusion::make_fused_function_object( 
                                       boost::bind(m,s PARAM_PLACE_HOLDERS ) ));
  }
  private:
    delegate_type m_delegate; 
};

#undef n
#undef PARAM_NAMES         
#undef PARAM_PLACE_HOLDERS
#undef PARAM_ARGS        
#undef PARAM_TYPE_NAMES 
#undef PARAM_TYPES     

#endif // BOOST_PP_IS_ITERATING
