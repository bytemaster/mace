#ifndef _MACE_CMT_ABSTRACT_THREAD_HPP_
#define _MACE_CMT_ABSTRACT_THREAD_HPP_
#include <mace/cmt/retainable.hpp>

namespace mace { namespace cmt {

  class abstract_thread : public retainable {
   public:
     typedef retainable_ptr<abstract_thread> ptr;

     virtual ~abstract_thread(){};
   protected:
     friend class promise_base;
     virtual void wait( const promise_base::ptr& p, const boost::chrono::microseconds& timeout_us ) = 0;
     virtual void wait( const promise_base::ptr& p, const system_clock::time_point& timeout ) = 0;
     virtual void notify( const promise_base::ptr& p ) = 0;
  };

} } // mace::cmt

#endif
