#include <mace/cmt/mutex.hpp>
#include <mace/cmt/thread.hpp>
#include <boost/thread/locks.hpp>
#include "context.hpp"

namespace mace { namespace cmt {

  mutex::mutex()
  :m_blist(0){}

  mutex::~mutex() {
    if( m_blist ) {
      auto c = m_blist;
      mace::cmt::thread::current().debug("~mutex");
      while( c )  {
        elog( "still blocking on context %1% (%2%)", m_blist, (m_blist->cur_task ? m_blist->cur_task->get_desc() : "no current task") ); 
        c = c->next_blocked;
      }
    }
    BOOST_ASSERT( !m_blist && "Attempt to free mutex while others are blocking on lock." );
  }

  /**
   *  @param  next - is set to the next context to get the lock.
   *  @return the last context (the one with the lock)
   */
  static cmt::context* get_tail( cmt::context* h, cmt::context*& next ) {
    next = 0;
    cmt::context* n = h;
    if( !n ) return n;
    while( n->next_blocked ) { 
      next = n;
      n=n->next_blocked;
    }
    return n;
  }
  static cmt::context* remove( cmt::context* head, cmt::context* target ) {
    cmt::context* c = head;
    cmt::context* p = 0;
    while( c ) {
      if( c == target ) {
        if( p ) { 
          p->next_blocked = c->next_blocked; 
          return head; 
        }
        return c->next_blocked;
      }
      p = c;
      c = c->next_blocked;
    }
    return head;
  }
  static void cleanup( cmt::mutex& m, cmt::spin_yield_lock& syl, cmt::context*& bl, cmt::context* cc ) {
    {  
      boost::unique_lock<cmt::spin_yield_lock> lock(syl);
      if( cc->next_blocked ) {
        bl = remove(bl, cc ); 
        return;
      }
    }
    m.unlock();
  }

  /**
   *  A mutex is considered to hold the lock when
   *  the current context is the tail in the wait queue.
   */
  bool mutex::try_lock() {
    cmt::thread* ct = &cmt::thread::current();
    cmt::context* cc = ct->current_context();
    cmt::context* n  = 0;

    boost::unique_lock<cmt::spin_yield_lock> lock(m_blist_lock, boost::try_to_lock_t());
    if( !lock  ) 
      return false;

    if( !m_blist ) { 
      m_blist = cc;
      return true;
    }
    // allow recursive locks.
    return ( get_tail( m_blist, n ) == cc );
  }

  bool mutex::timed_lock( const boost::system_time& abs_time ) {
    cmt::context* n  = 0;
    cmt::context* cc = cmt::thread::current().current_context();

    { // lock scope
      boost::unique_lock<cmt::spin_yield_lock> lock(m_blist_lock,abs_time);
      if( !lock ) return false;

      if( !m_blist ) { 
        m_blist = cc;
        return true;
      }

      // allow recusive locks
      if ( get_tail( m_blist, n ) == cc ) 
        return true;

      cc->next_blocked = m_blist;
      m_blist = cc;
    } // end lock scope
    try {
        cmt::thread::current().yield_until( to_time_point(abs_time), false );
        return( 0 == cc->next_blocked );
    } catch (...) {
      cleanup( *this, m_blist_lock, m_blist, cc);
      throw;
    }
  }

  void mutex::lock() {
    cmt::context* n  = 0;
    cmt::context* cc = cmt::thread::current().current_context();
    {
      boost::unique_lock<cmt::spin_yield_lock> lock(m_blist_lock);
      if( !m_blist ) { 
        m_blist = cc;
        return;
      }

      // allow recusive locks
      if ( get_tail( m_blist, n ) == cc ) {
        return;
      }
      cc->next_blocked = m_blist;
      m_blist = cc;

      int cnt = 0;
      auto i = m_blist;
      while( i ) {
        i = i->next_blocked;
        ++cnt;
      }
      wlog( "wait queue len %1%", cnt );

    }

    try {
      cmt::thread::current().yield(false);
      BOOST_ASSERT( cc->next_blocked == 0 );
    } catch ( ... ) {
      wlog( "lock with throw %1% %2%",this, boost::current_exception_diagnostic_information() );
      cleanup( *this, m_blist_lock, m_blist, cc);
      throw;
    }
  }

  void mutex::unlock() {
    cmt::context* next = 0;
    BOOST_ASSERT( cmt::thread::current().current_context()->next == 0 );
    { boost::unique_lock<cmt::spin_yield_lock> lock(m_blist_lock);
      get_tail(m_blist, next);
      if( next ) {
        next->next_blocked = 0;
        next->ctx_thread->unblock( next );
      } else {
        m_blist   = 0;
      }
    }
  }

} } // mace::cmt


