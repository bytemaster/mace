#ifndef _BOOST_CMT_ASYNC_DELEGATE_HPP_
#define _BOOST_CMT_ASYNC_DELEGATE_HPP_
#include <boost/type_traits/function_traits.hpp>
#include <mace/cmt/detail/async_delegate_impl.hpp>
#include <boost/bind.hpp>

namespace mace { namespace cmt {

    /**
     *  @brief functor that invokes method in specified thread.
     *
     *  This functor turns a normal method into an asynchronous operation
     *  that will be run in the desired thread.  This is useful for interfacing with
     *  Boost.Signals or other libraries.
     */
    template<typename Signature>
    class async_delegate : 
            public mace::cmt::detail::async_delegate_impl<boost::function_traits<Signature>::arity, Signature >
    {
        public:
            typedef Signature signature;

            template<typename Functor>
            async_delegate( const Functor& slot, thread& s = thread::current() )
            :detail::async_delegate_impl<boost::function_traits<Signature>::arity, Signature >(slot,s){}
    };

} // namespace mace::cmt

#endif
