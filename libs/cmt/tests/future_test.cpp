#define BOOST_TEST_MODULE MACE_CMT_ASIO
#include <boost/test/unit_test.hpp>
#include <mace/cmt/asio.hpp>
#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/cmt/thread.hpp>


mace::cmt::future<int> fut;

int run () {
  slog( "                                                               return 42!" );
  return 42;
}

int test() {
  slog( "   starting test func" );
  if( fut.valid() ) {
    wlog( "already waiting on future" );
    auto r= fut.wait();
    wlog( "done waiting on existing" );
    return r;
  }
  else {
    wlog( "starting task to get future" );
    fut = mace::cmt::async( run );
    slog( "waiting.." );
    auto r = fut.wait();
    slog( "done waiting" );
    return r;
  }
}

BOOST_AUTO_TEST_CASE( two_tasks_one_future ) {
  mace::cmt::thread::current().set_name("main");
  auto f1 = mace::cmt::async(test);
  auto f2 = mace::cmt::async(test);
  wlog( "start 1nd wait" );
  wlog( "result: %1%", f1.wait() );
  wlog( "start 2nd wait" );
  f2.wait();
  wlog( "result: %1%", f2.wait() );

  slog( "done" );
}
