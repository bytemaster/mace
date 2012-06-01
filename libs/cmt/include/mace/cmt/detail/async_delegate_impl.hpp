/**
    @file async_delegate_impl.hpp
    This file is designed to be used with BOOST_PP_ITERATE
*/

#ifndef BOOST_PP_IS_ITERATING

#   ifndef CMT_DELEGATE_IMPL_HPP_INCLUDED
#       define CMT_DELEGATE_IMPL_HPP_INCLUDED

#       include <boost/future/future.hpp>
#       include <boost/preprocessor/repetition.hpp>
#       include <boost/preprocessor/arithmetic/sub.hpp>
#       include <boost/preprocessor/punctuation/comma_if.hpp>

#        ifndef MAX_CMT_ASYNC_DELEGATE_PARAM
#           define MAX_CMT_ASYNC_DELEGATE_PARAM 8
#        endif


namespace mace { namespace cmt {
namespace detail {
    
/// @cond INTERNAL_DEV
    /**
        This class is specialized for delegates with up to CMT_DELEGATE_IMPL_SIZE parameters.
    */
    template<int N, typename Signature>
    struct async_delegate_impl{};
/// @endcond 
        #define PARAM_TYPE(z,n,type)    BOOST_PP_CAT(BOOST_PP_CAT(typename traits::arg,BOOST_PP_ADD(n,1)),_type)
        #define PARAM_NAME(z,n,type)    BOOST_PP_CAT(a,n)
        #define PARAM_ARG(z,n,type)     PARAM_TYPE(z,n,type) PARAM_NAME(z,n,type)

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, MAX_CMT_ASYNC_DELEGATE_PARAM )
#       define BOOST_PP_FILENAME_1 <boost/cmt/detail/async_delegate_impl.hpp>
#       include BOOST_PP_ITERATE()

        #undef PARAM_TYPE
        #undef PARAM_NAME
        #undef PARAM_ARG
} // detail 
} // cmt 
} // mace

#   endif // CMT_DELEGATE_IMPL_HPP_INCLUDED

#else // BOOST_PP_IS_ITERATING

#   define n BOOST_PP_ITERATION()
#define PARAM_TYPES     BOOST_PP_ENUM(n,PARAM_TYPE,A) // TYPE_N
#define PARAM_NAMES     BOOST_PP_ENUM(n,PARAM_NAME,A) // name_N
#define PARAM_ARGS      BOOST_PP_ENUM(n,PARAM_ARG,A) // TYPE_N name_N

template<typename Signature>
class async_delegate_impl<n, Signature > {
  public:
    typedef boost::function_traits<Signature>  traits;
    future<typename traits::result_type> operator()(PARAM_ARGS) { 
      return m_thr.async<typename traits::result_type>( boost::bind(m_del,PARAM_NAMES) );
    }

    template<typename Functor>
    async_delegate_impl( Functor f, thread& t )
    :m_del(f),m_thr(t) { }

  private:
    boost::function<Signature> m_del;
    thread&                    m_thr;
};


#undef n
#undef PARAM_TYPES
#undef PARAM_NAMES
#undef PARAM_ARGS

#endif // BOOST_PP_IS_ITERATING

