#include <boost/test/unit_test.hpp>
#include <mace/rpc/value.hpp>
#include <mace/rpc/value_io.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <mace/rpc/json/io.hpp>
#include <boost/process/all.hpp>
#include <boost/filesystem.hpp>

#include <mace/ssh/client.hpp>
#include <mace/ssh/process.hpp>

#include <boost/exception/all.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <mace/rpc/json/pipe/connection.hpp>
#include <mace/rpc/json/pipe/client.hpp>
#include <iostream>
#include <mace/cmt/thread.hpp>

class test_fixture {
  public:
    std::string hello( std::string w ) { return "Hi, " + w; }
};

void read_err( mace::ssh::process::ptr p ) {
  try {
    wlog( "                                                                                                         start read err.. " );
    std::string line;
    std::getline( p->err_stream(), line );
    while( p->err_stream() ) {
      wlog( "                                                                                                       err: %1%", line );
      std::getline( p->err_stream(), line );
   //   wlog( "                                                                                                       getline return" );      
    }
    wlog( "                                                                                                         exit read error" );
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
  }
}


void read_out( mace::ssh::process::ptr p ) {
  try {
    slog( "                                                                                                                                           start read out.. " );
    std::string line;
    std::getline( p->out_stream(), line );
    while( p->out_stream() ) {
      slog( "                                                                                                                                         out: %1%", line );
      std::getline( p->out_stream(), line );
  //    slog( "                                                                                                                                         getline return" );      
    }
    slog( "                                                                                                                                           exit read outor" );
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
  }
}


MACE_STUB( test_fixture, (hello) )

typedef mace::rpc::json::pipe::connection<> connection;

namespace bp = boost::process;
namespace bpb = boost::process::behavior;
namespace fs  = boost::filesystem;

int test( ) {
try {
   mace::cmt::thread::current().set_name("ssh_test_main");
   auto sshc = mace::ssh::client::create();
   sshc->connect( "dlarimer", "rapture", "localhost");

   fs::path run("runner");
   //auto proc = sshc->exec((fs::absolute(run).native()) + " 2> out.txt   | /usr/bin/tee sout.txt ");//, "vt100");
   auto proc = sshc->exec((fs::absolute(run).native()));//, "vt100");

   mace::cmt::async( std::bind(read_err, proc ), "read_err" );

   auto con = connection::ptr( new connection( proc->out_stream(), proc->in_stream() ) );
   
   slog( "creating client" );
   mace::rpc::json::pipe::client<test_fixture> cl(con);

   slog( "sending hello!" );
   auto start = boost::chrono::system_clock::now();
   int cnt = 10000;
   std::vector< mace::cmt::future<std::string> > results(cnt);
   for( int i = 0; i < cnt; ++i ) {
      try {
        results[i] = cl->hello("WorldEnd");
      } catch ( ... ) {
        elog( "%1%", boost::current_exception_diagnostic_information() );
      }
   }
   for( uint32_t i = 0; i < results.size(); ++i ) {
        try {
          results[i].wait(boost::chrono::microseconds(1000000*5));
        } catch ( ... ) {
          mace::cmt::thread::current().debug("wait on result" );
        }
   }
   auto end = boost::chrono::system_clock::now();
   slog( "call %1%/s", cnt * 1000000000ll / double((end-start).count()) );

   con->close();
   proc->in_stream().close();
   mace::cmt::thread::current().debug("main");
   wlog("result: %1%",proc->result() );
   return 0;
  /*
   boost::process::context ctx;
   ctx.streams[boost::process::stdout_id] = boost::process::behavior::pipe();
   ctx.streams[boost::process::stdin_id] = boost::process::behavior::pipe();
   boost::process::child c = boost::process::create_child(runner, std::vector<std::string>(), ctx );


   slog( "creating pipes" );
   boost::process::pistream in( c.get_handle(boost::process::stdout_id) );
   boost::process::postream out( c.get_handle(boost::process::stdin_id) );
   */
} catch( ... ) {
  elog( "%1%", boost::current_exception_diagnostic_information() );
}
return 0;
}
int main( int argc, char** argv ) {
  try {
  mace::cmt::thread* t1 = mace::cmt::thread::create("th1");
  mace::cmt::thread* t2 = mace::cmt::thread::create("th2");
  mace::cmt::thread* t3 = mace::cmt::thread::create("th3");
  auto start = boost::chrono::system_clock::now();
  auto f1 = t1->async( test, "t1" );
  /*
  auto f11 = t1->async( test, "t11" );
  auto f2 = t2->async( test, "t2" );
  auto f22 = t2->async( test, "t22" );
  auto f3 = t3->async( test, "t3" );
  auto f33 = t3->async( test, "t33" );
  auto f4 = mace::cmt::async( test, "t4" );
  auto f44 = mace::cmt::async( test, "t44" );
  */
  try {
  f1.wait(); 
//  f11.wait(); 
  } catch ( ... ) {
  elog( "%1%", boost::current_exception_diagnostic_information() );
  }
  /*
  slog("");
  try {
  f2.wait(); 
  f22.wait(); 
  } catch ( ... ) {
  elog( "%1%", boost::current_exception_diagnostic_information() );
  }
  slog("");
  try {
  f3.wait(); 
  f33.wait(); 
  } catch ( ... ) {
  elog( "%1%", boost::current_exception_diagnostic_information() );
  }
  slog("");
  f4.wait();
  f44.wait();
  slog("");
  auto end = boost::chrono::system_clock::now();
  slog( "call %1%/s", 8*10000 * 1000000000ll / double((end-start).count()) );
*/
} catch( ... ) {
  elog( "%1%", boost::current_exception_diagnostic_information() );
}
  return 0;
}
