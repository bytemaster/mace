#ifndef BOOST_PP_IS_ITERATING
  #ifndef MACE_CMT_ACTOR_INTERFACE_HPP
  #define MACE_CMT_ACTOR_INTERFACE_HPP
  #include <boost/bind.hpp>
  #include <mace/cmt/future.hpp>
  #include <boost/preprocessor/repetition.hpp>
  #include <boost/preprocessor/seq/for_each.hpp>

  #include <boost/fusion/container/vector.hpp>
  #include <boost/fusion/container/generation/make_vector.hpp>
  #include <boost/fusion/functional/generation/make_fused_function_object.hpp>
  #include <boost/fusion/functional/generation/make_unfused.hpp>
  #include <boost/typeof/typeof.hpp>
  #include <mace/stub/void.hpp>
  #include <boost/signals.hpp>
  #include <mace/stub/vtable.hpp>

  namespace mace { namespace cmt {
    /**
     *  @brief Specialized to mirror the member 
     *         variable/method pointed to by MemberPtr
     */
    template<typename MemberPtr>
    struct actor_member;

    namespace detail {
        struct actor_base {
          actor_base( mace::cmt::thread* t ):m_thread(t){}
          mace::cmt::thread* m_thread;
          /** Implemented as a friend so as not to polute actor interface */
          friend void                set_thread( actor_base& a, mace::cmt::thread* t ) { a.m_thread = t; }
          friend mace::cmt::thread* get_thread( actor_base& a) { return a.m_thread; }
        };

        class actor_member_base {
          public:
              void set_actor( detail::actor_base* ab ){m_actor=ab;}
          protected:
              detail::actor_base*  m_actor;
        };

        namespace actor_interface {
          #ifndef DOXYGEN
          /**
           *  Assigns VTableType's functors with values from T
           */
          template<typename T, typename VTableType>
          class set_visitor {
            public:
              set_visitor( VTableType& vt, T& self, actor_base* ab )
              :m_actor(ab),m_self(self),vtbl(vt){}

              template<typename MemberPtr, MemberPtr m>
              void operator()( const char* name )const {
                typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
                typedef typename boost::remove_reference<member_ref>::type member;
                (vtbl.*m).set_delegate( &m_self, member::template get_member_ptr<T>(),m_actor );
              }
            private:
              template<typename Member>
              struct assign {
                assign( T& _v, Member& _m, actor_base* ab ):v(_v),m(_m),a(ab){}

                template<typename MemberPtr>
                void operator=( const MemberPtr& p ) {
                  m.set_delegate( &v, p, a );
                }
                private:
                  actor_base*   a;
                  T&            v;
                  Member&       m;
              };
              actor_base*         m_actor;
              T&                  m_self;
              VTableType&         vtbl;
          };
          template<typename Interface, typename Delegate, typename VTableType>
          class set_visitor<mace::stub::vtable<Interface,Delegate>, VTableType> {
            public:
              typedef mace::stub::vtable<Interface,Delegate> T;

              set_visitor( VTableType& vt, T& self, actor_base* ab )
              :m_actor(ab), m_self(self),vtbl(vt){}

              template<typename M, M m>
              void operator()( const char* name )const {
                assign<M> a(m_self,vtbl.*m, m_actor);
                M::template get_member_ptr<T>( a );
              }
            private:
              template<typename Member>
              struct assign {
                assign( T& _v, Member& _m, actor_base* ab ):v(_v),m(_m),a(ab){}

                template<typename MemberPtr>
                void operator=( MemberPtr p ) {
                    m.set_actor(a);
                    m = boost::bind(boost::ref(v.*p), _1 );
                }
                private:
                 actor_base* a;
                 T&          v;
                 Member&     m;
              };
              actor_base*   m_actor;
              T&            m_self;
              VTableType&   vtbl;
          };
         
          #endif 
        }
    } // namespace detail
    
    /**
     *  @brief Interface Delegate that mirrors the 
     *         reflected interface without any transformation.
     *  
     *  To specialize how a particular member is mirrored define
     *  the partial specialization of actor_member for your type.
     *
     *  @code
     *  template<typename Type, typename Class>
     *  actor_member<Type(Class::*)> 
     *  @endcode
     */
    struct actor_interface 
    {
      /**
       * @brief Implements the InterfaceDelegate meta-function to 
       *      determine what type to create to mirror MemberPointer 
       *      in mace::stub::vtable used by mace::reflect::any_ptr
       */
      template<typename MemberPointer>
      struct calculate_type {
        typedef actor_member<MemberPointer>  type; 
      };

      template<typename T, typename VTableType>
      static void set_vtable( VTableType& vtable, T& value, detail::actor_base* ab ) {
        mace::stub::vtable_reflector<typename VTableType::interface_type,typename VTableType::delegate_type>::visit( 
                    detail::actor_interface::set_visitor<T,VTableType>(vtable,value,ab) );
      }
      template<typename T, typename VTableType>
      static void set_vtable( VTableType& vtable, const T& value, detail::actor_base* ab ) {
        mace::stub::vtable_reflector<typename VTableType::interface_type, typename VTableType::delegate_type>::visit( 
                    detail::actor_interface::set_visitor<T,VTableType>(vtable,value,ab) );
      }
    };

    #ifndef DOXYGEN

    /**
     *  Blocks a signal if it is currently unblocked and 
     *  unblocks it when it goes out of scope if it was blocked
     *  when constructed. 
     */
    struct scoped_block_signal {
      scoped_block_signal( boost::signals::connection& _c )
      :c(_c),unblock(false){ 
        if( c != boost::signals::connection() && !c.blocked() )  {
          unblock = true;
          c.block();
        }
      }
      ~scoped_block_signal() { 
        if( unblock && c != boost::signals::connection() ) 
            c.unblock(); 
      }
      private:
        bool                        unblock;
        boost::signals::connection& c; 
    };
    #endif


  #define PARAM_NAME(z,n,type)         BOOST_PP_CAT(a,n)
  #define PARAM_PLACE_HOLDER(z,n,type) BOOST_PP_CAT(_,BOOST_PP_ADD(n,1) )
  #define PARAM_TYPE_NAME(z,n,type)   BOOST_PP_CAT(typename A,n)
  #define PARAM_TYPE(z,n,type)   BOOST_PP_CAT(A,n)
  #define PARAM_ARG(z,n,type)     PARAM_TYPE(z,n,type) PARAM_NAME(z,n,type)
  #define DEDUCE_PARAM_TYPE(z,in,Type)  typename boost::remove_const<typename boost::remove_reference<BOOST_PP_CAT(A,in)>::type >::type

#        ifndef MACE_CMT_ACTOR_IMPL_SIZE
#           define MACE_CMT_ACTOR_IMPL_SIZE 8
#        endif

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, MACE_CMT_ACTOR_IMPL_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <mace/cmt/actor_interface.hpp>
#       include BOOST_PP_ITERATE()

  #undef PARAM_NAME
  #undef PARAM_TYPE
  #undef PARAM_ARG
  #undef DEDUCE_PARAM_TYPE

  } } // namespace mace::cmt
  #endif // MACE_CMT_ACTOR_INTERFACE_HPP

#else // BOOST_PP_IS_ITERATING

#define n BOOST_PP_ITERATION()
#define PARAM_NAMES          BOOST_PP_ENUM(n,PARAM_NAME,A) // name_N
#define PARAM_PLACE_HOLDERS  BOOST_PP_ENUM_TRAILING(n,PARAM_PLACE_HOLDER,A) // _(N+1)
#define PARAM_ARGS           BOOST_PP_ENUM(n,PARAM_ARG,A) // TYPE_N name_N
#define PARAM_TYPE_NAMES     BOOST_PP_ENUM(n,PARAM_TYPE_NAME,A) // typename TYPE_N
#define PARAM_TYPES          BOOST_PP_ENUM(n,PARAM_TYPE,A) // TYPE_N
#define DEDUCED_PARAM_TYPES  BOOST_PP_ENUM(n,DEDUCE_PARAM_TYPE,A) // TYPE_N

template<typename R, typename Class BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct actor_member<R(Class::*)(PARAM_TYPES)const> : public detail::actor_member_base
{
  // boost::result_of
  typedef typename mace::stub::adapt_void<R>::result_type                    result_type;
  typedef mace::cmt::future<result_type>                             future_type;
  typedef actor_member                                           self_type;
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
    if( this->m_actor && &mace::cmt::thread::current() != m_actor->m_thread ) {
      return this->m_actor->m_thread->async<R>( boost::bind(boost::ref(m_delegate),fp) );
    }
    return m_delegate( fp );
  }
  actor_member& operator=( const actor_member& d )  {
    m_delegate = d.m_delegate;
    return *this;
  }
  template<typename T>
  actor_member& operator=( const T& d )  {
    m_delegate = mace::stub::adapt_void<R,T>(d);
    return *this;
  }
  template<typename C, typename M>
  void set_delegate(  C* s, M m, detail::actor_base* ab ) {
    this->m_actor = ab;
    m_delegate = mace::stub::adapt_void<R, boost::function<R(const fused_params&)> >(
                    boost::fusion::make_fused_function_object( 
                                    boost::bind(m,s PARAM_PLACE_HOLDERS ) ));
  }
  private:
    boost::function<result_type(const fused_params&)> m_delegate; 
};

template<typename R, typename Class  BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct actor_member<R(Class::*)(PARAM_TYPES)> : public detail::actor_member_base
{
  typedef typename mace::stub::adapt_void<R>::result_type                result_type;
  typedef mace::cmt::future<result_type>                             future_type;
  typedef actor_member                                      self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                 fused_params;
  typedef boost::fusion::vector<DEDUCED_PARAM_TYPES>         deduced_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>   traits;
  typedef boost::function<result_type(const fused_params&)>  delegate_type;
  static const bool                                          is_const  = false;
  static const bool                                          is_signal = false;

  // boost::result_of
  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type signature;

  future_type operator()( PARAM_ARGS ) {
    return (*this)( boost::fusion::make_vector(PARAM_NAMES) );
  }
  future_type operator() ( const fused_params& fp ) {
    if( this->m_actor && &mace::cmt::thread::current() != this->m_actor->m_thread ) {
      return this->m_actor->m_thread->async<R>( boost::bind(m_delegate,fp) );
    }
    return m_delegate( fp );
  }
  template<typename T>
  actor_member& operator=( const T& d )  {
    m_delegate = mace::stub::adapt_void<R,T>(d);
    return *this;
  }
  template<typename C, typename M>
  void set_delegate(  C* s, M m, detail::actor_base* ab ) {
    this->m_actor = ab;
    m_delegate = mace::stub::adapt_void<R, boost::function<R(const fused_params&)> >(
                      boost::fusion::make_fused_function_object( 
                                       boost::bind(m,s PARAM_PLACE_HOLDERS ) ));
  }
  private:
    delegate_type m_delegate; 
};


template<typename R, typename Class  BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct actor_member< boost::signal<R(PARAM_TYPES)> (Class::*) >  : public detail::actor_member_base
{
  typedef typename mace::stub::adapt_void<R>::result_type                result_type;
  typedef mace::cmt::future<result_type>                             future_type;
  typedef actor_member                                      self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                 fused_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>   traits;
  typedef boost::signal<R(PARAM_TYPES)>                      signal_type;
  static const bool                                          is_const = false;
  static const bool                                          is_signal = true;

  // boost::result_of
  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type   signature;

  future_type operator()( PARAM_ARGS ) {
    return (*this)( boost::fusion::make_vector(PARAM_NAMES) );
  }

  future_type operator() ( const fused_params& fp ) {
    if( this->m_actor && &mace::cmt::thread::current() != this->m_actor->m_thread ) {
      return this->m_actor->m_thread->async<R>( boost::bind(boost::ref(*this),fp) );
    }
    scoped_block_signal block_reverse(m_reverse_con);
    if( int(m_signal.num_slots()) - 1 > 0 )  // do not count our reverse connection
        boost::fusion::make_fused_function_object( boost::ref(m_signal) )(fp);
    return m_delegate( fp );
  }

  // emits locally, but does not forward to delegate
  future_type emit( const fused_params& fp ) {
    scoped_block_signal block_reverse(m_reverse_con);
    return mace::stub::adapt_void<R,boost::function<R(const fused_params&)> >(
            boost::fusion::make_fused_function_object( boost::ref(m_signal) ) )(fp);
  }

  template<typename T>
  actor_member& operator=( const T& d )  {
    m_delegate = mace::stub::adapt_void<R,T>(d);
    return *this;
  }

  template<typename Functor>
  boost::signals::connection connect( const Functor& f ) {
    boost::signals::connection c = m_signal.connect(mace::stub::adapt_void<R,Functor>(f) );
    if( m_connect_delegate )
        m_connect_delegate(m_signal.num_slots());
    return c;
  }
  void disconnect( const boost::signals::connection& c ) {
    c.disconnect();
    if( m_connect_delegate )
        m_connect_delegate(m_signal.num_slots());
  }

  void set_connect_delegate( const boost::function<void(int)>& f ) {
    m_connect_delegate = f;
  }

  // sets the delegate that will be called when the signal is 'emited'
  template<typename C, typename M>
  void set_delegate(  C* s, M m, detail::actor_base* ab ) {
    this->m_actor = ab;
    m_signal.disconnect_all_slots();
    m_reverse_con = 
        (s->*m).connect( boost::fusion::make_unfused( 
                            boost::bind( &actor_member::emit, this, _1) )  );

    m_delegate = emit_or_throw( s->*m );
  }

  ~actor_member() {
    m_signal.disconnect_all_slots();
    if( m_reverse_con != boost::signals::connection() )
        m_reverse_con.disconnect();
  }

  private:

    struct emit_or_throw {
        struct no_connected_slots : public std::exception, public boost::exception {
          const char* what()const throw() { return "no connected slots"; }
        };
        emit_or_throw( signal_type& s ):sig(s){}
        result_type operator()( const fused_params& p ) {
          if( int(sig.num_slots()) -1 > 0 ) { // do not count our reverse connection
            return mace::stub::adapt_void<R, boost::function<R(const fused_params&)> >(
                      boost::fusion::make_fused_function_object(boost::ref(sig)))(p);
          }
          BOOST_THROW_EXCEPTION( no_connected_slots() );
        }
        signal_type&            sig;
    };

    boost::function<void(int)>                         m_connect_delegate;
    boost::function<result_type(const fused_params&)>  m_delegate; 
    boost::signals::connection                         m_reverse_con;
    boost::signal<R(PARAM_TYPES)>                      m_signal;
};

#undef n
#undef PARAM_NAMES         
#undef PARAM_PLACE_HOLDERS
#undef PARAM_ARGS        
#undef PARAM_TYPE_NAMES 
#undef PARAM_TYPES     

#endif // BOOST_PP_IS_ITERATING
