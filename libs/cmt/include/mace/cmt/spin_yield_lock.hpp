#ifndef _MACE_CMT_SPIN_YIELD_LOCK_HPP_
#define _MACE_CMT_SPIN_YIELD_LOCK_HPP_
#include <boost/atomic.hpp>
#include <boost/memory_order.hpp>
#include <boost/thread/thread_time.hpp>

namespace mace { namespace cmt {
  /// cmt::thread::current().yield()
  void yield();

  /**
   *  @class spin_yield_lock
   *  @brief modified spin-lock that yields on failure, but becomes a 'spin lock' 
   *         if there are no other tasks to yield to.
   *
   *  This kind of lock is lighter weight than a full mutex, but potentially slower
   *  than a staight spin_lock.
   *
   *  This spin_yield_lock does not block the current thread, but instead attempts to use
   *  an atomic operation to aquire the lock.  If unsuccessful, then it yields to
   *  other tasks before trying again. If there are no other tasks then yield is
   *  a no-op and spin_yield_lock becomes a spin-lock.  
   */
  class spin_yield_lock {
    public:
      spin_yield_lock():m_state(unlocked){}

      inline bool try_lock() {
        return m_state.exchange(locked, boost::memory_order_acquire)!=locked;
      }

      template<typename DurationType>
      bool timed_lock( const DurationType& rel_time ) {
        return timed_lock( boost::get_system_time() + rel_time );
      }

      bool timed_lock( const boost::system_time& abs_time) {
        while( abs_time > boost::get_system_time() ) {
           if( try_lock() ) 
              return true;
           yield(); 
        }
        return false;
      }
      void lock() {
        while( m_state.exchange(locked, boost::memory_order_acquire)==locked) {
           yield(); 
        }
      }
      void unlock() {
        m_state.store(unlocked, boost::memory_order_release);
      }
      
    private:
      enum lock_state {locked,unlocked};
      boost::atomic<lock_state> m_state;
  };

} } // namespace mace::cmt

#endif // _MACE_CMT_SPIN_YIELD_LOCK_HPP_
