#ifndef _MACE_CMT_STRAND_HPP_
#define _MACE_CMT_STRAND_HPP_


namespace mace { namespace cmt {
  
  /**
   *  A strand is a logical thread that is multi-plexed on
   *  a thread-pool of real threads.
   *
   *  Tasks performed by a strand could be run by any thread
   *  in the pool, but only one thread will be running these
   *  tasks at a time.
   *
   *  With this setup you gain the benefits of maximum parallelism 
   *  without the overhead of creating more real OS threads
   *  than necessary.
   *
   *  The primary use case for this concept is the actor paradigmn
   *  where each object becomes its own 'strand'.  You may have
   *  thousands of objects and each object can run in parallel but
   *  most of the time the majority of OS threads would be blocked
   *  or you would suffer from excessive context switching.
   *
   *  A strand is essentially a cmt::thread that instead of blocking
   *  on a wait condition when it runs out of tasks it yields to
   *  another strand and gets put back into the thread pool.
   *
   *  The next thread pool thread that blocks may resume processing
   *  where this strand left off. 
   */
  class strand : public boost::enable_shared_from_this<strand> {
    public:
      typedef boost::shared_ptr<strand> ptr;
      ~strand();

      /**
       *  Constructor is private, so use this method to create a new
       *  strand.  Under the hood it will assign it to the strand_pool.
       */
      static strand::ptr create( const std::string& name );
      static strand::ptr current();

      void   set_name( const std::string& name );

      void     exit();
      priority current_priority()const;

      template<typename R>
      cmt::future<R> async( const boost::function<R()>& f, 
                            const system_clock::time_point& when, 
                            priority prio = priority(), const char* n= "" );
      template<typename R>
      cmt::future<R> async( const boost::function<R()>& f, 
                                  priority prio = priority(), const char* n= "" );

      void           async( const boost::function<void()>& f, 
                                  const system_clock::time_point& when, 
                                  priority prio = priority(), const char* n= "" );
                     
      void           async( const boost::function<void()>& f, 
                                  priority prio = priority(), const char* n= "" );


      template<typename R>
      R sync( const boost::function<R()>& f, 
                    priority prio = priority(), const char* n= "" );

      bool is_running()const;


    protected: 
      strand( const std::string& name = "" );
    private:
      friend class strand_pool;
      class strand_private* my;
      strand*               next;

  };

} } // mace::cmt

#endif // _MACE_CMT_STRAND_HPP_
