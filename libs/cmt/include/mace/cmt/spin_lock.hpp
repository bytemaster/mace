#ifndef _MACE_CMT_SPINLOCK_HPP_
#define _MACE_CMT_SPINLOCK_HPP_
#include <boost/atomic.hpp>
#include <boost/memory_order.hpp>
#include <boost/thread/thread_time.hpp>

namespace mace { namespace cmt {

  /**
   *  @class spin_lock
   *  @brief A simple spin lock.
   *
   *  A spin lock will busy wait attempting to aquire the lock.  Only
   *  use the spin lock when all access to the shared state is short
   *  and well defined. (A couple math operations at most) 
   *
   *  If you don't need 'wait-free' access and are accessing a shared 
   *  resource to perform more than a trival operation you can try
   *  the @ref spin_yield_lock which is a busy wait unless there are
   *  other tasks to run in the current thread.  
   *
   *  For all other cases you should use the cmt::mutex and never use
   *  the boost::mutex.  The cmt::mutex will ensure that all threads
   *  will yield and not return until timeout or the lock is aquired. 
   *
   */
  class spin_lock {
    public:
      spin_lock():m_state(unlocked){}

      inline bool try_lock() {
        return m_state.exchange(locked, boost::memory_order_acquire)!=locked;
      }

      template<typename DurationType>
      inline bool timed_lock( const DurationType& rel_time ) {
        return timed_lock( boost::get_system_time() + rel_time );
      }

      inline bool timed_lock( const boost::system_time& abs_time) {
        while( abs_time > boost::get_system_time() ) {
           if( try_lock() )
           return true;
        }
        return false;
      }
      inline void lock() {
        while( m_state.exchange(locked, boost::memory_order_acquire)==locked) ;
      }
      inline void unlock() {
        m_state.store(unlocked, boost::memory_order_release);
      }
      
    private:
      enum lock_state {locked,unlocked};
      boost::atomic<lock_state> m_state;
  };

} } // namespace mace::cmt

#endif // _MACE_CMT_SPINLOCK_HPP_
