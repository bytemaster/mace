#ifndef BOOST_PP_IS_ITERATING
    #ifndef MACE_CMT_BIND_HPP_
    #define MACE_CMT_BIND_HPP_
    #include <boost/preprocessor/repetition.hpp>
    #include <boost/preprocessor/seq/for_each.hpp>
    #include <boost/fusion/container/vector.hpp>

    namespace mace { namespace cmt {
      template<typename T>
      T copy( const T& t ) { return t; }

      namespace detail {
        template<typename R, typename F, typename Seq> 
        struct bind : boost::noncopyable {
          bind( bind&& b ){}
          void operator()(){}
          bind& operator=(bind&& c){ return *this; }
        };
      } // detail

    } } // mace::cmt
      

  #define PARAM_NAME(z,n,type)        BOOST_PP_CAT(a,n)
  #define MEMBER_NAME(z,n,type)       BOOST_PP_CAT(m,n)
  #define PARAM_TYPE_NAME(z,n,type)   BOOST_PP_CAT(typename A,n)
  #define PARAM_TYPE(z,n,type)        BOOST_PP_CAT(A,n)
  #define PARAM_ARG(z,n,type)         PARAM_TYPE(z,n,type)&& PARAM_NAME(z,n,type)
  #define INIT_MEMBER(z,n,type)       MEMBER_NAME(z,n,type)( std::forward<PARAM_TYPE(z,n,type)>( PARAM_NAME(z,n,type) ) )
  #define INIT_MOVE(z,n,type)         MEMBER_NAME(z,n,type)( std::move( c.MEMBER_NAME(z,n,type) ) )
  #define ASSIGN_MEMBER(z,n,type)     MEMBER_NAME(z,n,type) = std::move( c.MEMBER_NAME(z,n,type) );
  #define MOVE_MEMBER(z,n,type)       std::move(MEMBER_NAME(z,n,type))
  #define FORWARD_PARAM(z,n,type)     std::forward<PARAM_TYPE(z,n,type)>(PARAM_NAME(z,n,type))
  #define PARAM_MEMBER(z,n,type)      PARAM_TYPE(z,n,type)   MEMBER_NAME(z,n,type);


  #ifndef MACE_CMT_BIND_IMPL_SIZE
  #define MACE_CMT_BIND_IMPL_SIZE 8
  #endif

  #include <boost/preprocessor/iteration/iterate.hpp>
  #define BOOST_PP_ITERATION_LIMITS (1, MACE_CMT_BIND_IMPL_SIZE  )
  #define BOOST_PP_FILENAME_1 <mace/cmt/bind.hpp>
  #include BOOST_PP_ITERATE()

  #undef PARAM_NAME
  #undef MEMBER_NAME
  #undef PARAM_TYPE_NAME
  #undef PARAM_TYPE
  #undef PARAM_ARG
  #undef INIT_MEMBER
  #undef INIT_MOVE
  #undef ASSIGN_MEMBER
  #undef MOVE_MEMBER
  #undef FORWARD_PARAM
  #undef PARAM_MEMBER


    #endif  // MACE_CMT_BIND_HPP_

#else // BOOST_PP_IS_ITERATING

#define n BOOST_PP_ITERATION()
  
#define PARAM_TYPE_NAMES     BOOST_PP_ENUM(n,PARAM_TYPE_NAME,A) // typename AN
#define PARAM_TYPES          BOOST_PP_ENUM(n,PARAM_TYPE,A)      // AN
#define PARAM_ARGS           BOOST_PP_ENUM(n,PARAM_ARG,A)       // AN&& aN
#define PARAM_NAMES          BOOST_PP_ENUM(n,PARAM_NAME,A)      // aN 
#define INIT_MEMBERS         BOOST_PP_ENUM(n,INIT_MEMBER,A)     // mN( std::forward<AN>(aN) )
#define INIT_MOVES           BOOST_PP_ENUM(n,INIT_MOVE,A)       // mN( std::move(c.mN) )
#define ASSIGN_MEMBERS       BOOST_PP_REPEAT(n,ASSIGN_MEMBER,A) // member_N = std::move( c.member_N );
#define MOVE_MEMBERS         BOOST_PP_ENUM(n,MOVE_MEMBER,A)     // std::move(mN)
#define FORWARD_PARAMS       BOOST_PP_ENUM(n,FORWARD_PARAM,A)   // std::forward<AN>(aN)
#define PARAM_MEMBERS        BOOST_PP_REPEAT(n,PARAM_MEMBER,A)  // TYPE_N member_N;

namespace mace { namespace cmt { namespace detail {

template<typename R, typename F, PARAM_TYPE_NAMES>
struct bind<R,F,boost::fusion::vector<PARAM_TYPES> > : boost::noncopyable {
  typedef R result_type;

  bind( bind&& c )
  :func( std::move(c.func) ), INIT_MOVES {}

  bind( F&& f, PARAM_ARGS )
  :func( std::move(f) ), INIT_MEMBERS {} 

  bind& operator=( bind&& c ) {
    func = std::move(c.func);
    ASSIGN_MEMBERS
    return *this;
  }

  result_type operator()() { return func( MOVE_MEMBERS ); }

  private:
    F func;
    PARAM_MEMBERS
};

}  // detail

template<typename F, PARAM_TYPE_NAMES>
auto bind( F&& f, PARAM_ARGS ) -> detail::bind< decltype(f(FORWARD_PARAMS)), F, boost::fusion::vector<PARAM_TYPES> > {
  typedef decltype(f(FORWARD_PARAMS)) R;
  return detail::bind<R,F,boost::fusion::vector<PARAM_TYPES> >( std::forward<F>(f), FORWARD_PARAMS );
}

} } // mace::cmt

#undef PARAM_TYPE_NAMES
#undef PARAM_TYPES
#undef PARAM_ARGS
#undef PARAM_NAMES
#undef INIT_MEMBERS
#undef INIT_MOVES
#undef ASSIGN_MEMBERS 
#undef MOVE_MEMBERS
#undef FORWARD_PARAMS
#undef PARAM_MEMBERS


#undef n

#endif // BOOST_PP_IS_ITERATING
