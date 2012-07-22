#include <mace/cmt/stdin.hpp>
#include <mace/cmt/thread.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>
#include <mace/cmt/thread.hpp>
#include <mace/cmt/stdin.hpp>



  class cin_source : public boost::iostreams::source {
    public:
      //typedef char char_type;
      //struct category : boost::iostreams::source::category, boost::iostreams::istream_tag{};

      cin_source( )
      :m_read_thread(mace::cmt::thread::create("cmt::cin")){ slog(""); }

      ~cin_source() {
        m_read_thread->quit();
      }


      std::streamsize read( char* s, std::streamsize n ) {
        slog( "%1%", n );
        if( n == 0 ){ return n; }
        auto r = std::cin.readsome(s,n);
        slog( "r %1%", r );
        if( r == 0 ) {
          slog( "async" );
          return m_read_thread->async( [=]()->size_t{ std::cin.read(s,1); return 1; }, "cin.getch" ).wait();
        }
        slog( "Return %1%", r );
        return r;
      }
    private:
      mace::cmt::thread* m_read_thread;
  };

  boost::iostreams::stream<cin_source>& init_cin() {
      slog( "" );
      static boost::iostreams::stream<cin_source> cin;
      slog( "" );
      return cin;
  }


void print() {
  while( true ) {
    std::cout<<".";
    mace::cmt::usleep(500000);
  }
}


int main( int argc, char** argv ) {
  mace::cmt::async( print );
  std::string s;
  char buf[100];
  init_cin().read(buf,8);
  //std::getline( mace::cmt::cin, s );
  while( init_cin() ) {
    std::cerr<<"Read: "<<std::string(buf,8)<<"\n";
    init_cin().read(buf,8);
//    std::getline( mace::cmt::cin, s );
  }
  return 0;
}
