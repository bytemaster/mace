#ifndef _MACE_CMT_MUTEX_HPP_
#define _MACE_CMT_MUTEX_HPP_
#include <boost/atomic.hpp>
#include <boost/memory_order.hpp>
#include <boost/thread.hpp>

namespace mace { namespace cmt {
    /// cmt::thread::current().yield()
    void yield();

    /**
     *  @class mutex
     *  @brief A non-blocking mutex, modified spin-lock that yields on failure.
     *
     *  This mutex does not block the current thread, but instead attempts to use
     *  an atomic operation to aquire the lock.  If unsuccessful, then it yields to
     *  other tasks before trying again. If there is no current fiber, then yield is
     *  a no-op and mutex becomes a spin-lock.  
     */
    class mutex {
        public:
            mutex():m_state(unlocked){}

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

#endif // _MACE_CMT_MUTEX_HPP_
