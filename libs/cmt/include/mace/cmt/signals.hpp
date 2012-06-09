#ifndef _MACE_CMT_SIGNALS_HPP
#define _MACE_CMT_SIGNALS_HPP
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <mace/cmt/task.hpp>
#include <mace/cmt/thread.hpp>

namespace mace { namespace cmt {
   template<typename T>
   inline T wait( boost::signal<void(T)>& sig, const microseconds& timeout_us=microseconds::max() ) {
           typename promise<T>::ptr p(new promise<T>());
           void (promise<T>::*m)( const T& ) = &promise<T>::set_value;
           boost::signals::scoped_connection c = sig.connect( boost::bind(m,p,_1) );
           return p->wait( timeout_us ); 
   }

   inline void wait( boost::signal<void()>& sig, const microseconds& timeout_us=microseconds::max() ) {
           promise<void_t>::ptr p(new promise<void_t>());
           void (promise<void_t>::*m)( const void_t& ) = &promise<void_t>::set_value;
           boost::signals::scoped_connection c = sig.connect( boost::bind(m,p,void_t()) );
           p->wait( timeout_us ); 
   }
} } // mace::cmt

#endif
