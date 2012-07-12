#include<mace/cmt/thread.hpp>
#include<mace/rpc/json/process/client.hpp>
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
    std::cerr<<line<<std::endl;
    mace::cmt::yield();
    std::getline(err,line);
  }
}

int main( int argc, char** argv ) {
  try {
    mace::rpc::json::process::client<test_fixture> c;
    c.exec( "child" );
    slog( "return: '%1%'", c->hello( "World" ).wait() );
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
