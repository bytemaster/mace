#include<mace/cmt/thread.hpp>
#include<mace/rpc/json/process_client.hpp>
#include<mace/stub/ptr.hpp>

class test_fixture {
  public:
    std::string hello( std::string w ) { return "Hi, " + w; }
};

MACE_STUB( test_fixture, (hello) )


void read_error( std::istream& err ) {
  std::string line;
  std::getline(err,line);
  while( err ) {
    std::cerr<<"child:                                               "<<line<<std::endl;
    mace::cmt::yield();
    std::getline(err,line);
  }
}

int main( int argc, char** argv ) {
  try {
    mace::rpc::json::process::client<test_fixture> c;
    c.exec( "child" );

   auto start = boost::chrono::system_clock::now();
   int cnt = 100000;
   for( uint32_t i = 0; i < cnt; ++i ) {
        auto r = c->hello("World").wait();
   }
   auto end = boost::chrono::system_clock::now();
   slog( "call %1%/s", cnt * 1000000000ll / double((end-start).count()) );
   std::vector< mace::cmt::future<std::string> > results(cnt);
   start = boost::chrono::system_clock::now();
   for( int i = 0; i < cnt; ++i ) {
      try {
        if( i > 2 ) results[0].wait();
        results[i%2] = c->hello("World");
      } catch ( ... ) {
        elog( "%1%", boost::current_exception_diagnostic_information() );
      }
   }
   for( uint32_t i = 0; i < 2; ++i ) {
        try {
          results[i].wait(boost::chrono::microseconds(1000000*5));
        } catch ( ... ) {
          mace::cmt::thread::current().debug("wait on result" );
        }
   }
   end = boost::chrono::system_clock::now();
   slog( "call %1%/s", cnt * 1000000000ll / double((end-start).count()) );




  //  slog( "return: '%1%'", r );
    auto f = mace::cmt::async( [&](){ read_error( c.err_stream()); } );
    c.close();
    slog( "result: %1%", c.result() );
    f.wait();
    return 0;
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
    return -1;
  }
}
