#include <mace/cmt/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <set>
#include <mace/cmt/log/log.hpp>

#include "context.hpp"

#include <list>
#include <vector>

namespace mace { namespace cmt {
    using boost::chrono::system_clock;

    boost::posix_time::ptime to_system_time( const system_clock::time_point& t ) {
        typedef boost::chrono::microseconds duration_t;
        typedef duration_t::rep rep_t;
        rep_t d = boost::chrono::duration_cast<duration_t>(t.time_since_epoch()).count();
        static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
        return epoch + boost::posix_time::seconds(long(d/1000000))  
                     + boost::posix_time::microseconds(long(d%1000000));
    }

    system_clock::time_point to_time_point( const boost::posix_time::ptime& from ) {
      namespace chrono = boost::chrono;
      boost::posix_time::time_duration const time_since_epoch=from-boost::posix_time::from_time_t(0); 
      system_clock::time_point t = chrono::system_clock::from_time_t(time_since_epoch.total_seconds());
      long nsec=time_since_epoch.fractional_seconds()*(1000000000/time_since_epoch.ticks_per_second()); 
      return t+chrono::nanoseconds(nsec); 
    }

    struct sleep_priority_less {
        bool operator()( const cmt::context::ptr& a, const cmt::context::ptr& b ) {
            return a->resume_time > b->resume_time;
        }
    };

    class thread_private {
        public:
           thread_private(cmt::thread& s)
            :self(s), boost_thread(0),
             task_in_queue(0),
             done(false),
             current(0),
             pt_head(0),
             ready_head(0),
             ready_tail(0),
             blocked(0)
            { 
              name = boost::lexical_cast<std::string>(uint64_t(this));
            }
           cmt::thread&             self;
           boost::thread* boost_thread;
           bc::stack_allocator              stack_alloc;
           boost::mutex                     task_ready_mutex;
           boost::condition_variable        task_ready;

           boost::atomic<task*>             task_in_queue;
           std::vector<task*>               task_pqueue;
           std::vector<task*>               task_sch_queue;
           std::vector<cmt::context*>       sleep_pqueue;
           std::vector<cmt::context*>       free_list;


           bool                      done;
           std::string               name;
           cmt::context*             current;

           cmt::context*             pt_head;

           cmt::context*             ready_head;
           cmt::context*             ready_tail;

           cmt::context*             blocked;

           system_clock::time_point  check_for_timeouts();

            // insert at from of blocked linked list
           inline void add_to_blocked( cmt::context* c ) {
              c->next_blocked = blocked;
              blocked = c;
           }

           void pt_push_back(cmt::context* c) {
              c->next = pt_head;
              pt_head = c;
              /* 
              cmt::context* n = pt_head;
              int i = 0;
              while( n ) {
                ++i;
                n = n->next;
              }
              wlog( "idle context...%2%  %1%", c, i );
              */
           }
           cmt::context::ptr ready_pop_front() {
                cmt::context::ptr tmp = 0;
                if( ready_head ) {
                    tmp        = ready_head;
                    ready_head = tmp->next;
                    if( !ready_head )   
                        ready_tail = 0;
                    tmp->next = 0;
                }
                return tmp;
           }
           void ready_push_front( const cmt::context::ptr& c ) {
                c->next = ready_head;
                ready_head = c;
                if( !ready_tail ) 
                    ready_tail = c;
           }
           void ready_push_back( const cmt::context::ptr& c ) {
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
                    BOOST_ASSERT( this == thread::current().my );
                    std::push_heap( task_pqueue.begin(),
                                    task_pqueue.end(), task_priority_less()   );
                  }
                    cur = cur->next;
                }
           }
           task* dequeue() {
                // get a new task
                BOOST_ASSERT( this == thread::current().my );
                
                task* pending = 0; 

                pending = task_in_queue.exchange(0,boost::memory_order_consume);
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
            * This should be before or after a context switch to
            * detect quit/cancel operations and throw an exception.
            */
           void check_fiber_exceptions() {
              if( current->canceled ) {
                BOOST_THROW_EXCEPTION( error::task_canceled() );
              } else if( done )  {
                BOOST_THROW_EXCEPTION( error::thread_quit() );
              }
           }
           
           /**
            *   Find the next available context and switch to it.
            *   If none are available then create a new context and
            *   have it wait for something to do.
            */
           bool start_next_fiber( bool reschedule = false ) {
              check_for_timeouts();
              if( !current ) current = new cmt::context( &cmt::thread::current() );

              // check to see if any other contexts are ready
              if( ready_head ) { 
                cmt::context* next = ready_pop_front();
                BOOST_ASSERT( next != current ); 
                if( reschedule ) ready_push_back(current);

                // jump to next context, saving current context
                cmt::context* prev = current;
                current = next;
                bc::jump_fcontext( &prev->my_context, &next->my_context, 0 );
                current = prev;
                BOOST_ASSERT( current );
              } else { // all contexts are blocked, create a new context 
                       // that will process posted tasks...
                if( reschedule )  ready_push_back(current);
                
                cmt::context* next;
                if( pt_head ) {
                  next = pt_head;
                  pt_head = pt_head->next;
                  next->next = 0;
                } else {
                  next = new cmt::context( &thread_private::start_process_tasks, stack_alloc,
                                                                      &cmt::thread::current() );
                }
                cmt::context* prev = current;
                current = next;
                bc::jump_fcontext( &prev->my_context, &next->my_context, (intptr_t)this );
                current = prev;
                BOOST_ASSERT( current );
              }

              if( current->canceled )
                  BOOST_THROW_EXCEPTION( cmt::error::task_canceled() );

              return true;
           }

           static void start_process_tasks( intptr_t my ) {
              thread_private* self = (thread_private*)my;
              try {
                self->process_tasks();
              } catch ( ... ) {
                std::cerr<<"fiber exited with uncaught exception:\n "<< 
                      boost::current_exception_diagnostic_information() <<std::endl;
              }
              self->free_list.push_back(self->current);
              self->start_next_fiber( false );
           }

           bool run_next_task() {
                system_clock::time_point timeout_time = check_for_timeouts();
                task* next = dequeue();
                if( next ) {
                    next->set_active_context( current );
                    next->run();
                    next->set_active_context(0);
                    delete next;
                    return true;
                }
                return false;
           }
           bool has_next_task() {
             if( task_pqueue.size() ||
                 (task_sch_queue.size() && task_sch_queue.front()->when <= system_clock::now()) ||
                 task_in_queue.load( boost::memory_order_relaxed ) )
                  return true;
             return false;
           }
           void clear_free_list() {
              for( uint32_t i = 0; i < free_list.size(); ++i ) {
                delete free_list[i];
              }
              free_list.clear();
           }
           void process_tasks() {
              while( !done || blocked ) {
                if( run_next_task() ) continue;

                // if I have something else to do other than
                // process tasks... do it.
                if( ready_head ) { 
                   pt_push_back( current ); 
                   start_next_fiber(false);  
                   continue;
                }

                clear_free_list();

                { // lock scope
                  boost::unique_lock<boost::mutex> lock(task_ready_mutex);
                  if( has_next_task() ) continue;
                  system_clock::time_point timeout_time = check_for_timeouts();
                  
                  if( timeout_time == system_clock::time_point::max() ) {
                    task_ready.wait( lock );
                  } else if( timeout_time != system_clock::time_point::min() ) {
                    task_ready.timed_wait( lock, to_system_time(timeout_time) );
                  }
                }
              }
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
            cmt::context::ptr c = sleep_pqueue.front();
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
      try {
        p->set_value( &thread::current() );
        thread::current().set_name(n);
        exec();
      } catch ( ... ) {
        elog( "Caught unhandled exception" );
      }
    }

    thread* thread::create( const char* n ) {
      promise<thread*>::ptr p(new promise<thread*>());
      boost::thread* t = new boost::thread( boost::bind(start_thread,p,n) );
      cmt::thread* ct = p->wait();
      ct->set_boost_thread(t);
      return ct;
    }

    void thread::set_boost_thread( boost::thread* t ) {
      my->boost_thread = t;
    }

    int  exec() { cmt::thread::current().exec(); return 0; }
    void async( const boost::function<void()>& t, priority prio ) {
      thread::current().post(t,prio);
    }

    thread::thread() {
      my = new thread_private(*this);
    }

    thread::~thread() {
      delete my;
    }

    void thread::sleep_until( const system_clock::time_point& tp ) {
      my->check_fiber_exceptions();
      
      BOOST_ASSERT( &current() == this );
      if( !my->current )  {
        my->current = new cmt::context(&cmt::thread::current());
      }

      my->current->resume_time = tp;
      my->current->clear_blocking_promises();

      my->sleep_pqueue.push_back(my->current);
      std::push_heap( my->sleep_pqueue.begin(),
                      my->sleep_pqueue.end(), sleep_priority_less()   );

      my->start_next_fiber();
      my->current->resume_time = system_clock::time_point::max();

      my->check_fiber_exceptions();
    }

    void thread::usleep( uint64_t timeout_us ) {
      //slog( "usleep %1%", timeout_us );
      BOOST_ASSERT( &current() == this );
      //BOOST_ASSERT(my->current);
      sleep_until( system_clock::now() + microseconds(timeout_us) );
    }

    void thread::wait( const promise_base::ptr& p, const system_clock::time_point& timeout ) {
      if( p->ready() ) return;
      if( timeout < system_clock::now() ) 
          BOOST_THROW_EXCEPTION( cmt::error::future_wait_timeout() );
      
      if( !my->current ) { 
        my->current = new cmt::context(&cmt::thread::current()); 
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
      my->start_next_fiber();
      //slog( "resuming %1%", my->current );

      my->current->remove_blocking_promise(p.get());

      my->check_fiber_exceptions();
    }
    void thread::wait( const promise_base::ptr& p, const boost::chrono::microseconds& timeout_us ) {
      if( timeout_us == microseconds::max() ) 
        wait( p, system_clock::time_point::max() ); 
      else 
        wait( p, system_clock::now() + timeout_us );
    }

    void thread::notify( const promise_base::ptr& p ) {
     // slog( "notify task complete" );
      BOOST_ASSERT(p->ready());
      if( &current() != this )  {
        //slog( "post notify to %1% from %2%", name(), current().name() );
        this->async( boost::bind( &thread::notify, this, p ) );
        return;
      }
      // TODO: store a list of blocked contexts with the promise 
      //  to accelerate the lookup.... unless it introduces contention...
      
      // iterate over all blocked contexts
      cmt::context* cur_blocked  = my->blocked;
      cmt::context* prev_blocked = 0;
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
          //slog( "ready push front %1%", cur_blocked );
          my->ready_push_front( cur_blocked );
          cur_blocked =  cur_blocked->next_blocked;
        } else { // goto the next blocked task
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
    void thread::exec() { 
       if( !my->current ) my->current = new cmt::context(&cmt::thread::current());
       my->process_tasks(); 
       delete my->current;
       my->current = 0;
    }

    bool thread::is_running()const {
      return !my->done;
    }

    /**
     *   Switches from the current task to the next ready task.
     *
     *   If there are no other ready tasks and the input queue is empty then
     *   return immediately if reschedule is set to true (default)..  
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
    void thread::yield( bool reschedule ) {
        my->check_fiber_exceptions();
        my->start_next_fiber(reschedule);
        my->check_fiber_exceptions();
    }

    void thread::yield_until( const system_clock::time_point& tp, bool reschedule ) {
      my->check_fiber_exceptions();

      if( tp <= system_clock::now() ) 
        return;

      if( !my->current )  {
        my->current = new cmt::context(&cmt::thread::current());
      }

      my->current->resume_time = tp;
      my->current->clear_blocking_promises();

      my->sleep_pqueue.push_back(my->current);
      std::push_heap( my->sleep_pqueue.begin(),
                      my->sleep_pqueue.end(), sleep_priority_less()   );

      my->start_next_fiber(reschedule);

      // clear current context from sleep queue...
      for( uint32_t i = 0; i < my->sleep_pqueue.size(); ++i ) {
        if( my->sleep_pqueue[i] == my->current ) {
          my->sleep_pqueue[i] = my->sleep_pqueue.back();
          my->sleep_pqueue.pop_back();
          std::make_heap( my->sleep_pqueue.begin(),
                          my->sleep_pqueue.end(), sleep_priority_less() );
          break;
        }
      }

      my->current->resume_time = system_clock::time_point::max();
      my->check_fiber_exceptions();
    }

    cmt::context* thread::current_context()const  {
      return my->current;
    }

    /**
     *  @todo make 'thread::unblock' this atomic instead of posting an event
     */
    void thread::unblock( cmt::context* c ) {
      if(  &current() != this ) {
        async( boost::bind(  &thread::unblock, this, c ) );
        return;
      }
      my->ready_push_front(c); 
    }

    void thread::quit() {
        if( &current() != this ) {
            async( boost::bind( &thread::quit, this ) ).wait();
            if( my->boost_thread ) {
              //slog("%2% joining thread... %1%", this->name(), current().name() );
              my->boost_thread->join();
              //wlog( "%2% joined thread %1% !!!", name(), current().name() );
            }
            return;
        }

        // break all promises, thread quit!
        cmt::context* cur  = my->blocked;
        while( cur ) {
            cmt::context* n = cur->next;
            // this will move the context into the ready list.
            cur->prom->set_exception( boost::copy_exception( error::thread_quit() ) );
            cur = n;
        }
        BOOST_ASSERT( my->blocked == 0 );
        //my->blocked = 0;
        
       
        // move all sleep tasks to ready
        for( uint32_t i = 0; i < my->sleep_pqueue.size(); ++i ) {
          my->ready_push_front( my->sleep_pqueue[i] );
        }
        my->sleep_pqueue.clear();

        // move all idle tasks to ready
        cur = my->pt_head;
        while( cur ) {
          cmt::context* n = cur->next;
          cur->next = 0;
          my->ready_push_front( cur );
          cur = n;
        }

        // mark all ready tasks (should be everyone)... as canceled 
        cur = my->ready_head;
        while( cur ) {
          cur->canceled = true;
          cur = cur->next;
        }

        my->done = true;

        // now that we have poked all fibers... switch to the next one and
        // let them all quit.
        while( my->ready_head ) { 
          my->start_next_fiber(true); 
          my->check_for_timeouts();
        }
        my->clear_free_list();
    }


    void thread::async_task( const task::ptr& t ) {
        task::ptr stale_head = my->task_in_queue.load(boost::memory_order_relaxed);
        do { t->next = stale_head;
        }while( !my->task_in_queue.compare_exchange_weak( stale_head, t, boost::memory_order_release ) );

        // Because only one thread can post the 'first task', only that thread will attempt
        // to aquire the lock and therefore there should be no contention on this lock except
        // when *this thread is about to block on a wait condition.  
        if( this != &current() &&  !stale_head ) { 
            boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
            my->task_ready.notify_one();
        }
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
     *  This is implemented in thread.cpp because it needs access to the cmt::context type
     *  in order to kill the current context.
     *
     *  @todo this can be moved to task.cpp now that context.hpp is a separate file.
     *        
     *  @todo determine if this is really called by different threads
     */
    void task::cancel() {
      boost::unique_lock<cmt::spin_lock> lock( active_context_lock );
      canceled = true;
      if( active_context ) {
        if( active_context->prom ) {
          // set exception and return
          active_context->prom->cancel();
        }
        active_context->canceled = true;
        active_context = 0;
      }
    }

} } // namespace mace::cmt 

