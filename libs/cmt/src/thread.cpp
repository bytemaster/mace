#include <boost/chrono.hpp>
#include <mace/cmt/thread.hpp>
#include <boost/context/all.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/lexical_cast.hpp>

#include <mace/cmt/log/log.hpp>

#include <list>
#include <vector>

namespace mace { namespace cmt {
    using boost::chrono::system_clock;

    boost::posix_time::ptime to_system_time( const system_clock::time_point& t ) {
        typedef boost::chrono::microseconds duration_t;
        typedef duration_t::rep rep_t;
        rep_t d = boost::chrono::duration_cast<duration_t>(t.time_since_epoch()).count();
        static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
        return epoch + boost::posix_time::seconds(long(d/1000000)) + boost::posix_time::microseconds(long(d%1000000));
    }


    namespace bc = boost::ctx;

    
    /**
     *  maintains information associated with each context such as
     *  where it is blocked, what time it should resume, priority,
     *  etc.
     */
    struct cmt_context  {
        typedef cmt_context* ptr;

        bc::fcontext_t      my_context;
        cmt_context*        caller_context;

        void (*start_func)(intptr_t);

        cmt_context( void (*sf)(intptr_t), bc::stack_allocator& alloc )
        : caller_context(0),
          start_func(sf), 
          next_blocked(0), next(0),
          prom(0), canceled(false), m_complete(false)
        {
            my_context.fc_stack.base = alloc.allocate( bc::minimum_stacksize() );
            my_context.fc_stack.limit = 
              static_cast<char*>( my_context.fc_stack.base) - bc::minimum_stacksize();
            make_fcontext( &my_context, start_func );
        }

        cmt_context()
        :caller_context(0),
          start_func(0),
          next_blocked(0), next(0),
          prom(0), canceled(false), m_complete(false)
        {}
        
        /**
         *  @todo Have a list of promises so that we can wait for
         *        P1 or P2 and either will unblock instead of requiring both
         *  @param req - require this promise to 'unblock', otherwise try_unblock
         *         will allow it to be one of many that could 'unblock'
         */
        void add_blocking_promise( promise_base* p, bool req = true ) {
          prom = p;
        }
        /**
         *  If all of the required promises and any optional promises then
         *  return true, else false.
         *  @todo check list
         */
        bool try_unblock( promise_base* p ) {
          return p == prom;
        }

        void remove_blocking_promise( promise_base* p ) {
          prom = 0;
        }
        void clear_blocking_promises() {
          prom = 0;
        }

        bool is_complete()const { return m_complete; }
        bool m_complete;


        priority                 prio;
        promise_base*            prom; 
        system_clock::time_point resume_time;
        cmt_context*               next_blocked;
        cmt_context*               next;
        bool                     canceled;
    };
    struct sleep_priority_less {
        bool operator()( const cmt_context::ptr& a, const cmt_context::ptr& b ) {
            return a->resume_time > b->resume_time;
        }
    };

    class thread_private {
        public:
           boost::thread* boost_thread;
           thread_private(cmt::thread& s)
            :self(s), boost_thread(0),
             current(0),
             ready_head(0),
             ready_tail(0),
             blocked(0),
             task_in_queue(0),
             done(false)
            { 
              name = boost::lexical_cast<std::string>(uint64_t(this));
            }
           bc::stack_allocator              stack_alloc;
           boost::mutex                     task_ready_mutex;
           boost::condition_variable        task_ready;

           boost::atomic<task*>             task_in_queue;
           std::vector<task*>               task_pqueue;
           std::vector<task*>               task_sch_queue;
           std::vector<cmt_context*>        sleep_pqueue;

           cmt::thread&             self;
           bool                      done;
           std::string               name;
           cmt_context*              current;
           cmt_context*              ready_head;
           cmt_context*              ready_tail;

           cmt_context*                blocked;

           system_clock::time_point  check_for_timeouts();

            // insert at from of blocked linked list
           inline void add_to_blocked( cmt_context* c ) {
              c->next_blocked = blocked;
              blocked = c;
           }
           cmt_context::ptr ready_pop_front() {
                cmt_context::ptr tmp = 0;
                if( ready_head ) {
                    tmp        = ready_head;
                    ready_head = tmp->next;
                    if( !ready_head )   
                        ready_tail = 0;
                    tmp->next = 0;
                }
                return tmp;
           }
           void ready_push_front( const cmt_context::ptr& c ) {
                c->next = ready_head;
                ready_head = c;
                if( !ready_tail ) 
                    ready_tail = c;
           }
           void ready_push_back( const cmt_context::ptr& c ) {
                c->next = 0;
                if( ready_tail ) { 
                    ready_tail->next = c;
                } else {
                    ready_head = c;
                }
                ready_tail = c;
           }
           struct task_priority_less {
               bool operator()( const task::ptr& a, const task::ptr& b ) {
                   return a->prio.value < b->prio.value ? true :  (a->prio.value > b->prio.value ? false : a->posted_num > b->posted_num );
               }
           };
           struct task_when_less {
                bool operator()( const task::ptr& a, const task::ptr& b ) {
                    return a->when < b->when;
                }
           };

           void enqueue( const task::ptr& t ) {
                system_clock::time_point now = system_clock::now();
                task::ptr cur = t;
                while( cur ) {
                  if( cur->when > now ) {
                    task_sch_queue.push_back(cur);
                    std::push_heap( task_sch_queue.begin(),
                                    task_sch_queue.end(), task_when_less()   );
                  } else {
                    task_pqueue.push_back(cur);
                    std::push_heap( task_pqueue.begin(),
                                    task_pqueue.end(), task_priority_less()   );
                  }
                    cur = cur->next;
                }
           }
           task::ptr dequeue() {
                // get a new task
                task::ptr pending = task_in_queue.exchange(0,boost::memory_order_consume);
                if( pending ) { enqueue( pending ); }

                task::ptr p(0);
                if( task_sch_queue.size() ) {
                    if( task_sch_queue.front()->when <= system_clock::now() ) {
                        p = task_sch_queue.front();
                        std::pop_heap(task_sch_queue.begin(), task_sch_queue.end(), task_when_less() );
                        task_sch_queue.pop_back();
                        return p;
                    }
                }
                if( task_pqueue.size() ) {
                    p = task_pqueue.front();
                    std::pop_heap(task_pqueue.begin(), task_pqueue.end(), task_priority_less() );
                    task_pqueue.pop_back();
                }
                return p;
           }
           
           /**
            *   Find the next available context and switch to it.
            *   If none are available then create a new context and
            *   have it wait for something to do.
            */
           bool start_next_task( bool reschedule = false ) {
              check_for_timeouts();

              // check to see if any other contexts are ready
              if( ready_head ) { 
                cmt_context* next = ready_pop_front();

                BOOST_ASSERT( next != current ); 
                if( reschedule ) { 
                    BOOST_ASSERT(current);
                    ready_push_back(current);
                }

                // jump to next context, saving current context
                //slog( "jump from %1% to %2%", current, next );
                cmt_context* prev = current;
                current = next;
                bc::jump_fcontext( &prev->my_context, &next->my_context, 0 );
                current = prev;

                // restore current context
              } else { // all contexts are blocked, create a new context 
                       // that will process posted tasks...
                if( reschedule ) { 
                    BOOST_ASSERT(current);
                    ready_push_back(current);
                }

                cmt_context* next = new cmt_context( &thread_private::start_process_tasks, stack_alloc );
               // wlog( "create new context %1%", next );
                cmt_context* prev = current;
               // slog( "jump from %1% to %2%", prev, next );
                current = next;
                bc::jump_fcontext( &prev->my_context, &next->my_context, (intptr_t)this );
                current = prev;
               // slog( "back to %1%", current );
              }

              if( current->canceled )
                  BOOST_THROW_EXCEPTION( cmt::error::task_canceled() );

              return true;
           }

           static void start_process_tasks( intptr_t my ) {
              thread_private* self = (thread_private*)my;
              self->process_tasks();
           }

           /**
            * Process tasks until everything is done...
            */
           void process_tasks( ) {
              system_clock::time_point timeout_time;
              task::ptr next = dequeue();

              while( !done ) {
                timeout_time = check_for_timeouts();

                while( next ) {
                    next->set_active_context( current );
                    next->run();
                    next->set_active_context( 0 );
                    next->release();
                    timeout_time = check_for_timeouts();
                    next = dequeue();
                }

                if( !blocked && done ) { 
                  elog( "%1% exit!", current );
                  return;
                }

                if( timeout_time == system_clock::time_point::min() ) {
                    continue;
                }

                bool block_wait = false;

                if( ready_head ) { 
                  start_next_task(true);
                  if( !(next = dequeue()) )  {
                    block_wait = true;
                  }
                } else { block_wait = true; }

                if( block_wait ) {
               //   wlog( "%2% no task to run, block until task posted or %1%", 
               //                 timeout_time - system_clock::now(), current );
                  boost::unique_lock<boost::mutex> lock(task_ready_mutex);
                  if( !(next = dequeue())) {
                    if( timeout_time == system_clock::time_point::max() ) {
                        task_ready.wait( lock );
                    } else {
                        task_ready.timed_wait( lock, to_system_time(timeout_time) );
                    }
                  }
                }
              }
             elog( "exit process tasks blocked %1%  !done %2%", blocked, !done );
           }

    };

    /**
     *    Return system_clock::time_point::min() if tasks have timed out
     *    Retunn system_clock::time_point::max() if there are no scheduled tasks
     *    Return the time the next task needs to be run if there is anything scheduled.
     */
    system_clock::time_point thread_private::check_for_timeouts() {
        if( !sleep_pqueue.size() && !task_sch_queue.size() ) {
            return system_clock::time_point::max();
        }


        system_clock::time_point next = system_clock::time_point::max();
        if( task_sch_queue.size() && next > task_sch_queue.front()->when )
          next = task_sch_queue.front()->when;
        if( sleep_pqueue.size() && next > sleep_pqueue.front()->resume_time )
          next = sleep_pqueue.front()->resume_time;

        boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
        if( now < next ) { return next; }

        // move all expired sleeping tasks to the ready queue
        while( sleep_pqueue.size() && now >= sleep_pqueue.front()->resume_time ) {
            cmt_context::ptr c = sleep_pqueue.front();
            std::pop_heap(sleep_pqueue.begin(), sleep_pqueue.end(), sleep_priority_less() );
            sleep_pqueue.pop_back();

            if( c->prom ) {
                c->prom->set_exception( boost::copy_exception( error::future_wait_timeout() ) );
            }
            else { ready_push_back( c ); }
        }
        return system_clock::time_point::min();
    }

    thread& thread::current() {
// Apple does not support __thread by default, but some custom gcc builds
// for Mac OS X support it.  Backup use boost::thread_specific_ptr
#if defined(__APPLE__) && (__GNUC__ <= 4 && __GNUC_MINOR__ < 4)
    #warning using boost::thread_specific_ptr instead of __thread, use gcc 4.5 for better performance.
    static boost::thread_specific_ptr<thread>  t;
    if( !t.get() ) t.reset( new thread() );
        return *t.get();
#else
    #ifdef _MSC_VER
       static __declspec(thread) thread* t = NULL;
    #else
       static __thread thread* t = NULL;
    #endif
       if( !t ) t = new thread();
       return *t;
#endif
    }

    void start_thread( const promise<thread*>::ptr p, const char* n  ) {
        //slog( "starting cmt::thread %1%", n );
        p->set_value( &thread::current() );
        thread::current().set_name(n);
        exec();
        //wlog( "exiting cmt::thread" );
    }

    thread* thread::create( const char* n ) {
        //if( current().my->current ) {
          promise<thread*>::ptr p(new promise<thread*>());
          boost::thread* t = new boost::thread( boost::bind(start_thread,p,n) );
          cmt::thread* ct = p->wait();
          ct->set_boost_thread(t);
          return ct;
          /*
        }
        promise<thread*>::ptr p(new blocking_promise<thread*>());
        boost::thread* t = new boost::thread( boost::bind(start_thread,p,n) );
        cmt::thread* ct = p->wait();
        ct->set_boost_thread(t);
        return ct;
        */
    }

    void thread::set_boost_thread( boost::thread* t ) {
      my->boost_thread = t;
    }

    int  exec() { cmt::thread::current().exec(); return 0; }
    void async( const boost::function<void()>& t, priority prio ) {
       thread::current().async(t,prio);
    }

    thread::thread() {
        my = new thread_private(*this);
    }

    thread::~thread() {
        delete my;
    }

    void thread::sleep_until( const boost::chrono::system_clock::time_point& tp ) {
        if( my->done )  {
          BOOST_THROW_EXCEPTION( error::thread_quit() );
        }
        BOOST_ASSERT( &current() == this );

        if( !my->current )  {
        //  wlog( "!my->current" );
          my->current = new cmt_context();
        }

        my->current->resume_time = tp;
        my->current->clear_blocking_promises();

        my->sleep_pqueue.push_back(my->current);
        std::push_heap( my->sleep_pqueue.begin(),
                        my->sleep_pqueue.end(), sleep_priority_less()   );

       // slog( "starting next task" );
        my->start_next_task();

       // slog( "return from sleep_until" );

        my->current->resume_time = system_clock::time_point::max();
        
        
        //my->current->prom = 0;
        if( my->current->canceled ) {
          BOOST_THROW_EXCEPTION( cmt::error::task_canceled() );
        }
    }
    void thread::usleep( uint64_t timeout_us ) {
        //slog( "usleep %1%", timeout_us );
        BOOST_ASSERT( &current() == this );
        //BOOST_ASSERT(my->current);
        sleep_until( system_clock::now() + microseconds(timeout_us) );
    }

    void thread::wait( const promise_base::ptr& p, const system_clock::time_point& timeout ) {
        BOOST_ASSERT( &current() == this );

        if( p->ready() ) return;
        if( timeout < system_clock::now() ) 
            BOOST_THROW_EXCEPTION( cmt::error::future_wait_timeout() );
        
        if( !my->current ) { 
          slog( "new cmt context becaure current is 0" );
          my->current = new cmt_context(); 
        }

        my->current->add_blocking_promise(p.get(),true);

        // if not max timeout, added to sleep pqueue
        if( timeout != system_clock::time_point::max() ) {
            my->current->resume_time = timeout;
            my->sleep_pqueue.push_back(my->current);
            std::push_heap( my->sleep_pqueue.begin(),
                            my->sleep_pqueue.end(), 
                            sleep_priority_less()   );
        }
        //slog( "blocking %1%", my->current );
        my->add_to_blocked( my->current );
        my->start_next_task();
        //slog( "resuming %1%", my->current );

        my->current->remove_blocking_promise(p.get());

        if( my->current->canceled )
            BOOST_THROW_EXCEPTION( cmt::error::task_canceled() );
    }
    void thread::wait( const promise_base::ptr& p, const boost::chrono::microseconds& timeout_us ) {
       if( timeout_us == microseconds::max() ) 
          wait( p, system_clock::time_point::max() ); 
       else 
          wait( p, system_clock::now() + timeout_us );
    }

    void thread::notify( const promise_base::ptr& p ) {
        
        BOOST_ASSERT(p->ready());
        if( &current() != this )  {
            async( boost::bind( &thread::notify, this, p ) );
            return;
        }
     //   slog( "notify!" );

        // TODO: store a list of blocked contexts with the promise 
        //  to accelerate the lookup.... unless it introduces contention...
        
        // iterate over all blocked contexts
        cmt_context* cur_blocked  = my->blocked;
        cmt_context* prev_blocked = 0;
        while( cur_blocked ) {

            // if the blocked context is waiting on this promise 
            // TODO: what if multiple things are blocked sleeping on this promise??
            if( cur_blocked->try_unblock( p.get() )  ) {
                //slog( "unblock!" );
                // remove it from the blocked list.
                if( prev_blocked ) {  
                    prev_blocked->next_blocked = cur_blocked->next_blocked; 
                } else { 
                    my->blocked = cur_blocked->next_blocked; 
                }
                cur_blocked->next_blocked = 0;
               // slog( "ready push front %1%", cur_blocked );
                my->ready_push_front( cur_blocked );
                cur_blocked =  cur_blocked->next_blocked;
            } else { // goto the next blocked task
                //elog( "unable to unblock.." );
                prev_blocked  = cur_blocked;
                cur_blocked   = cur_blocked->next_blocked;
            }
        }


        for( uint32_t i = 0; i < my->sleep_pqueue.size(); ++i ) {
            if( my->sleep_pqueue[i]->prom == p.get() ) {
                my->sleep_pqueue[i]->prom = 0;
                my->sleep_pqueue[i] = my->sleep_pqueue.back();
                my->sleep_pqueue.pop_back();
                std::make_heap( my->sleep_pqueue.begin(),my->sleep_pqueue.end(), sleep_priority_less() );
                break;
            }
        }
    }

    /**
     *  If no current context, create a new context running exec()
     *  If a current context exists, process tasks until the queue is
     *  empty and then block.
     */
    void thread::exec() { my->process_tasks(); }

    bool thread::is_running()const {
      return my->current != NULL;
    }

    /**
     *   Switches from the current task to the next ready task.
     *
     *   If there are no other ready tasks and the input queue is empty then
     *   return immediately.  
     *
     *   If there are no other ready contexts, but items on the input queue then push
     *   this context into the ready queue, mark current as NULL and then
     *   start a new context running exec() to process the next item on the
     *   input queue. Yield will return next time a context yields. 
     *
     *   If there are other ready contexts, push this context on to the end and then
     *   yield to the first ready context.  Yield will return after all other ready
     *   tasks have yielded at least once.
     */
    void thread::yield() {
        my->start_next_task(true);
    }

    void thread::quit() {
        if( &current() != this ) {
            async<void>( boost::bind( &thread::quit, this ) ).wait();
            if( my->boost_thread ) {
              slog("%2% joining thread... %1%", this->name(), current().name() );
              my->boost_thread->join();
              wlog( "%2% joined thread %1% !!!", name(), current().name() );
            }
            return;
        }
        cmt_context* cur  = my->blocked;
        while( cur ) {
            //cur->canceled = true;
            // setting the exception, will notify, thus modify blocked list
            cur->prom->set_exception( boost::copy_exception( error::thread_quit() ) );
            //cur = my->blocked;
            cur = cur->next;
        }
        cur = my->ready_head;
        while( cur ) {
          cur->canceled = true;
          //if( cur->prom )
          //    cur->prom->set_exception( boost::copy_exception( error::thread_quit() ) );
          cur = cur->next;
        }
        for( uint32_t i = 0; i < my->sleep_pqueue.size(); ++i ) {
          my->sleep_pqueue[i]->canceled = true;
          my->ready_push_front( my->sleep_pqueue[i] );
          // move to ready?
        }
        my->sleep_pqueue.clear();

        my->done = true;
        my->task_ready.notify_all();
    }


    void thread::async( const boost::function<void()>& t, priority prio ) {
       // disabled in attempt to resolve crashes when current_priority is called from ASIO callback???
       //async(task::ptr( new vtask(t,(std::max)(current_priority(),prio)) ) );
       async(task::ptr( new vtask(t,prio) ) );
    }
    void thread::async( const task::ptr& t ) {
        //slog( "async..." );
        task::ptr stale_head = my->task_in_queue.load(boost::memory_order_relaxed);
        do {
            t->next = stale_head;
        }while( !my->task_in_queue.compare_exchange_weak( stale_head, t, boost::memory_order_release ) );

        if( this != &current() ) {
            boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
            my->task_ready.notify_all();
        }
        //slog( "return.." );
    }
    void yield() { thread::current().yield(); }

    priority current_priority() { return cmt::thread::current().current_priority(); }
    priority thread::current_priority()const {
        BOOST_ASSERT(my);
        if( my->current ) return my->current->prio;
        return priority();
    }

    const char* thread::name()const               { 
      return my->name.c_str();
    }
    void        thread::set_name( const char* n ) { my->name = n;    }
    const char* thread_name() { return thread::current().name(); }


    /**
     *  This is implemented in thread.cpp because it needs access to the cmt_context type
     *  in order to kill the current context.
     */
    void task::cancel() {
      boost::unique_lock<cmt::spin_lock> lock( active_context_lock );
      canceled = true;
      if( active_context ) {
        active_context->canceled = true;
        active_context = 0;
      }
    }

} } // namespace mace::cmt 

