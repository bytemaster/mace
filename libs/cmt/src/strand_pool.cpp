#include <mace/cmt/strand_pool.hpp>

namespace mace { namespace cmt {

  struct waiting_strand{
    waiting_strand( const system_clock::time_point& timeout, const cmt::strand::ptr& s )
    :timeout(to),str(s){}

    boost::chrono::system_clock::time_point timeout;
    cmt::strand::ptr                             str;

    bool operator < ( const waiting_strand& ws )const { return timeout < ws.timeout; }
  };

  class strand_pool_private {
    public:
      std::vector<waiting_strand>             waiting_strands;
      strand*                                 ready_head;
      strand*                                 ready_tail;

      boost::atomic<int>                      ready_strand_count;
      boost::atomic<int>                      idle_thread_count;

      std::list<boost::shared_ptr<boost::thread> > thread_pool;


      void process_strands() {
        // get next ready strand
        // run the strand until it calls yield() or wait_until() at which point
        //   we save its context... and switch to the next strand's context. 
      }

      void ready_push_back( cmt::strand::ptr& s ) {
        // TODO: Grab Lock while we manipulate the shared list.
        if( ready_tail ) { 
          ready_tail->next = s.get();
          ready_tail = s.get();
          ++ready_strand_count;
        }
        else { 
          ++ready_strand_count;
          ready_head = ready_tail = s.get();
        }
      }
  };



  strand_pool::strand_pool() {
    my = new strand_pool_private();
  }

  strand_pool::~strand_pool() {
    delete my;
  }

  cmt::strand::ptr strand_pool::create( const std::string& name ) {
    cmt::strand::ptr str( new cmt::strand(name) );
    waiting_strands.push_back( waiting_strand(system_clock::time_point::max(), str) );
    std::push_heap( waiting_strands.begin(), waiting_strands.end() );
    return static_pointer_cast<cmt::strand>(str);
  }

  void strand_pool::yield( strand::ptr& s ) {
    strand* next_ready = 0;
    
    // push s to back and pop front ready task
    {   // TODO: Grab Lock while we manipulate the shared list.
        // push to the back of ready
        if( ready_tail ) { 
          ready_tail->next = s.get();
          ready_tail = s.get();

          next_ready = ready_head;
          ready_head = next_ready->next;
          if( !ready_head ) ready_tail = 0;
        }
        else { 
          return; // no one else is ready.
        }
    }
    // TODO: Switch to next strand
  }

  void strand_pool::wait_until( const strand::ptr& s, const system_clock::time_point& tp ) {
    if( tp < system_clock::now() ) return;
    my->waiting_strands.push_back( my->waiting_strand( tp, str) );
    std::push_heap( my->waiting_strands.begin(), my->waiting_strands.end() );

    // TODO: Switch to next strand
  }

  /**
   *  Could be called by any thread...
   */
  void strand_pool::wake( const strand::ptr& s ) {
    // find s in waiting pool, move it to ready front
    for( uint32_t i = 0; i < my->waiting_strands.size(); ++i ) {
      if( my->waiting_strands[i].str == s ) {
        my->waiting_strands[i] = my->waiting_strands.back();
        my->waiting_strands.pop_back();
        std::make_heap( my->waiting_strands.begin(), my->waiting_strands.end() );
      }
    }
    my->ready_push_back( s );
    if( my->idle_thread_count == 0 && my->thread_pool.size() < my->max_thread_count ) {
      thread_pool.push_back( 
        boost::make_shared<boost::thread>(boost::bind(&strand_pool_private, my ) ) );
    } else {
      boost::unique_lock<boost::mutex> lock(my->strand_ready_mutex);
      my->strand_ready.notify_one();
    }
  }

  strand_pool& strand_pool::instance() {
    static strand_pool inst;
    return inst;
  }


} }
