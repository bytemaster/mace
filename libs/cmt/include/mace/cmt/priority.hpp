#ifndef _MACE_CMT_PRIORITY_HPP_
#define _MACE_CMT_PRIORITY_HPP_
namespace mace { namespace cmt {
  /**
   *  An integer value used to sort asynchronous tasks.  The higher the
   *  prioirty the sooner it will be run.
   */
   struct priority {
     explicit priority( int v = 0):value(v){}
     priority( const priority& p ):value(p.value){}
     bool operator < ( const priority& p )const {
        return value < p.value;
     }
     int value;
   };
} }  // mace::cmt
#endif
