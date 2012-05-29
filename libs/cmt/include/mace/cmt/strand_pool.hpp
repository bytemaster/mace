#ifndef _STRAND_POOL_HPP_
#define _STRAND_POOL_HPP_

namespace mace { namespace cmt {

    /**
     *  The strand_pool creates a small number of OS threads
     *  that will take turns running strands.  The tasks running
     *  within each strand are gauranteed to never run at the same time,
     *  but they may be run on different OS threads based upon load.
     *
     *  Because the strand pool is where every strand comes back to when
     *  it is 'idle', if you have 1000's of strands disbursed on say 2 threads
     *  per core (say 16 cores) then there will be a lot of waking and sleeping
     *  of strands.  The process of exchanging strands must avoid locks.
     *
     *  To achieve this, a scheduler thread will be required that has access
     *  to all of the context state and can safely manipulate the strand
     *  states.
     *
     *  The task of enquing a strand to be run (now or later) is atomic and 
     *  not blocking using the same system as cmt::thread for accepting tasks.
     *
     */
    class strand_pool {
      public:
        static strand_pool& instance();

        void     set_max_threads( uint16_t t );
        uint16_t max_threads()const;
        
        strand::ptr create( const std::string& str );

        /**
         *  Execution on @param s will stop until the specified @param tp or wake(s) is called.
         */
        void        wait_until( const strand::ptr&, const system_clock::time_point& tp );
        void        wake( const strand::ptr& s );

        /**
         *  Puts this strand at the end of the list of ready strands
         */
        void        yield( const strand::ptr& s );

      private:
        strand_pool();

        class strand_pool_private* my;
    };

} }

#endif
